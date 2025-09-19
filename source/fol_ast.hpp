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
    Fraction coef;
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

struct Term : public std::variant<RationalNumber, Variable, Addition, Subtraction>
{
    using variant::variant;
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
    using variant::variant;
};

struct Formula;

struct AtomWrapper
{
    std::shared_ptr<Atom> atom;
};

struct True
{

};

struct False
{

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

struct Formula : public std::variant<AtomWrapper, True, False, Negation, Conjuction, Disjunction, Implication, Equivalence, UniversalQuantification, ExistentialQuantification>
{
    using variant::variant;
};

// For overloaded lambdas...
template <typename... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename T, typename... Args>
std::shared_ptr<Term> t_ptr(Args&&... args)
{
    return std::make_shared<Term>(T(args...));
}

template <typename T, typename... Args>
std::shared_ptr<Atom> a_ptr(Args&&... args)
{
    return std::make_shared<Atom>(T(args...));
}

template <typename T, typename... Args>
std::shared_ptr<Formula> f_ptr(Args&&... args)
{
    return std::make_shared<Formula>(T(args...));
}

#endif // FOL_AST_HPP
