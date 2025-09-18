#include "fol_ast.hpp"

#include <ostream>
#include <variant>
#include <cassert>
#include <set>

static unsigned precedence(std::shared_ptr<Term> term)
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
                const auto num = node.value.get_numerator();
                const auto den = node.value.get_denominator();
                return std::to_string(num) + (den == 1 ? "" : "/" + std::to_string(den));
            },
            [](const Variable &node) {
                return node.symbol;
            },
            [&term](const Addition &node) {
                return wrap(node.left, term) + "+" + wrap(node.right, term);
            },
            [&term](const Subtraction &node) {
                return wrap(node.left, term) + "-" + wrap(node.right, term);
            },
            [&term](const Multiplication &node) {
                return wrap(std::make_shared<Term>(*node.coef), term) + "*" + wrap(std::make_shared<Term>(*node.var), term);
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
            [](const LogicConstant &node) {
                return node.value ? std::string("T") : std::string("F");
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

std::shared_ptr<Formula> nnf_h(std::shared_ptr<Formula> formula);

std::shared_ptr<Formula> nnf_not(std::shared_ptr<Formula> formula)
{
    return std::visit(
        overloaded{
            [&formula](const AtomWrapper &node) {
                return f_ptr<Negation>(formula);
            },
            [](const Negation &node) {
                return nnf_h(node.operand);
            },
            [](const Conjuction &node) {
                return f_ptr<Disjunction>(nnf_not(node.left), nnf_not(node.right));
            },
            [](const Disjunction &node) {
                return f_ptr<Conjuction>(nnf_not(node.left), nnf_not(node.right));
            },
            [](const Implication &node) {
                return f_ptr<Conjuction>(nnf_h(node.left), nnf_not(node.right));;
            },
            [](const Equivalence &node) {
                return f_ptr<Conjuction>(
                    f_ptr<Disjunction>(nnf_h(node.left), nnf_h(node.right)),
                    f_ptr<Disjunction>(nnf_not(node.left), nnf_not(node.right))
                );
            },
            [](const UniversalQuantification &node) {
                return f_ptr<ExistentialQuantification>(node.var_symbol, nnf_not(node.formula));
            },
            [](const ExistentialQuantification &node) {
                return f_ptr<ExistentialQuantification>(node.var_symbol, nnf_not(node.formula));
            },
            [&formula](const auto &node) {
                assert(!"Unreachable");
                return formula;
            }
        }, *formula
    );
}

std::shared_ptr<Formula> nnf_h(std::shared_ptr<Formula> formula)
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
                return f_ptr<Conjuction>(nnf_h(node.left), nnf_h(node.right));
            },
            [](const Disjunction &node) {
                return f_ptr<Disjunction>(nnf_h(node.left), nnf_h(node.right));
            },
            [](const Implication &node) {
                return f_ptr<Disjunction>(nnf_not(node.left), nnf_h(node.right));;
            },
            [](const Equivalence &node) {
                return f_ptr<Conjuction>(
                    f_ptr<Disjunction>(nnf_h(node.left), nnf_not(node.right)),
                    f_ptr<Disjunction>(nnf_not(node.left), nnf_h(node.right))
                );
            },
            [](const UniversalQuantification &node) {
                return f_ptr<UniversalQuantification>(node.var_symbol, nnf_h(node.formula));
            },
            [](const ExistentialQuantification &node) {
                return f_ptr<ExistentialQuantification>(node.var_symbol, nnf_h(node.formula));
            }
        }, *formula
    );
}

std::shared_ptr<Formula> nnf(std::shared_ptr<Formula> formula)
{
    return nnf_h(simplify(formula));
}

void collect_free_variables(std::shared_ptr<Term> term, std::set<std::string> &free_vars)
{
    std::visit(
        overloaded{
            [](const RationalNumber &node) {
            },
            [&free_vars](const Variable &node) {
                free_vars.insert(node.symbol);
            },
            [&free_vars](const Addition &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const Subtraction &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const Multiplication &node) {
                free_vars.insert(node.var->symbol);
            }
        }, *term
    );
}

void collect_free_variables(std::shared_ptr<Atom> atom, std::set<std::string> &free_vars)
{
    std::visit(
        overloaded{
            [&free_vars](const EqualTo &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const LessThan &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const LessOrEqualTo &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const GreaterThan &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const GreaterOrEqualTo &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const NotEqualTo &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            }
        }, *atom
    );
}

void collect_free_variables(std::shared_ptr<Formula> formula, std::set<std::string> &free_vars)
{
    std::visit(
        overloaded{
            [&free_vars](const AtomWrapper &node) {
                collect_free_variables(node.atom, free_vars);
            },
            [](const LogicConstant &node) {
            },
            [&free_vars](const Negation &node) {
                collect_free_variables(node.operand, free_vars);
            },
            [&free_vars](const Conjuction &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const Disjunction &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const Implication &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const Equivalence &node) {
                collect_free_variables(node.left, free_vars);
                collect_free_variables(node.right, free_vars);
            },
            [&free_vars](const UniversalQuantification &node) {
                bool is_free = free_vars.contains(node.var_symbol);
                collect_free_variables(node.formula, free_vars);
                if (!is_free) {
                    free_vars.erase(node.var_symbol);
                }
            },
            [&free_vars](const ExistentialQuantification &node) {
                bool is_free = free_vars.contains(node.var_symbol);
                collect_free_variables(node.formula, free_vars);
                if (!is_free) {
                    free_vars.erase(node.var_symbol);
                }
            }
        }, *formula
    );
}

void collect_quantified_variables(std::shared_ptr<Formula> formula, std::set<std::string> &quantified_vars)
{
    std::visit(
        overloaded{
            [](const AtomWrapper &node) {
            },
            [](const LogicConstant &node) {
            },
            [&quantified_vars](const Negation &node) {
                collect_quantified_variables(node.operand, quantified_vars);
            },
            [&quantified_vars](const Conjuction &node) {
                collect_quantified_variables(node.left, quantified_vars);
                collect_quantified_variables(node.right, quantified_vars);
            },
            [&quantified_vars](const Disjunction &node) {
                collect_quantified_variables(node.left, quantified_vars);
                collect_quantified_variables(node.right, quantified_vars);
            },
            [&quantified_vars](const Implication &node) {
                collect_quantified_variables(node.left, quantified_vars);
                collect_quantified_variables(node.right, quantified_vars);
            },
            [&quantified_vars](const Equivalence &node) {
                collect_quantified_variables(node.left, quantified_vars);
                collect_quantified_variables(node.right, quantified_vars);
            },
            [&quantified_vars](const UniversalQuantification &node) {
                quantified_vars.insert(node.var_symbol);
            },
            [&quantified_vars](const ExistentialQuantification &node) {
                quantified_vars.insert(node.var_symbol);
            }
        }, *formula
    );
}

std::string generate_unique_variable(const std::string &var, const std::set<std::string> &free_vars)
{
    std::string new_var = var;
    unsigned counter = 0;
    while (free_vars.contains(new_var)) {
        new_var = var + std::to_string(counter);
        counter++;
    }
    return new_var;
}

std::shared_ptr<Term> substitute(std::shared_ptr<Term> term, const std::string &var, const std::string &s_var)
{
    return std::visit(
        overloaded{
            [&term](const RationalNumber &node) {
                return term;
            },
            [&term, &var, &s_var](const Variable &node) {
                return node.symbol == var ? t_ptr<Variable>(s_var) : term;
            },
            [&var, &s_var](const Addition &node) {
                return t_ptr<Addition>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&var, &s_var](const Subtraction &node) {
                return t_ptr<Subtraction>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&term, &var, &s_var](const Multiplication &node) {
                return node.var->symbol == var ? t_ptr<Multiplication>(node.coef, std::make_shared<Variable>(s_var)) : term;
            }
        }, *term
    );
}

std::shared_ptr<Atom> substitute(std::shared_ptr<Atom> atom, const std::string &var, const std::string &s_var)
{
    return std::visit(
        overloaded{
            [&var, &s_var](const EqualTo &node) {
                return a_ptr<EqualTo>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&var, &s_var](const LessThan &node) {
                return a_ptr<LessThan>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&var, &s_var](const LessOrEqualTo &node) {
                return a_ptr<LessOrEqualTo>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&var, &s_var](const GreaterThan &node) {
                return a_ptr<GreaterThan>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&var, &s_var](const GreaterOrEqualTo &node) {
                return a_ptr<GreaterOrEqualTo>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&var, &s_var](const NotEqualTo &node) {
                return a_ptr<NotEqualTo>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            }
        }, *atom
    );
}

std::shared_ptr<Formula> substitute(std::shared_ptr<Formula> formula, const std::string &var, const std::string &s_var)
{
    return std::visit(
        overloaded{
            [&var, &s_var](const AtomWrapper &node) {
                return f_ptr<AtomWrapper>(substitute(node.atom, var, s_var));
            },
            [&formula](const LogicConstant &node) {
                return formula;
            },
            [&var, &s_var](const Negation &node) {
                return f_ptr<Negation>(substitute(node.operand, var, s_var));
            },
            [&var, &s_var](const Conjuction &node) {
                return f_ptr<Conjuction>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&var, &s_var](const Disjunction &node) {
                return f_ptr<Disjunction>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&var, &s_var](const Implication &node) {
                return f_ptr<Implication>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&var, &s_var](const Equivalence &node) {
                return f_ptr<Equivalence>(substitute(node.left, var, s_var), substitute(node.right, var, s_var));
            },
            [&formula, &var, &s_var](const UniversalQuantification &node) {
                if (node.var_symbol == var) {
                    return formula;
                }
                if (node.var_symbol == s_var) {
                    const auto new_var = generate_unique_variable(node.var_symbol, {s_var});
                    const auto new_subformula = substitute(node.formula, node.var_symbol, new_var);
                    return f_ptr<UniversalQuantification>(new_var, substitute(new_subformula, var, s_var));
                } else {
                    return f_ptr<UniversalQuantification>(node.var_symbol, substitute(node.formula, var, s_var));
                }
            },
            [&formula, &var, &s_var](const ExistentialQuantification &node) {
                if (node.var_symbol == var) {
                    return formula;
                }
                if (node.var_symbol == s_var) {
                    const auto new_var = generate_unique_variable(node.var_symbol, {s_var});
                    const auto new_subformula = substitute(node.formula, node.var_symbol, new_var);
                    return f_ptr<ExistentialQuantification>(new_var, substitute(new_subformula, var, s_var));
                } else {
                    return f_ptr<ExistentialQuantification>(node.var_symbol, substitute(node.formula, var, s_var));
                }
            }
        }, *formula
    );
}

std::shared_ptr<Formula> pull_quantifiers(std::shared_ptr<Formula> formula);

template<typename BinaryType, typename QuantifierType>
std::shared_ptr<Formula> pull_quantifiers(const BinaryType &node, const QuantifierType &quant, bool quantifier_on_left)
{
    std::set<std::string> free_vars;
    collect_free_variables(quantifier_on_left ? node.right : node.left, free_vars);
    if (free_vars.contains(quant.var_symbol)) {
        const auto new_var = generate_unique_variable(quant.var_symbol, free_vars);
        const auto new_subformula = substitute(quant.formula, quant.var_symbol, new_var);
        return f_ptr<QuantifierType>(new_var, pull_quantifiers(quantifier_on_left ? f_ptr<BinaryType>(new_subformula, node.right) : f_ptr<BinaryType>(node.left, new_subformula)));
    } else {
        return f_ptr<QuantifierType>(quant.var_symbol, pull_quantifiers(quantifier_on_left ? f_ptr<BinaryType>(quant.formula, node.right) : f_ptr<BinaryType>(node.left, quant.formula)));
    }
}

template<typename QuantifierType>
std::shared_ptr<Formula> pull_quantifiers(const QuantifierType &quant)
{
    std::set<std::string> quantified_vars;
    collect_quantified_variables(quant.formula, quantified_vars);
    if (quantified_vars.contains(quant.var_symbol)) {
        const auto new_var = generate_unique_variable(quant.var_symbol, quantified_vars);
        return f_ptr<QuantifierType>(new_var, pnf_h(substitute(quant.formula, quant.var_symbol, new_var)));
    } else {
        return f_ptr<QuantifierType>(quant.var_symbol, pnf_h(quant.formula));
    }
}

std::shared_ptr<Formula> pull_quantifiers(std::shared_ptr<Formula> formula)
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
            [](const Conjuction &node) {
                if (std::holds_alternative<UniversalQuantification>(*node.left) && std::holds_alternative<UniversalQuantification>(*node.right)) {
                    const auto l = std::get<UniversalQuantification>(*node.left);
                    const auto r = std::get<UniversalQuantification>(*node.right);
                    if (l.var_symbol == r.var_symbol) {
                        return f_ptr<UniversalQuantification>(l.var_symbol, pull_quantifiers(f_ptr<Conjuction>(l.formula, r.formula)));
                    }
                }
                if (const auto *uni = std::get_if<UniversalQuantification>(node.left.get())) {
                    return pull_quantifiers<Conjuction, UniversalQuantification>(node, *uni, true);
                } else if (const auto *exi = std::get_if<ExistentialQuantification>(node.left.get())) {
                    return pull_quantifiers<Conjuction, ExistentialQuantification>(node, *exi, true);
                } else if (const auto *uni = std::get_if<UniversalQuantification>(node.right.get())) {
                    return pull_quantifiers<Conjuction, UniversalQuantification>(node, *uni, false);
                } else if (const auto *exi = std::get_if<ExistentialQuantification>(node.right.get())) {
                    return pull_quantifiers<Conjuction, ExistentialQuantification>(node, *exi, false);
                } else {
                    return f_ptr<Conjuction>(pull_quantifiers(node.left), pull_quantifiers(node.right));
                }
            },
            [&formula](const Disjunction &node) {
                if (std::holds_alternative<ExistentialQuantification>(*node.left) && std::holds_alternative<ExistentialQuantification>(*node.right)) {
                    const auto l = std::get<ExistentialQuantification>(*node.left);
                    const auto r = std::get<ExistentialQuantification>(*node.right);
                    if (l.var_symbol == r.var_symbol) {
                        return f_ptr<UniversalQuantification>(l.var_symbol, pull_quantifiers(f_ptr<Disjunction>(l.formula, r.formula)));
                    }
                }
                if (const auto *uni = std::get_if<UniversalQuantification>(node.left.get())) {
                    return pull_quantifiers<Disjunction, UniversalQuantification>(node, *uni, true);
                } else if (const auto *exi = std::get_if<ExistentialQuantification>(node.left.get())) {
                    return pull_quantifiers<Disjunction, ExistentialQuantification>(node, *exi, true);
                } else if (const auto *uni = std::get_if<UniversalQuantification>(node.right.get())) {
                    return pull_quantifiers<Disjunction, UniversalQuantification>(node, *uni, false);
                } else if (const auto *exi = std::get_if<ExistentialQuantification>(node.right.get())) {
                    return pull_quantifiers<Disjunction, ExistentialQuantification>(node, *exi, false);
                } else {
                    return f_ptr<Disjunction>(pull_quantifiers(node.left), pull_quantifiers(node.right));
                }
            },
            [&formula](const UniversalQuantification &node) {
                return formula;
            },
            [&formula](const ExistentialQuantification &node) {
                return formula;
            },
            [&formula](const auto &node) {
                assert(!"Unreachable");
                return formula;
            }
        }, *formula
    );
}

std::shared_ptr<Formula> pnf_h(std::shared_ptr<Formula> formula)
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
            [](const Conjuction &node) {
                return pull_quantifiers(f_ptr<Conjuction>(pnf_h(node.left), pnf_h(node.right)));
            },
            [](const Disjunction &node) {
                return pull_quantifiers(f_ptr<Disjunction>(pnf_h(node.left), pnf_h(node.right)));
            },
            [](const UniversalQuantification &node) {
                return pull_quantifiers<UniversalQuantification>(node);
            },
            [](const ExistentialQuantification &node) {
                return pull_quantifiers<ExistentialQuantification>(node);
            },
            [&formula](const auto &node) {
                assert(!"Unreachable");
                return formula;
            }
        }, *formula
    );
}

std::shared_ptr<Formula> pnf(std::shared_ptr<Formula> formula)
{
    return pnf_h(nnf(formula));
}

std::shared_ptr<Formula> dnf_h(std::shared_ptr<Formula> formula)
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
                const auto left = dnf_h(node.left);
                const auto right = dnf_h(node.right);
                if (const auto *dis = std::get_if<Disjunction>(left.get())) {
                    return f_ptr<Disjunction>(
                        dnf_h(f_ptr<Conjuction>(dis->left, right)),
                        dnf_h(f_ptr<Conjuction>(dis->right, right))
                    );
                } else if (const auto *dis = std::get_if<Disjunction>(right.get())) {
                    return f_ptr<Disjunction>(
                        dnf_h(f_ptr<Conjuction>(left, dis->left)),
                        dnf_h(f_ptr<Conjuction>(left, dis->right))
                    );
                } else {
                    return formula;
                }
            },
            [](const Disjunction &node) {
                return f_ptr<Disjunction>(dnf_h(node.left), dnf_h(node.right));
            },
            [](const UniversalQuantification &node) {
                return f_ptr<UniversalQuantification>(node.var_symbol, dnf_h(node.formula));
            },
            [](const ExistentialQuantification &node) {
                return f_ptr<UniversalQuantification>(node.var_symbol, dnf_h(node.formula));
            },
            [&formula](const auto &node) {
                assert(!"Unreachable");
                return formula;
            }
        }, *formula
    );
}

std::shared_ptr<Formula> dnf(std::shared_ptr<Formula> formula)
{
    return dnf_h(pnf(formula));
}

std::shared_ptr<Formula> close(std::shared_ptr<Formula> formula)
{
    std::set<std::string> free_vars;
    collect_free_variables(formula, free_vars);
    auto closed_formula = formula;
    for (const auto &var : free_vars) {
        closed_formula = f_ptr<ExistentialQuantification>(var, closed_formula);
    }
    return closed_formula;
}
