#include "theorem_prover.hpp"
#include "fourier_motzkin.hpp"

#include <stdexcept>
#include <map>

std::map<std::string, std::size_t> collect_variables(std::shared_ptr<Formula> formula);
std::shared_ptr<Formula> simplify_constraints(std::shared_ptr<Formula> formula);

bool TheoremProver::is_theorem(const std::string &fol_formula) const
{
    auto formula = m_driver.parse(fol_formula);
    if (!formula) {
        throw std::invalid_argument("Parsing failed: \"" + fol_formula + "\" is not a valid first order logic formula");
    }

    formula = simplify_constraints(close(pnf(formula)));
    std::cout << formula_to_string(*formula) << std::endl;

    const auto var_map = collect_variables(formula);
    for (const auto &[k, v] : var_map) {
        std::cout << k << ": " << v << std::endl;
    }

    return false;
}

std::map<std::string, std::size_t> collect_variables(std::shared_ptr<Formula> formula)
{
    std::map<std::string, std::size_t> var_map;

    std::size_t counter = 0;
    auto current = formula;
    while (true) {
        if (const auto *uni = std::get_if<UniversalQuantification>(current.get())) {
            var_map[uni->var_symbol] = counter++;
            current = uni->formula;
        } else if (const auto *exi = std::get_if<ExistentialQuantification>(current.get())) {
            var_map[exi->var_symbol] = counter++;
            current = exi->formula;
        } else {
            break;
        }
    }

    return var_map;
}

std::shared_ptr<Formula> simplify_constraints(std::shared_ptr<Atom> atom)
{
    return std::visit(
        overloaded{
            [&atom](const EqualTo &node) {
                return f_ptr<AtomWrapper>(atom);
            },
            [&atom](const LessThan &node) {
                return f_ptr<AtomWrapper>(atom);
            },
            [](const LessOrEqualTo &node) {
                return f_ptr<Disjunction>(
                    f_ptr<AtomWrapper>(a_ptr<LessThan>(node.left, node.right)),
                    f_ptr<AtomWrapper>(a_ptr<EqualTo>(node.left, node.right))
                );
            },
            [&atom](const GreaterThan &node) {
                return f_ptr<AtomWrapper>(atom);
            },
            [](const GreaterOrEqualTo &node) {
                return f_ptr<Disjunction>(
                    f_ptr<AtomWrapper>(a_ptr<GreaterThan>(node.left, node.right)),
                    f_ptr<AtomWrapper>(a_ptr<EqualTo>(node.left, node.right))
                );
            },
            [](const NotEqualTo &node) {
                return f_ptr<Disjunction>(
                    f_ptr<AtomWrapper>(a_ptr<LessThan>(node.left, node.right)),
                    f_ptr<AtomWrapper>(a_ptr<LessThan>(node.right, node.left))
                );
            }
        }, *atom
    );
}

std::shared_ptr<Formula> simplify_negated_constraints(std::shared_ptr<Atom> atom)
{
    return std::visit(
        overloaded{
            [](const EqualTo &node) {
                return simplify_constraints(a_ptr<NotEqualTo>(node.left, node.right));
            },
            [](const LessThan &node) {
                return simplify_constraints(a_ptr<GreaterOrEqualTo>(node.left, node.right));
            },
            [](const LessOrEqualTo &node) {
                return f_ptr<AtomWrapper>(a_ptr<GreaterThan>(node.left, node.right));
            },
            [](const GreaterThan &node) {
                return simplify_constraints(a_ptr<LessOrEqualTo>(node.left, node.right));
            },
            [](const GreaterOrEqualTo &node) {
                return f_ptr<AtomWrapper>(a_ptr<LessThan>(node.left, node.right));
            },
            [](const NotEqualTo &node) {
                return f_ptr<AtomWrapper>(a_ptr<EqualTo>(node.left, node.right));
            }
        }, *atom
    );
}

std::shared_ptr<Formula> simplify_constraints(std::shared_ptr<Formula> formula)
{
    return std::visit(
        overloaded{
            [](const AtomWrapper &node) {
                return simplify_constraints(node.atom);
            },
            [&formula](const LogicConstant &node) {
                return formula;
            },
            [](const Negation &node) {
                if (const auto *atom = std::get_if<AtomWrapper>(node.operand.get())) {
                    return simplify_negated_constraints(atom->atom);
                } else {
                    return f_ptr<Negation>(simplify_constraints(node.operand));
                }
            },
            [](const Conjuction &node) {
                return f_ptr<Conjuction>(simplify_constraints(node.left), simplify_constraints(node.right));
            },
            [](const Disjunction &node) {
                return f_ptr<Disjunction>(simplify_constraints(node.left), simplify_constraints(node.right));
            },
            [](const Implication &node) {
                return f_ptr<Implication>(simplify_constraints(node.left), simplify_constraints(node.right));
            },
            [](const Equivalence &node) {
                return f_ptr<Equivalence>(simplify_constraints(node.left), simplify_constraints(node.right));
            },
            [](const UniversalQuantification &node) {
                return f_ptr<UniversalQuantification>(node.var_symbol, simplify_constraints(node.formula));
            },
            [](const ExistentialQuantification &node) {
                return f_ptr<ExistentialQuantification>(node.var_symbol, simplify_constraints(node.formula));
            }
        }, *formula
    );
}
