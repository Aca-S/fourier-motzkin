#include "fol_ast.hpp"

#include <ostream>
#include <variant>

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

void process_inequalities(Formula &formula)
{
    std::visit(
        overloaded{
            [&formula](const AtomWrapper &node) {
                auto &constraint = *node.atom;
                if (auto *leq = std::get_if<LessOrEqualTo>(&constraint)) {
                    auto lt = std::make_shared<Atom>(LessThan(leq->left, leq->right));
                    auto eq = std::make_shared<Atom>(EqualTo(leq->left, leq->right));
                    formula = Disjunction(
                        std::make_shared<Formula>(AtomWrapper(lt)),
                        std::make_shared<Formula>(AtomWrapper(eq))
                    );
                } else if (auto *geq = std::get_if<GreaterOrEqualTo>(&constraint)) {
                    auto gt = std::make_shared<Atom>(GreaterThan(geq->left, geq->right));
                    auto eq = std::make_shared<Atom>(EqualTo(geq->left, geq->right));
                    formula = Disjunction(
                        std::make_shared<Formula>(AtomWrapper(gt)),
                        std::make_shared<Formula>(AtomWrapper(eq))
                    );
                } else if (auto *neq = std::get_if<NotEqualTo>(&constraint)) {
                    auto lt = std::make_shared<Atom>(LessThan(neq->left, neq->right));
                    auto gt = std::make_shared<Atom>(GreaterThan(neq->left, neq->right));
                    formula = Disjunction(
                        std::make_shared<Formula>(AtomWrapper(lt)),
                        std::make_shared<Formula>(AtomWrapper(gt))
                    );
                }
            },
            [](const LogicConstant &node) {

            },
            [](const Negation &node) {
                process_inequalities(*node.operand);
            },
            [](const Conjuction &node) {
                process_inequalities(*node.left);
                process_inequalities(*node.right);
            },
            [](const Disjunction &node) {
                process_inequalities(*node.left);
                process_inequalities(*node.right);
            },
            [](const Implication &node) {
                process_inequalities(*node.left);
                process_inequalities(*node.right);
            },
            [](const Equivalence &node) {
                process_inequalities(*node.left);
                process_inequalities(*node.right);
            },
            [](const UniversalQuantification &node) {
                process_inequalities(*node.formula);
            },
            [](const ExistentialQuantification &node) {
                process_inequalities(*node.formula);
            }
        }, formula
    );
}

void eliminate_constants(Formula &formula)
{
    std::visit(
        overloaded{
            [](const AtomWrapper &node) {

            },
            [](const LogicConstant &node) {

            },
            [&formula](const Negation &node) {
                eliminate_constants(*node.operand);
                auto &operand = *node.operand;
                if (auto *constant = std::get_if<LogicConstant>(&operand)) {
                    formula = LogicConstant(!constant->value);
                }
            },
            [&formula](const Conjuction &node) {
                eliminate_constants(*node.left);
                eliminate_constants(*node.right);
                if (auto *constant = std::get_if<LogicConstant>(&*node.left)) {
                    formula = constant->value ? *node.right : LogicConstant(false);
                } else if (auto *constant = std::get_if<LogicConstant>(&*node.right)) {
                    formula = constant->value ? *node.left : LogicConstant(false);
                }
            },
            [&formula](const Disjunction &node) {
                eliminate_constants(*node.left);
                eliminate_constants(*node.right);
                if (auto *constant = std::get_if<LogicConstant>(&*node.left)) {
                    formula = constant->value ? LogicConstant(true) : *node.right;
                } else if (auto *constant = std::get_if<LogicConstant>(&*node.right)) {
                    formula = constant->value ? LogicConstant(true) : *node.left;
                }
            },
            [&formula](const Implication &node) {
                eliminate_constants(*node.left);
                eliminate_constants(*node.right);
                if (auto *constant = std::get_if<LogicConstant>(&*node.left)) {
                    formula = constant->value ? *node.right : LogicConstant(true);
                } else if (auto *constant = std::get_if<LogicConstant>(&*node.right)) {
                    if (constant->value) {
                        formula = LogicConstant(true);
                    } else {
                        formula = Negation(node.left);
                    }
                }
            },
            [&formula](const Equivalence &node) {
                eliminate_constants(*node.left);
                eliminate_constants(*node.right);
                if (auto *constant = std::get_if<LogicConstant>(&*node.left)) {
                    formula = constant->value ? *node.right : Negation(node.right);
                } else if (auto *constant = std::get_if<LogicConstant>(&*node.right)) {
                    formula = constant->value ? *node.left : Negation(node.left);
                }
            },
            [&formula](const UniversalQuantification &node) {
                eliminate_constants(*node.formula);
                if (auto *constant = std::get_if<LogicConstant>(&*node.formula)) {
                    formula = *constant;
                }
            },
            [&formula](const ExistentialQuantification &node) {
                eliminate_constants(*node.formula);
                if (auto *constant = std::get_if<LogicConstant>(&*node.formula)) {
                    formula = *constant;
                }
            }
        }, formula
    );
}

void eliminate_implications_and_equivalences(Formula &formula)
{
    std::visit(
        overloaded{
            [](const AtomWrapper &node) {

            },
            [](const LogicConstant &node) {

            },
            [](const Negation &node) {
                eliminate_implications_and_equivalences(*node.operand);
            },
            [](const Conjuction &node) {
                eliminate_implications_and_equivalences(*node.left);
                eliminate_implications_and_equivalences(*node.right);
            },
            [](const Disjunction &node) {
                eliminate_implications_and_equivalences(*node.left);
                eliminate_implications_and_equivalences(*node.right);
            },
            [&formula](const Implication &node) {
                eliminate_implications_and_equivalences(*node.left);
                eliminate_implications_and_equivalences(*node.right);
                formula = Disjunction(
                    std::make_shared<Formula>(Negation(node.left)),
                    node.right
                );
            },
            [&formula](const Equivalence &node) {
                eliminate_implications_and_equivalences(*node.left);
                eliminate_implications_and_equivalences(*node.right);
                formula = Disjunction(
                    std::make_shared<Formula>(Conjuction(node.left, node.right)),
                    std::make_shared<Formula>(Conjuction(
                        std::make_shared<Formula>(Negation(node.left)),
                        std::make_shared<Formula>(Negation(node.right))
                    ))
                );
            },
            [](const UniversalQuantification &node) {
                eliminate_implications_and_equivalences(*node.formula);
            },
            [](const ExistentialQuantification &node) {
                eliminate_implications_and_equivalences(*node.formula);
            }
        }, formula
    );
}

void push_negations(Formula &formula)
{
    std::visit(
        overloaded{
            [](const AtomWrapper &node) {

            },
            [](const LogicConstant &node) {

            },
            [&formula](const Negation &node) {
                if (auto *conjuction = std::get_if<Conjuction>(&*node.operand)) {
                    auto disjunction = Disjunction(
                        std::make_shared<Formula>(Negation(conjuction->left)),
                        std::make_shared<Formula>(Negation(conjuction->right))
                    );
                    push_negations(*disjunction.left);
                    push_negations(*disjunction.right);
                    formula = disjunction;
                } else if (auto *disjunction = std::get_if<Disjunction>(&*node.operand)) {
                    auto conjuction = Conjuction(
                        std::make_shared<Formula>(Negation(disjunction->left)),
                        std::make_shared<Formula>(Negation(disjunction->right))
                    );
                    push_negations(*conjuction.left);
                    push_negations(*conjuction.right);
                    formula = conjuction;
                } else if (auto *negation = std::get_if<Negation>(&*node.operand)) {
                    auto nested_formula = *negation->operand;
                    push_negations(nested_formula);
                    formula = nested_formula;
                } else if (auto *universal_quant = std::get_if<UniversalQuantification>(&*node.operand)) {
                    auto existential_quant = ExistentialQuantification(
                        universal_quant->var_symbol,
                        std::make_shared<Formula>(Negation(universal_quant->formula))
                    );
                    push_negations(*existential_quant.formula);
                    formula = existential_quant;
                } else if (auto *existential_quant = std::get_if<ExistentialQuantification>(&*node.operand)) {
                    auto universal_quant = UniversalQuantification(
                        existential_quant->var_symbol,
                        std::make_shared<Formula>(Negation(existential_quant->formula))
                    );
                    push_negations(*universal_quant.formula);
                    formula = universal_quant;
                } else {
                    push_negations(*node.operand);
                }
            },
            [](const Conjuction &node) {
                push_negations(*node.left);
                push_negations(*node.right);
            },
            [](const Disjunction &node) {
                push_negations(*node.left);
                push_negations(*node.right);
            },
            [](const Implication &node) {
                throw std::invalid_argument("Formula must not contain implication operators");
            },
            [](const Equivalence &node) {
                throw std::invalid_argument("Formula must not contain equivalence operators");
            },
            [](const UniversalQuantification &node) {
                push_negations(*node.formula);
            },
            [](const ExistentialQuantification &node) {
                push_negations(*node.formula);
            }
        }, formula
    );
}

void convert_to_negation_normal_form(Formula &formula)
{
    eliminate_implications_and_equivalences(formula);
    push_negations(formula);
}
