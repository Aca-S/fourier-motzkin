#ifndef FOL_AST_HPP
#define FOL_AST_HPP

#include "fraction.hpp"

#include <string>
#include <memory>
#include <variant>

struct Term;

struct RationalNumber
{
    Fraction value;
};

struct Variable
{
    std::string symbol;
};

struct Addition
{
    std::shared_ptr<Term> left, right;
};

struct Subtraction
{
    std::shared_ptr<Term> left, right;
};

struct Multiplication
{
    std::shared_ptr<Term> left, right;
};

struct Division
{
    std::shared_ptr<Term> left, right;
};

struct Term : public std::variant<RationalNumber, Variable, Addition, Subtraction, Multiplication, Division>
{
    using variant<RationalNumber, Variable, Addition, Subtraction, Multiplication, Division>::variant;
};

struct EqualTo
{
    std::shared_ptr<Term> left, right;
};

struct LessThan
{
    std::shared_ptr<Term> left, right;
};

struct LessOrEqualTo
{
    std::shared_ptr<Term> left, right;
};

struct GreaterThan
{
    std::shared_ptr<Term> left, right;
};

struct GreaterOrEqualTo
{
    std::shared_ptr<Term> left, right;
};

struct NotEqualTo
{
    std::shared_ptr<Term> left, right;
};

struct Atom : public std::variant<EqualTo, LessThan, LessOrEqualTo, GreaterThan, GreaterOrEqualTo, NotEqualTo>
{
    using variant<EqualTo, LessThan, LessOrEqualTo, GreaterThan, GreaterOrEqualTo, NotEqualTo>::variant;
};

struct Formula;

struct AtomWrapper
{
    std::shared_ptr<Atom> atom;
};

struct LogicConstant
{
    bool value;
};

struct Negation
{
    std::shared_ptr<Formula> operand;
};

struct Conjuction
{
    std::shared_ptr<Formula> left, right;
};

struct Disjunction
{
    std::shared_ptr<Formula> left, right;
};

struct Implication
{
    std::shared_ptr<Formula> left, right;
};

struct Equivalence
{
    std::shared_ptr<Formula> left, right;
};

struct UniversalQuantification
{
    std::string var_symbol;
    std::shared_ptr<Formula> formula;
};

struct ExistentialQuantification
{
    std::string var_symbol;
    std::shared_ptr<Formula> formula;
};

struct Formula : public std::variant<AtomWrapper, LogicConstant, Negation, Conjuction, Disjunction, Implication, Equivalence, UniversalQuantification, ExistentialQuantification>
{
    using variant<AtomWrapper, LogicConstant, Negation, Conjuction, Disjunction, Implication, Equivalence, UniversalQuantification, ExistentialQuantification>::variant;
};

std::string formula_to_string(const Formula &formula);

std::shared_ptr<Formula> simplify(std::shared_ptr<Formula> formula);
std::shared_ptr<Formula> nnf(std::shared_ptr<Formula> formula);
std::shared_ptr<Formula> pnf(std::shared_ptr<Formula> formula);

std::shared_ptr<Formula> substitute(std::shared_ptr<Formula> formula, const std::string &var, std::shared_ptr<Term> s_term);

#endif // FOL_AST_HPP
