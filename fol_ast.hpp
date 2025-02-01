#ifndef FOL_AST_HPP
#define FOL_AST_HPP

#include "fraction.hpp"

#include <memory>
#include <variant>

struct Term;

struct RationalNumber
{
    Fraction value;
};

struct Variable
{
    char symbol;
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
    char var_symbol;
    std::shared_ptr<Formula> formula;
};

struct ExistentialQuantification
{
    char var_symbol;
    std::shared_ptr<Formula> formula;
};

struct Formula : public std::variant<AtomWrapper, LogicConstant, Negation, Conjuction, Disjunction, Implication, Equivalence, UniversalQuantification, ExistentialQuantification>
{
    using variant<AtomWrapper, LogicConstant, Negation, Conjuction, Disjunction, Implication, Equivalence, UniversalQuantification, ExistentialQuantification>::variant;
};

// For overloaded lambdas...
template <typename... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::string formula_to_string(const Formula &formula);

// Removes occurrences of <=, >= and != in the formula by transforming
// them into disjunctions of <, > and =.
void process_inequalities(Formula &formula);

void eliminate_constants(Formula &formula);

#endif // FOL_AST_HPP
