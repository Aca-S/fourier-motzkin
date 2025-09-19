#include "fol_string_conversion.hpp"
#include "fol_driver.hpp"

static unsigned precedence(std::shared_ptr<Term> term)
{
    return std::visit(
        overloaded{
            [](const RationalNumber &node) {
                return 1;
            },
            [](const Variable &node) {
                return 1;
            },
            [](const Addition &node) {
                return 0;
            },
            [](const Subtraction &node) {
                return 0;
            }
        }, *term
    );
}

static std::string term_to_string(std::shared_ptr<Term> term)
{
    static constexpr auto wrap = [](const auto term, const auto parent) {
        if (precedence(term) < precedence(parent)) {
            return "(" + term_to_string(term) + ")";
        } else {
            return term_to_string(term);
        }
    };

    return std::visit(
        overloaded{
            [](const RationalNumber &node) {
                return static_cast<std::string>(node.value);
            },
            [](const Variable &node) {
                if (node.coef == Fraction(1, 1)) {
                    return node.symbol;
                } else if (node.coef == Fraction(-1, 1)) {
                    return "-" + node.symbol;
                } else {
                    return static_cast<std::string>(node.coef) + "*" + node.symbol;
                }
            },
            [&term](const Addition &node) {
                return wrap(node.left, term) + "+" + wrap(node.right, term);
            },
            [&term](const Subtraction &node) {
                return wrap(node.left, term) + "-" + wrap(node.right, term);
            }
        }, *term
    );
}

static std::string atomic_formula_to_string(std::shared_ptr<Atom> atom)
{
    return std::visit(
        overloaded{
            [](const EqualTo &node) {
                return term_to_string(node.left) + "=" + term_to_string(node.right);
            },
            [](const LessThan &node) {
                return term_to_string(node.left) + "<" + term_to_string(node.right);
            },
            [](const LessOrEqualTo &node) {
                return term_to_string(node.left) + "<=" + term_to_string(node.right);
            },
            [](const GreaterThan &node) {
                return term_to_string(node.left) + ">" + term_to_string(node.right);
            },
            [](const GreaterOrEqualTo &node) {
                return term_to_string(node.left) + ">=" + term_to_string(node.right);
            },
            [](const NotEqualTo &node) {
                return term_to_string(node.left) + "!=" + term_to_string(node.right);
            }
        }, *atom
    );
}

static unsigned precedence(std::shared_ptr<Formula> formula)
{
    return std::visit(
        overloaded{
            [](const AtomWrapper &node) {
                return 6;
            },
            [](const True &node) {
                return 6;
            },
            [](const False &node) {
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
        }, *formula
    );
}

std::string formula_to_string(std::shared_ptr<Formula> formula)
{
    static constexpr auto wrap = [](const auto formula, const auto parent) {
        if (precedence(formula) < precedence(parent)) {
            return "(" + formula_to_string(formula) + ")";
        } else {
            return formula_to_string(formula);
        }
    };

    return std::visit(
        overloaded{
            [](const AtomWrapper &node) {
                return atomic_formula_to_string(node.atom);
            },
            [](const True &node) {
                return std::string("T");
            },
            [](const False &node) {
                return std::string("F");
            },
            [&formula](const Negation &node) {
                return "~" + wrap(node.operand, formula);
            },
            [&formula](const Conjuction &node) {
                return wrap(node.left, formula) + " & " + wrap(node.right, formula);
            },
            [&formula](const Disjunction &node) {
                return wrap(node.left, formula) + " | " + wrap(node.right, formula);
            },
            [&formula](const Implication &node) {
                return wrap(node.left, formula) + " => " + wrap(node.right, formula);
            },
            [&formula](const Equivalence &node) {
                return wrap(node.left, formula) + " <=> " + wrap(node.right, formula);
            },
            [&formula](const UniversalQuantification &node) {
                return "!" + node.var_symbol + "." + wrap(node.formula, formula);
            },
            [&formula](const ExistentialQuantification &node) {
                return "?" + node.var_symbol + "." + wrap(node.formula, formula);
            }
        }, *formula
    );
}

std::shared_ptr<Formula> string_to_formula(const std::string &formula)
{
    FOLDriver driver;
    return driver.parse(formula);
}
