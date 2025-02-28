#include "theorem_prover.hpp"
#include "fourier_motzkin.hpp"

#include <stdexcept>
#include <cassert>

std::shared_ptr<Formula> eliminate_quantifiers(std::shared_ptr<Formula> formula, VariableMapping &var_map);

bool TheoremProver::is_theorem(const std::string &fol_formula) const
{
    auto formula = m_driver.parse(fol_formula);
    if (!formula) {
        throw std::invalid_argument("Parsing failed: \"" + fol_formula + "\" is not a valid first order logic formula");
    }

    formula = close(pnf(formula));
    std::cout << formula_to_string(*formula) << std::endl;

    VariableMapping var_map;
    formula = eliminate_quantifiers(formula, var_map);

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

std::shared_ptr<Formula> eliminate_quantifiers(std::shared_ptr<Formula> formula, VariableMapping &var_map)
{
    return std::visit(
        overloaded{
            [&formula](const AtomWrapper &node) {
                return formula;
            },
            [&formula](const LogicConstant &node) {
                return formula;
            },
            [&formula](const Negation &node) {
                return formula;
            },
            [&formula](const Conjuction &node) {
                return formula;
            },
            [&formula](const Disjunction &node) {
                return formula;
            },
            [&formula](const Implication &node) {
                assert(!"Unreachable");
                return formula;
            },
            [&formula](const Equivalence &node) {
                assert(!"Unreachable");
                return formula;
            },
            [&var_map](const UniversalQuantification &node) {
                var_map.add_variable(node.var_symbol);
                const auto subformula = eliminate_quantifiers(node.formula, var_map);
                const auto base = dnf(simplify_constraints(nnf(f_ptr<Negation>(subformula))));
                var_map.remove_variable(node.var_symbol);
                return f_ptr<Negation>(base);
            },
            [&var_map](const ExistentialQuantification &node) {
                var_map.add_variable(node.var_symbol);
                const auto subformula = eliminate_quantifiers(node.formula, var_map);
                const auto base = dnf(simplify_constraints(nnf(subformula)));
                // Convert to FM form
                // Elim. var
                // Convert back to AST form
                var_map.remove_variable(node.var_symbol);
                return base;
            }
        }, *formula
    );
}

void collect_coefficients(std::shared_ptr<Term> term, std::vector<Fraction> &lhs, Fraction &rhs, bool on_left)
{

}

Constraint<Fraction> atom_to_constraint(std::shared_ptr<Atom> atom, const VariableMapping &var_map)
{
    return std::visit(
        overloaded{
            [](const EqualTo &node) {
                std::vector<Fraction> lhs;
                Fraction rhs;
                collect_coefficients(node.left, lhs, rhs, true);
                collect_coefficients(node.right, lhs, rhs, false);
                return Constraint<Fraction>(lhs, Constraint<Fraction>::Relation::EQ, rhs);
            },
            [](const LessThan &node) {
                return Constraint<Fraction>({}, Constraint<Fraction>::Relation::LT, Fraction{});
            },
            [](const LessOrEqualTo &node) {
                assert(!"Unreachable");
                return Constraint<Fraction>({}, Constraint<Fraction>::Relation::EQ, Fraction{});
            },
            [](const GreaterThan &node) {
                return Constraint<Fraction>({}, Constraint<Fraction>::Relation::GT, Fraction{});
            },
            [](const GreaterOrEqualTo &node) {
                assert(!"Unreachable");
                return Constraint<Fraction>({}, Constraint<Fraction>::Relation::EQ, Fraction{});
            },
            [](const NotEqualTo &node) {
                assert(!"Unreachable");
                return Constraint<Fraction>({}, Constraint<Fraction>::Relation::EQ, Fraction{});
            }
        }, *atom
    );
}
