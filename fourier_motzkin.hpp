#ifndef FOURIER_MOTZKIN_HPP
#define FOURIER_MOTZKIN_HPP

#include <vector>
#include <cstddef>

class Fraction
{
public:
    Fraction(int numerator = 0, int denominator = 1);

    int get_numerator() const;
    int get_denominator() const;

    Fraction operator+(const Fraction &other) const;
    Fraction operator-(const Fraction &other) const;
    Fraction operator*(const Fraction &other) const;
    Fraction operator/(const Fraction &other) const;

    Fraction operator-() const;

    auto operator<=>(const Fraction &other) const
    {
        const auto cross_l = m_numerator * other.m_denominator;
        const auto cross_r = other.m_numerator * m_denominator;
        if (cross_l == cross_r) {
            return std::strong_ordering::equal;
        } else if (cross_l < cross_r) {
            return std::strong_ordering::less;
        } else {
            return std::strong_ordering::greater;
        }
    }

    bool operator==(const Fraction &other) const = default;

private:
    int m_numerator;
    int m_denominator;
};

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
