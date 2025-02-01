#include "fol_ast.hpp"

#include <ostream>
#include <variant>

unsigned precedence(const Term &term)
{
    return std::visit(
        overloaded{
            [](const RationalNumber &rn) {
                return 2;
            },
            [](const Variable &var) {
                return 2;
            },
            [](const Addition &add) {
                return 0;
            },
            [](const Subtraction &sub) {
                return 0;
            },
            [](const Multiplication &mul) {
                return 1;
            },
            [](const Division &div) {
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
            [](const RationalNumber &rn) {
                const auto num = rn.value.get_numerator();
                const auto den = rn.value.get_denominator();
                return std::to_string(num) + (den == 1 ? "" : "/" + std::to_string(den));
            },
            [](const Variable &var) {
                return std::string(1, var.symbol);
            },
            [&term](const Addition &add) {
                return term_to_string(*add.left, term) + "+" + term_to_string(*add.right, term);
            },
            [&term](const Subtraction &sub) {
                return term_to_string(*sub.left, term) + "-" + term_to_string(*sub.right, term);
            },
            [&term](const Multiplication &mul) {
                return term_to_string(*mul.left, term) + "*" + term_to_string(*mul.right, term);
            },
            [&term](const Division &div) {
                return term_to_string(*div.left, term) + "/" + term_to_string(*div.right, term);
            }
        }, term
    );
}

std::string atomic_formula_to_string(const AtomicFormula &atomic_formula)
{
    return std::visit(
        overloaded{
            [](const LogicalConstant &atomic_formula) {
                return atomic_formula.value ? std::string("T") : std::string("F");
            },
            [](const EqualTo &atomic_formula) {
                return term_to_string(*atomic_formula.left) + "=" + term_to_string(*atomic_formula.right);
            },
            [](const LessThan &atomic_formula) {
                return term_to_string(*atomic_formula.left) + "<" + term_to_string(*atomic_formula.right);
            },
            [](const LessOrEqualTo &atomic_formula) {
                return term_to_string(*atomic_formula.left) + "<=" + term_to_string(*atomic_formula.right);
            },
            [](const GreaterThan &atomic_formula) {
                return term_to_string(*atomic_formula.left) + ">" + term_to_string(*atomic_formula.right);
            },
            [](const GreaterOrEqualTo &atomic_formula) {
                return term_to_string(*atomic_formula.left) + ">=" + term_to_string(*atomic_formula.right);
            }
        }, atomic_formula
    );
}

unsigned precedence(const Formula &formula)
{
    return std::visit(
        overloaded{
            [](const AtomicFormulaWrapper &afw) {
                return 6;
            },
            [](const Negation &neg) {
                return 5;
            },
            [](const Conjuction &con) {
                return 4;
            },
            [](const Disjunction &dis) {
                return 3;
            },
            [](const Implication &impl) {
                return 2;
            },
            [](const Equivalence &equi) {
                return 1;
            },
            [](const UniversalQuantification &uni) {
                return 0;
            },
            [](const ExistentialQuantification &exi) {
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
            [](const AtomicFormulaWrapper &afw) {
                return atomic_formula_to_string(*afw.atomic_formula);
            },
            [&formula](const Negation &neg) {
                return "~" + formula_to_string(*neg.operand, formula);
            },
            [&formula](const Conjuction &con) {
                return formula_to_string(*con.left, formula) + " & " + formula_to_string(*con.right, formula);
            },
            [&formula](const Disjunction &dis) {
                return formula_to_string(*dis.left, formula) + " | " + formula_to_string(*dis.right, formula);
            },
            [&formula](const Implication &impl) {
                return formula_to_string(*impl.left, formula) + " => " + formula_to_string(*impl.right, formula);
            },
            [&formula](const Equivalence &equi) {
                return formula_to_string(*equi.left, formula) + " <=> " + formula_to_string(*equi.right, formula);
            },
            [&formula](const UniversalQuantification &uni) {
                return "!" + std::string(1, uni.var_symbol) + "." + formula_to_string(*uni.formula, formula);
            },
            [&formula](const ExistentialQuantification &exi) {
                return "?" + std::string(1, exi.var_symbol) + "." + formula_to_string(*exi.formula, formula);
            }
        }, formula
    );
}
