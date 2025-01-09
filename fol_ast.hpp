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
    std::unique_ptr<Term> left, right;
};

struct Subtraction
{
    std::unique_ptr<Term> left, right;
};

struct Multiplication
{
    std::unique_ptr<Term> left, right;
};

struct Division
{
    std::unique_ptr<Term> left, right;
};

struct Term : public std::variant<RationalNumber, Variable, Addition, Subtraction, Multiplication, Division>
{
    using variant<RationalNumber, Variable, Addition, Subtraction, Multiplication, Division>::variant;
};

struct LogicalConstant
{
    bool value;
};

struct EqualTo
{
    std::unique_ptr<Term> left, right;
};

struct LessThan
{
    std::unique_ptr<Term> left, right;
};

struct LessOrEqualTo
{
    std::unique_ptr<Term> left, right;
};

struct GreaterThan
{
    std::unique_ptr<Term> left, right;
};

struct GreaterOrEqualTo
{
    std::unique_ptr<Term> left, right;
};

struct AtomicFormula : public std::variant<LogicalConstant, EqualTo, LessThan, LessOrEqualTo, GreaterThan, GreaterOrEqualTo>
{
    using variant<LogicalConstant, EqualTo, LessThan, LessOrEqualTo, GreaterThan, GreaterOrEqualTo>::variant;
};

struct Formula;

struct Negation
{
    std::unique_ptr<Formula> operand;
};

struct Conjuction
{
    std::unique_ptr<Formula> left, right;
};

struct Disjunction
{
    std::unique_ptr<Formula> left, right;
};

struct Implication
{
    std::unique_ptr<Formula> left, right;
};

struct Equivalence
{
    std::unique_ptr<Formula> left, right;
};

struct UniversalQuantification
{
    std::unique_ptr<Variable> variable;
    std::unique_ptr<Formula> formula;
};

struct ExistentialQuantification
{
    std::unique_ptr<Variable> variable;
    std::unique_ptr<Formula> formula;
};

struct Formula : public std::variant<AtomicFormula, Conjuction, Disjunction, Implication, Equivalence, UniversalQuantification, ExistentialQuantification>
{
    using variant<AtomicFormula, Conjuction, Disjunction, Implication, Equivalence, UniversalQuantification, ExistentialQuantification>::variant;
};

// For overloaded lambdas...
template <typename... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

#endif // FOL_AST_HPP
