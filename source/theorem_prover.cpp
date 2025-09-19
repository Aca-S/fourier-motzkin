#include "theorem_prover.hpp"
#include "fourier_motzkin.hpp"
#include "fol_string_conversion.hpp"
#include "fol_normalization.hpp"

#include <stdexcept>
#include <cassert>
#include <iomanip>

TheoremProver::TheoremProver(std::ostream &log)
    : m_log(log)
{

}

static bool evaluate(const std::shared_ptr<Formula> formula);

bool TheoremProver::is_theorem(const std::string &fol_formula) const
{
    auto formula = string_to_formula(fol_formula);
    if (!formula) {
        throw std::invalid_argument("Parsing failed: \"" + fol_formula + "\" is not a valid first order logic formula");
    }
    m_log << "========== [PROOF START] ==========" << std::endl;
    m_log << "[FORMULA] " << formula_to_string(formula) << std::endl;
    formula = close(pnf(formula));
    m_log << "[CLOSED PRENEX] " << formula_to_string(formula) << std::endl;
    VariableMapping var_map;
    formula = eliminate_quantifiers(formula, var_map);
    m_log << "[QUANTIFIER FREE FORM] " << formula_to_string(formula) << std::endl;
    bool result = evaluate(formula);
    m_log << "[RESULT] " << (result ? "Formula is a theorem" : "Formula is not a theorem") << std::endl;
    m_log << "=========== [PROOF END] ===========" << std::endl;
    return result;
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

static std::shared_ptr<Formula> simplify_constraints(std::shared_ptr<Atom> atom)
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

static std::shared_ptr<Formula> simplify_negated_constraints(std::shared_ptr<Atom> atom)
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

static std::shared_ptr<Formula> simplify_constraints(std::shared_ptr<Formula> formula)
{
    return std::visit(
        overloaded{
            [](const AtomWrapper &node) {
                return simplify_constraints(node.atom);
            },
            [&formula](const True &node) {
                return formula;
            },
            [&formula](const False &node) {
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

static std::vector<ConstraintConjuction<Fraction>> formula_to_constraints(std::shared_ptr<Formula> formula, const VariableMapping &var_map);
static std::shared_ptr<Formula> constraints_to_formula(const std::vector<ConstraintConjuction<Fraction>> &constraints, const VariableMapping &var_map);

std::shared_ptr<Formula> TheoremProver::eliminate_quantifiers(std::shared_ptr<Formula> formula, VariableMapping &var_map) const
{
    return std::visit(
        overloaded{
            [&formula](const AtomWrapper &node) {
                return formula;
            },
            [&formula](const True &node) {
                return formula;
            },
            [&formula](const False &node) {
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
            [this, &var_map](const UniversalQuantification &node) {
                return eliminate_variable(node.formula, node.var_symbol, var_map, false);
            },
            [this, &var_map](const ExistentialQuantification &node) {
                return eliminate_variable(node.formula, node.var_symbol, var_map, true);
            },
            [&formula](const auto &node) {
                assert(!"Unreachable");
                return formula;
            }
        }, *formula
    );
}

std::shared_ptr<Formula> TheoremProver::eliminate_variable(std::shared_ptr<Formula> base_formula, const std::string &quantified_variable, VariableMapping &var_map, bool is_existential) const
{
    var_map.add_variable(quantified_variable);
    base_formula = eliminate_quantifiers(base_formula, var_map);
    m_log << "[VARIABLE ELIMINATION] Eliminating " << (is_existential ? "existentially" : "universally") << " bound variable " << "\"" << quantified_variable << "\"" << std::endl;
    if (!is_existential) {
        base_formula = f_ptr<Negation>(base_formula);
    }
    m_log << "\tBase formula" << (is_existential ? "" : " (negated due to universal quantification)") << ": " << formula_to_string(base_formula) << std::endl;
    base_formula = dnf(simplify_constraints(nnf(base_formula)));
    m_log << "\tBase formula DNF: " << formula_to_string(base_formula) << std::endl;
    if (!std::holds_alternative<True>(*base_formula) && !std::holds_alternative<False>(*base_formula)) {
        auto constraints = formula_to_constraints(base_formula, var_map);
        for (auto &conjuction : constraints) {
            conjuction.eliminate_variable(var_map.get_variable_number(quantified_variable));
        }
        base_formula = constraints_to_formula(constraints, var_map);
        var_map.remove_variable(quantified_variable);
    }
    base_formula = is_existential ? base_formula : f_ptr<Negation>(base_formula);
    m_log << "\tNew base formula" << (is_existential ? "" : " (negated due to universal quantification)") << ": " << formula_to_string(base_formula) << std::endl;
    return base_formula;
}

static void collect_coefficients(std::shared_ptr<Term> term, std::vector<Fraction> &lhs, Fraction &rhs, const VariableMapping &var_map, bool flip_sign)
{
    std::visit(
        overloaded{
            [&rhs, flip_sign](const RationalNumber &node) {
                rhs = flip_sign ? rhs + node.value : rhs - node.value;
            },
            [&lhs, &var_map, flip_sign](const Variable &node) {
                const auto var_num = var_map.get_variable_number(node.symbol);
                lhs[var_num] = flip_sign ? lhs[var_num] - node.coef : lhs[var_num] + node.coef;
            },
            [&lhs, &rhs, &var_map, flip_sign](const Addition &node) {
                collect_coefficients(node.left, lhs, rhs, var_map, flip_sign);
                collect_coefficients(node.right, lhs, rhs, var_map, flip_sign);
            },
            [&lhs, &rhs, &var_map, flip_sign](const Subtraction &node) {
                collect_coefficients(node.left, lhs, rhs, var_map, flip_sign);
                collect_coefficients(node.right, lhs, rhs, var_map, !flip_sign);
            }
        }, *term
    );
}

static Constraint<Fraction> atom_to_constraint(std::shared_ptr<Atom> atom, const VariableMapping &var_map)
{
    return std::visit(
        overloaded{
            [&var_map](const EqualTo &node) {
                std::vector<Fraction> lhs(var_map.size());
                Fraction rhs;
                collect_coefficients(node.left, lhs, rhs, var_map, false);
                collect_coefficients(node.right, lhs, rhs, var_map, true);
                return Constraint<Fraction>(lhs, Constraint<Fraction>::Relation::EQ, rhs);
            },
            [&var_map](const LessThan &node) {
                std::vector<Fraction> lhs(var_map.size());
                Fraction rhs;
                collect_coefficients(node.left, lhs, rhs, var_map, false);
                collect_coefficients(node.right, lhs, rhs, var_map, true);
                return Constraint<Fraction>(lhs, Constraint<Fraction>::Relation::LT, rhs);
            },
            [&var_map](const GreaterThan &node) {
                std::vector<Fraction> lhs(var_map.size());
                Fraction rhs;
                collect_coefficients(node.left, lhs, rhs, var_map, false);
                collect_coefficients(node.right, lhs, rhs, var_map, true);
                return Constraint<Fraction>(lhs, Constraint<Fraction>::Relation::GT, rhs);
            },
            [](const auto &node) {
                assert(!"Unreachable");
                return Constraint<Fraction>({}, Constraint<Fraction>::Relation::EQ, Fraction{});
            }
        }, *atom
    );
}

static ConstraintConjuction<Fraction> conjuction_to_constraints(std::shared_ptr<Formula> formula, const VariableMapping &var_map)
{
    return std::visit(
        overloaded{
            [&var_map](const AtomWrapper &node) {
                return ConstraintConjuction<Fraction>({atom_to_constraint(node.atom, var_map)});
            },
            [&var_map](const Conjuction &node) {
                const auto left = conjuction_to_constraints(node.left, var_map).get_constraints();
                const auto right = conjuction_to_constraints(node.right, var_map).get_constraints();
                std::vector<Constraint<Fraction>> left_right_concat;
                left_right_concat.reserve(left.size() + right.size());
                left_right_concat.insert(left_right_concat.end(), left.begin(), left.end());
                left_right_concat.insert(left_right_concat.end(), right.begin(), right.end());
                return ConstraintConjuction<Fraction>(left_right_concat);
            },
            [](const auto &node) {
                assert(!"Unreachable");
                return ConstraintConjuction<Fraction>({});
            }
        }, *formula
    );
}

static std::vector<ConstraintConjuction<Fraction>> formula_to_constraints(std::shared_ptr<Formula> formula, const VariableMapping &var_map)
{
    return std::visit(
        overloaded{
            [&formula, &var_map](const AtomWrapper &node) {
                return std::vector<ConstraintConjuction<Fraction>>({conjuction_to_constraints(formula, var_map)});
            },
            [&formula, &var_map](const Conjuction &node) {
                return std::vector<ConstraintConjuction<Fraction>>({conjuction_to_constraints(formula, var_map)});
            },
            [&var_map](const Disjunction &node) {
                const auto left = formula_to_constraints(node.left, var_map);
                const auto right = formula_to_constraints(node.right, var_map);
                std::vector<ConstraintConjuction<Fraction>> left_right_concat;
                left_right_concat.reserve(left.size() + right.size());
                left_right_concat.insert(left_right_concat.end(), left.begin(), left.end());
                left_right_concat.insert(left_right_concat.end(), right.begin(), right.end());
                return left_right_concat;
            },
            [](const auto &node) {
                assert(!"Unreachable");
                return std::vector<ConstraintConjuction<Fraction>>();
            }
        }, *formula
    );
}

static std::shared_ptr<Formula> constraint_to_formula(const Constraint<Fraction> &constraint, const VariableMapping &var_map)
{
    const auto &lhs = constraint.get_lhs();
    std::shared_ptr<Term> left = t_ptr<RationalNumber>(0);
    for (std::size_t var_num = 0; var_num < lhs.size(); var_num++) {
        const auto coef = lhs[var_num];
        const auto var = var_map.get_variable_symbol(var_num);
        if (coef > 0) {
            left = t_ptr<Addition>(left, t_ptr<Variable>(coef, var));
        } else if (coef < 0) {
            left = t_ptr<Subtraction>(left, t_ptr<Variable>(-coef, var));
        }
    }
    const auto right = t_ptr<RationalNumber>(constraint.get_rhs());
    if (constraint.get_relation() == Constraint<Fraction>::Relation::LT) {
        return f_ptr<AtomWrapper>(a_ptr<LessThan>(left, right));
    } else if (constraint.get_relation() == Constraint<Fraction>::Relation::GT) {
        return f_ptr<AtomWrapper>(a_ptr<GreaterThan>(left, right));
    } else {
        return f_ptr<AtomWrapper>(a_ptr<EqualTo>(left, right));
    }
}

static std::shared_ptr<Formula> conjuction_to_formula(const ConstraintConjuction<Fraction> &conjuction, const VariableMapping &var_map)
{
    const auto &constraints = conjuction.get_constraints();
    if (constraints.size() == 0) {
        return f_ptr<True>();
    } else {
        auto acc = constraint_to_formula(constraints[0], var_map);
        for (std::size_t i = 1; i < constraints.size(); i++) {
            acc = f_ptr<Conjuction>(acc, constraint_to_formula(constraints[i], var_map));
        }
        return acc;
    }
}

static std::shared_ptr<Formula> constraints_to_formula(const std::vector<ConstraintConjuction<Fraction>> &constraints, const VariableMapping &var_map)
{
    if (constraints.size() == 0) {
        return f_ptr<False>();
    } else {
        auto acc = conjuction_to_formula(constraints[0], var_map);
        for (std::size_t i = 1; i < constraints.size(); i++) {
            acc = f_ptr<Disjunction>(acc, conjuction_to_formula(constraints[i], var_map));
        }
        return acc;
    }
}

static Fraction evaluate(const std::shared_ptr<Term> term)
{
    return std::visit(
        overloaded{
            [](const RationalNumber &node) {
                return node.value;
            },
            [](const Addition &node) {
                return evaluate(node.left) + evaluate(node.right);
            },
            [](const Subtraction &node) {
                return evaluate(node.left) - evaluate(node.right);
            },
            [](const auto &node) {
                assert(!"Unreachable");
                return Fraction{};
            }
        }, *term
    );
}

static bool evaluate(const std::shared_ptr<Atom> atom)
{
    return std::visit(
        overloaded{
            [](const EqualTo &node) {
                return evaluate(node.left) == evaluate(node.right);
            },
            [](const LessThan &node) {
                return evaluate(node.left) < evaluate(node.right);
            },
            [](const LessOrEqualTo &node) {
                return evaluate(node.left) <= evaluate(node.right);
            },
            [](const GreaterThan &node) {
                return evaluate(node.left) > evaluate(node.right);
            },
            [](const GreaterOrEqualTo &node) {
                return evaluate(node.left) >= evaluate(node.right);
            },
            [](const NotEqualTo &node) {
                return evaluate(node.left) != evaluate(node.right);
            }
        }, *atom
    );
}

static bool evaluate(const std::shared_ptr<Formula> formula)
{
    return std::visit(
        overloaded{
            [](const AtomWrapper &node) {
                return evaluate(node.atom);
            },
            [](const True &node) {
                return true;
            },
            [](const False &node) {
                return false;
            },
            [](const Negation &node) {
                return !evaluate(node.operand);
            },
            [](const Conjuction &node) {
                return evaluate(node.left) && evaluate(node.right);
            },
            [](const Disjunction &node) {
                return evaluate(node.left) || evaluate(node.right);
            },
            [](const Implication &node) {
                if (evaluate(node.left)) {
                    return evaluate(node.right);
                } else {
                    return true;
                }
            },
            [](const Equivalence &node) {
                return evaluate(node.left) == evaluate(node.right);
            },
            [](const auto &node) {
                assert(!"Unreachable");
                return false;
            }
        }, *formula
    );
}
