#ifndef FOURIER_MOTZKIN_HPP
#define FOURIER_MOTZKIN_HPP

#include <vector>

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
public:
    enum class Relation { EQ, LT, GT };

    Constraint(const std::vector<Fraction> &lhs, Relation relation, const Fraction &rhs);

private:
    std::vector<Fraction> m_lhs;
    Relation m_relation;
    Fraction m_rhs;
};

#endif // FOURIER_MOTZKIN_HPP
