#include "fol_ast.hpp"

#include <ostream>
#include <variant>

// For overloaded lambdas...
template <typename... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;


template <typename T, typename... Args>
std::shared_ptr<Formula> f_ptr(Args&&... args)
{
    return std::make_shared<Formula>(T(args...));
}

unsigned precedence(const Term &term)
{
    return std::visit(
        overloaded{
            [](const RationalNumber &node) {
                return 2;
            },
            [](const Variable &node) {
                return 2;
            },
            [](const Addition &node) {
                return 0;
            },
            [](const Subtraction &node) {
                return 0;
            },
            [](const Multiplication &node) {
                return 1;
            },
            [](const Division &node) {
                return 1;
            }
        }, term
    );
}

std::string term_to_string(const Term &term);

std::string term_to_string(const Term &term, const Term &parent)
{
    if (precedence(term) < precedence(parent)) {
        return "(" + term_to_string(term) + ")";
    } else {
        return term_to_string(term);
    }
}

std::string term_to_string(const Term &term)
{
    return std::visit(
        overloaded{
            [](const RationalNumber &node) {
                const auto num = node.value.get_numerator();
                const auto den = node.value.get_denominator();
                return std::to_string(num) + (den == 1 ? "" : "/" + std::to_string(den));
            },
            [](const Variable &node) {
                return std::string(1, node.symbol);
            },
            [&term](const Addition &node) {
                return term_to_string(*node.left, term) + "+" + term_to_string(*node.right, term);
            },
            [&term](const Subtraction &node) {
                return term_to_string(*node.left, term) + "-" + term_to_string(*node.right, term);
            },
            [&term](const Multiplication &node) {
                return term_to_string(*node.left, term) + "*" + term_to_string(*node.right, term);
            },
            [&term](const Division &node) {
                return term_to_string(*node.left, term) + "/" + term_to_string(*node.right, term);
            }
        }, term
    );
}

std::string atomic_formula_to_string(const Atom &atom)
{
    return std::visit(
        overloaded{
            [](const EqualTo &node) {
                return term_to_string(*node.left) + "=" + term_to_string(*node.right);
            },
            [](const LessThan &node) {
                return term_to_string(*node.left) + "<" + term_to_string(*node.right);
            },
            [](const LessOrEqualTo &node) {
                return term_to_string(*node.left) + "<=" + term_to_string(*node.right);
            },
            [](const GreaterThan &node) {
                return term_to_string(*node.left) + ">" + term_to_string(*node.right);
            },
            [](const GreaterOrEqualTo &node) {
                return term_to_string(*node.left) + ">=" + term_to_string(*node.right);
            },
            [](const NotEqualTo &node) {
                return term_to_string(*node.left) + "!=" + term_to_string(*node.right);
            }
        }, atom
    );
}

unsigned precedence(const Formula &formula)
{
    return std::visit(
        overloaded{
            [](const AtomWrapper &node) {
                return 6;
            },
            [](const LogicConstant &node) {
                return 6;
            },
            [](const Negation &node) {
                return 5;
            },
            [](const Conjuction &node) {
                return 4;
            },
            [](const Disjunction &node) {
                return 3;
            },
            [](const Implication &node) {
                return 2;
            },
            [](const Equivalence &node) {
                return 1;
            },
            [](const UniversalQuantification &node) {
                return 0;
            },
            [](const ExistentialQuantification &node) {
                return 0;
            }
        }, formula
    );
}

std::string formula_to_string(const Formula &formula);

std::string formula_to_string(const Formula &formula, const Formula &parent)
{
    if (precedence(formula) < precedence(parent)) {
        return "(" + formula_to_string(formula) + ")";
    } else {
        return formula_to_string(formula);
    }
}

std::string formula_to_string(const Formula &formula)
{
    return std::visit(
        overloaded{
            [](const AtomWrapper &node) {
                return atomic_formula_to_string(*node.atom);
            },
            [](const LogicConstant &node) {
                return node.value ? std::string("T") : std::string("F");
            },
            [&formula](const Negation &node) {
                return "~" + formula_to_string(*node.operand, formula);
            },
            [&formula](const Conjuction &node) {
                return formula_to_string(*node.left, formula) + " & " + formula_to_string(*node.right, formula);
            },
            [&formula](const Disjunction &node) {
                return formula_to_string(*node.left, formula) + " | " + formula_to_string(*node.right, formula);
            },
            [&formula](const Implication &node) {
                return formula_to_string(*node.left, formula) + " => " + formula_to_string(*node.right, formula);
            },
            [&formula](const Equivalence &node) {
                return formula_to_string(*node.left, formula) + " <=> " + formula_to_string(*node.right, formula);
            },
            [&formula](const UniversalQuantification &node) {
                return "!" + std::string(1, node.var_symbol) + "." + formula_to_string(*node.formula, formula);
            },
            [&formula](const ExistentialQuantification &node) {
                return "?" + std::string(1, node.var_symbol) + "." + formula_to_string(*node.formula, formula);
            }
        }, formula
    );
}

std::shared_ptr<Formula> simplify(std::shared_ptr<Formula> formula)
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
                const auto subformula = simplify(node.operand);
                if (const auto *lc = std::get_if<LogicConstant>(subformula.get())) {
                    return f_ptr<LogicConstant>(!lc->value);
                } else {
                    return f_ptr<Negation>(subformula);
                }
            },
            [](const Conjuction &node) {
                const auto l_subformula = simplify(node.left);
                const auto r_subformula = simplify(node.right);
                if (const auto *lc = std::get_if<LogicConstant>(l_subformula.get())) {
                    return lc->value
                        ? r_subformula
                        : f_ptr<LogicConstant>(false);
                } else if (const auto *lc = std::get_if<LogicConstant>(r_subformula.get())) {
                    return lc->value
                        ? l_subformula
                        : f_ptr<LogicConstant>(false);
                } else {
                    return f_ptr<Conjuction>(l_subformula, r_subformula);
                }
            },
            [](const Disjunction &node) {
                const auto l_subformula = simplify(node.left);
                const auto r_subformula = simplify(node.right);
                if (const auto *lc = std::get_if<LogicConstant>(l_subformula.get())) {
                    return lc->value
                        ? f_ptr<LogicConstant>(true)
                        : r_subformula;
                } else if (const auto *lc = std::get_if<LogicConstant>(r_subformula.get())) {
                    return lc->value
                        ? f_ptr<LogicConstant>(true)
                        : l_subformula;
                } else {
                    return f_ptr<Disjunction>(l_subformula, r_subformula);
                }
            },
            [](const Implication &node) {
                const auto l_subformula = simplify(node.left);
                const auto r_subformula = simplify(node.right);
                if (const auto *lc = std::get_if<LogicConstant>(l_subformula.get())) {
                    return lc->value
                        ? r_subformula
                        : f_ptr<LogicConstant>(true);
                } else if (const auto *lc = std::get_if<LogicConstant>(r_subformula.get())) {
                    return lc->value
                        ? f_ptr<LogicConstant>(true)
                        : f_ptr<Negation>(l_subformula);
                } else {
                    return f_ptr<Implication>(l_subformula, r_subformula);
                }
            },
            [](const Equivalence &node) {
                const auto l_subformula = simplify(node.left);
                const auto r_subformula = simplify(node.right);
                if (const auto *lc = std::get_if<LogicConstant>(l_subformula.get())) {
                    return lc->value
                        ? r_subformula
                        : f_ptr<Negation>(r_subformula);
                } else if (const auto *lc = std::get_if<LogicConstant>(r_subformula.get())) {
                    return lc->value
                        ? l_subformula
                        : f_ptr<Negation>(l_subformula);
                } else {
                    return f_ptr<Equivalence>(l_subformula, r_subformula);
                }
            },
            [](const UniversalQuantification &node) {
                const auto subformula = simplify(node.formula);
                if (const auto *lc = std::get_if<LogicConstant>(subformula.get())) {
                    return subformula;
                } else {
                    return f_ptr<UniversalQuantification>(node.var_symbol, subformula);
                }
            },
            [](const ExistentialQuantification &node) {
                const auto subformula = simplify(node.formula);
                if (const auto *lc = std::get_if<LogicConstant>(subformula.get())) {
                    return subformula;
                } else {
                    return f_ptr<ExistentialQuantification>(node.var_symbol, subformula);
                }
            }
        }, *formula
    );
}

std::shared_ptr<Formula> nnf_not(std::shared_ptr<Formula> formula)
{
    return std::visit(
        overloaded{
            [&formula](const AtomWrapper &node) {
                return f_ptr<Negation>(formula);
            },
            [&formula](const LogicConstant &node) {
                return f_ptr<LogicConstant>(!node.value);
            },
            [](const Negation &node) {
                return nnf(node.operand);
            },
            [](const Conjuction &node) {
                return f_ptr<Disjunction>(nnf_not(node.left), nnf_not(node.right));
            },
            [](const Disjunction &node) {
                return f_ptr<Conjuction>(nnf_not(node.left), nnf_not(node.right));
            },
            [](const Implication &node) {
                return f_ptr<Conjuction>(nnf(node.left), nnf_not(node.right));;
            },
            [](const Equivalence &node) {
                return f_ptr<Conjuction>(
                    f_ptr<Disjunction>(nnf(node.left), nnf(node.right)),
                    f_ptr<Disjunction>(nnf_not(node.left), nnf_not(node.right))
                );
            },
            [](const UniversalQuantification &node) {
                return f_ptr<ExistentialQuantification>(node.var_symbol, nnf_not(node.formula));
            },
            [](const ExistentialQuantification &node) {
                return f_ptr<ExistentialQuantification>(node.var_symbol, nnf_not(node.formula));
            }
        }, *formula
    );
}

std::shared_ptr<Formula> nnf(std::shared_ptr<Formula> formula)
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
                return nnf_not(node.operand);
            },
            [](const Conjuction &node) {
                return f_ptr<Conjuction>(nnf(node.left), nnf(node.right));
            },
            [](const Disjunction &node) {
                return f_ptr<Disjunction>(nnf(node.left), nnf(node.right));
            },
            [](const Implication &node) {
                return f_ptr<Disjunction>(nnf_not(node.left), nnf(node.right));;
            },
            [](const Equivalence &node) {
                return f_ptr<Conjuction>(
                    f_ptr<Disjunction>(nnf(node.left), nnf_not(node.right)),
                    f_ptr<Disjunction>(nnf_not(node.left), nnf(node.right))
                );
            },
            [](const UniversalQuantification &node) {
                return f_ptr<UniversalQuantification>(node.var_symbol, nnf(node.formula));
            },
            [](const ExistentialQuantification &node) {
                return f_ptr<ExistentialQuantification>(node.var_symbol, nnf(node.formula));
            }
        }, *formula
    );
}
