#ifndef FOURIER_MOTZKIN_HPP
#define FOURIER_MOTZKIN_HPP

#include <vector>
#include <cstddef>
#include <stdexcept>
#include <algorithm>

template <typename T>
class ConstraintConjuction;

template <typename T>
class Constraint
{
    friend class ConstraintConjuction<T>;

public:
    enum class Relation { EQ, LT, GT };

    Constraint(const std::vector<T> &lhs, Relation relation, const T &rhs);

    const std::vector<T>& get_lhs() const;
    Relation get_relation() const;
    const T& get_rhs() const;

private:
    std::vector<T> m_lhs;
    Relation m_relation;
    T m_rhs;
};

template <typename T>
Constraint<T>::Constraint(const std::vector<T> &lhs, Relation relation, const T &rhs)
    : m_lhs(lhs)
    , m_relation(relation)
    , m_rhs(rhs)
{

}

template <typename T>
const std::vector<T>& Constraint<T>::get_lhs() const
{
    return m_lhs;
}

template <typename T>
Constraint<T>::Relation Constraint<T>::get_relation() const
{
    return m_relation;
}

template <typename T>
const T& Constraint<T>::get_rhs() const
{
    return m_rhs;
}

template <typename T>
class ConstraintConjuction
{
public:
    ConstraintConjuction(const std::vector<Constraint<T>> &constraints);

    bool is_satisfiable() const;
    const std::vector<Constraint<T>>& get_constraints() const;

    void eliminate_variable(std::size_t var_index);

private:
    std::vector<Constraint<T>> m_constraints;

    bool eliminate_variable_by_equality(std::vector<Constraint<T>> &conjuction, std::size_t var_index) const;
    void eliminate_variable_by_inequality(std::vector<Constraint<T>> &conjuction, std::size_t var_index) const;
};

template <typename T>
ConstraintConjuction<T>::ConstraintConjuction(const std::vector<Constraint<T>> &constraints)
{
    for (std::size_t i = 1; i < constraints.size(); i++) {
        if (constraints[i - 1].m_lhs.size() != constraints[i].m_lhs.size()) {
            throw std::invalid_argument("All constraints in the conjuction must have the same number of variables - pad with 0 coefficients if needed");
        }
    }

    m_constraints = constraints;
}

template <typename T>
bool ConstraintConjuction<T>::ConstraintConjuction::is_satisfiable() const
{
    if (m_constraints.size() == 0) {
        return true;
    }

    // We don't want to modify the original set of constraints, so we'll be working on their copy
    auto conjuction = m_constraints;
    const auto num_of_vars = conjuction[0].m_lhs.size();

    for (std::size_t i = 0; i < num_of_vars; i++) {
        if (!eliminate_variable_by_equality(conjuction, i)) {
            eliminate_variable_by_inequality(conjuction, i);
        }
    }

    for (const auto &constraint : conjuction) {
        if (constraint.m_relation == Constraint<T>::Relation::EQ && constraint.m_rhs != T{}) {
            return false;
        }
        if (constraint.m_relation == Constraint<T>::Relation::LT && constraint.m_rhs <= T{}) {
            return false;
        }
        if (constraint.m_relation == Constraint<T>::Relation::GT && constraint.m_rhs >= T{}) {
            return false;
        }
    }

    return true;
}

template <typename T>
bool ConstraintConjuction<T>::eliminate_variable_by_equality(std::vector<Constraint<T>> &conjuction, std::size_t var_index) const
{
    for (std::size_t i = 0; i < conjuction.size(); i++) {
        if (conjuction[i].m_relation == Constraint<T>::Relation::EQ) {
            const auto coef = conjuction[i].m_lhs[var_index];
            if (coef == T{}) {
                continue;
            }

            for (std::size_t j = 0; j < conjuction.size(); j++) {
                if (j == i || conjuction[j].m_lhs[var_index] == T{}) {
                    continue;
                }

                const auto mul = conjuction[j].m_lhs[var_index];
                for (std::size_t k = 0; k < conjuction[j].m_lhs.size(); k++) {
                    conjuction[j].m_lhs[k] = conjuction[j].m_lhs[k] - mul * conjuction[i].m_lhs[k] / coef;
                }
                conjuction[j].m_rhs = conjuction[j].m_rhs - mul * conjuction[i].m_rhs / coef;
            }

            conjuction.erase(conjuction.begin() + i);

            return true;
        }
    }

    return false;
}

template <typename T>
void ConstraintConjuction<T>::eliminate_variable_by_inequality(std::vector<Constraint<T>> &conjuction, std::size_t var_index) const
{
    std::vector<std::size_t> lt_inequalities, gt_inequalities;
    for (std::size_t i = 0; i < conjuction.size(); i++) {
        const auto coef = conjuction[i].m_lhs[var_index];
        if (coef == T{}) {
            continue;
        }

        if (conjuction[i].m_relation == Constraint<T>::Relation::LT) {
            if (coef > T{}) {
                lt_inequalities.push_back(i);
            } else {
                gt_inequalities.push_back(i);
            }
        } else if (conjuction[i].m_relation == Constraint<T>::Relation::GT) {
            if (coef > T{}) {
                gt_inequalities.push_back(i);
            } else {
                lt_inequalities.push_back(i);
            }
        }
    }

    for (const auto lt_idx : lt_inequalities) {
        for (const auto gt_idx : gt_inequalities) {
            std::vector<T> new_ineq_lhs(conjuction[gt_idx].m_lhs.size());
            T new_ineq_rhs;

            for (std::size_t k = 0; k < new_ineq_lhs.size(); k++) {
                new_ineq_lhs[k] = conjuction[lt_idx].m_lhs[k] / conjuction[lt_idx].m_lhs[var_index] - conjuction[gt_idx].m_lhs[k] / conjuction[gt_idx].m_lhs[var_index];
            }
            new_ineq_rhs = conjuction[lt_idx].m_rhs / conjuction[lt_idx].m_lhs[var_index] - conjuction[gt_idx].m_rhs / conjuction[gt_idx].m_lhs[var_index];

            conjuction.push_back(Constraint<T>{
               new_ineq_lhs, Constraint<T>::Relation::LT, new_ineq_rhs
            });
        }
    }

    std::vector<std::size_t> to_remove;
    std::merge(lt_inequalities.cbegin(), lt_inequalities.cend(), gt_inequalities.cbegin(), gt_inequalities.cend(), std::back_inserter(to_remove));
    for (auto it = to_remove.rbegin(); it != to_remove.rend(); it++) {
        conjuction.erase(conjuction.cbegin() + *it);
    }
}

template <typename T>
const std::vector<Constraint<T>>& ConstraintConjuction<T>::get_constraints() const
{
    return m_constraints;
}

template <typename T>
void ConstraintConjuction<T>::eliminate_variable(std::size_t var_index)
{
    if (!eliminate_variable_by_equality(m_constraints, var_index)) {
        eliminate_variable_by_inequality(m_constraints, var_index);
    }
}

#endif // FOURIER_MOTZKIN_HPP
