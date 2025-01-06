#ifndef FOURIER_MOTZKIN_HPP
#define FOURIER_MOTZKIN_HPP

#include <vector>
#include <cstddef>

#include "fraction.hpp"

class Constraint
{
    friend class ConstraintConjuction;

public:
    enum class Relation { EQ, LT, GT };

    Constraint(const std::vector<Fraction> &lhs, Relation relation, const Fraction &rhs);

private:
    std::vector<Fraction> m_lhs;
    Relation m_relation;
    Fraction m_rhs;
};

class ConstraintConjuction
{
public:
    ConstraintConjuction(const std::vector<Constraint> &constraints);

    bool is_satisfiable() const;

private:
    std::vector<Constraint> m_constraints;

    bool eliminate_variable_by_equality(std::vector<Constraint> &conjuction, std::size_t var_index) const;
    void eliminate_variable_by_inequality(std::vector<Constraint> &conjuction, std::size_t var_index) const;
};

#endif // FOURIER_MOTZKIN_HPP
