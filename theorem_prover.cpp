#include "theorem_prover.hpp"
#include "fourier_motzkin.hpp"

#include <stdexcept>
#include <cassert>

VariableMapping collect_variables(std::shared_ptr<Formula> formula);
std::shared_ptr<Formula> convert_universal_to_existential(std::shared_ptr<Formula> formula);
std::shared_ptr<Formula> simplify_constraints(std::shared_ptr<Formula> formula);

bool TheoremProver::is_theorem(const std::string &fol_formula) const
{
    auto formula = m_driver.parse(fol_formula);
    if (!formula) {
        throw std::invalid_argument("Parsing failed: \"" + fol_formula + "\" is not a valid first order logic formula");
    }

    formula = close(pnf(formula));
    const auto var_map = collect_variables(formula);
    formula = convert_universal_to_existential(formula);

    std::cout << formula_to_string(*formula) << std::endl;
    std::cout << var_map.size() << std::endl;

    return false;
}

void VariableMapping::add_variable(const std::string &variable_symbol)
{
    if (!m_symbol_to_number.contains(variable_symbol)) {
        const auto variable_number = size();
        m_number_to_symbol.insert({variable_number, variable_symbol});
        m_symbol_to_number.insert({variable_symbol, variable_number});
    }
}

void VariableMapping::remove_variable(const std::string &variable_symbol)
{
    assert(m_symbol_to_number.contains(variable_symbol));
    const auto variable_number = get_variable_number(variable_symbol);
    m_symbol_to_number.erase(variable_symbol);
    m_number_to_symbol.erase(variable_number);
}

std::size_t VariableMapping::get_variable_number(const std::string &variable_symbol) const
{
    assert(m_symbol_to_number.contains(variable_symbol));
    return m_symbol_to_number.find(variable_symbol)->second;
}

std::string VariableMapping::get_variable_symbol(std::size_t variable_number) const
{
    assert(m_number_to_symbol.contains(variable_number));
    return m_number_to_symbol.find(variable_number)->second;
}

std::size_t VariableMapping::size() const
{
    return m_symbol_to_number.size();
}

VariableMapping collect_variables(std::shared_ptr<Formula> formula)
{
    VariableMapping var_map;
    auto current = formula;
    while (true) {
        if (const auto *uni = std::get_if<UniversalQuantification>(current.get())) {
            var_map.add_variable(uni->var_symbol);
            current = uni->formula;
        } else if (const auto *exi = std::get_if<ExistentialQuantification>(current.get())) {
            var_map.add_variable(exi->var_symbol);
            current = uni->formula;
        } else {
            break;
        }
    }
    return var_map;
}

std::shared_ptr<Formula> convert_universal_to_existential(std::shared_ptr<Formula> formula)
{
    return std::visit(
        overloaded{
            [&formula](const AtomWrapper &node) {
                return formula;
            },
            [&formula](const LogicConstant &node) {
                return formula;
            },
            [](const Negation &node) {
                return f_ptr<Negation>(convert_universal_to_existential(node.operand));
            },
            [](const Conjuction &node) {
                return f_ptr<Conjuction>(convert_universal_to_existential(node.left), convert_universal_to_existential(node.right));
            },
            [](const Disjunction &node) {
                return f_ptr<Disjunction>(convert_universal_to_existential(node.left), convert_universal_to_existential(node.right));
            },
            [](const Implication &node) {
                return f_ptr<Implication>(convert_universal_to_existential(node.left), convert_universal_to_existential(node.right));
            },
            [](const Equivalence &node) {
                return f_ptr<Equivalence>(convert_universal_to_existential(node.left), convert_universal_to_existential(node.right));
            },
            [](const UniversalQuantification &node) {
                const auto subformula = convert_universal_to_existential(node.formula);
                if (const auto *neg = std::get_if<Negation>(subformula.get())) {
                    return f_ptr<Negation>(f_ptr<ExistentialQuantification>(node.var_symbol, neg->operand));
                } else {
                    return f_ptr<Negation>(f_ptr<ExistentialQuantification>(node.var_symbol, f_ptr<Negation>(subformula)));
                }
            },
            [](const ExistentialQuantification &node) {
                return f_ptr<ExistentialQuantification>(node.var_symbol, convert_universal_to_existential(node.formula));
            }
        }, *formula
    );
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
