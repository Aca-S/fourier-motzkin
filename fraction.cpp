#include "fraction.hpp"

#include <stdexcept>
#include <numeric>

Fraction::Fraction(int numerator, int denominator)
{
    if (denominator == 0) {
        throw std::invalid_argument("Fraction denominator must be non-zero");
    }

    if (denominator < 0) {
        numerator = -numerator;
        denominator = -denominator;
    }

    const auto gcd = std::gcd(numerator, denominator);
    m_numerator = numerator / gcd;
    m_denominator = denominator / gcd;
}

int Fraction::get_numerator() const
{
    return m_numerator;
}

int Fraction::get_denominator() const
{
    return m_denominator;
}

Fraction Fraction::operator+(const Fraction &other) const
{
    const auto res_numerator = m_numerator * other.m_denominator + m_denominator * other.m_numerator;
    const auto res_denominator = m_denominator * other.m_denominator;
    return Fraction(res_numerator, res_denominator);
}

Fraction Fraction::operator-(const Fraction &other) const
{
    const auto res_numerator = m_numerator * other.m_denominator - m_denominator * other.m_numerator;
    const auto res_denominator = m_denominator * other.m_denominator;
    return Fraction(res_numerator, res_denominator);
}

Fraction Fraction::operator*(const Fraction &other) const
{
    const auto res_numerator = m_numerator * other.m_numerator;
    const auto res_denominator = m_denominator * other.m_denominator;
    return Fraction(res_numerator, res_denominator);
}

Fraction Fraction::operator/(const Fraction &other) const
{
    const auto res_numerator = m_numerator * other.m_denominator;
    const auto res_denominator = m_denominator * other.m_numerator;
    return Fraction(res_numerator, res_denominator);
}

Fraction Fraction::operator-() const
{
    return Fraction(-m_numerator, m_denominator);
}
