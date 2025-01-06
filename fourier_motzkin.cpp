#include "fourier_motzkin.hpp"

#include <stdexcept>
#include <algorithm>

Constraint::Constraint(const std::vector<Fraction> &lhs, Relation relation, const Fraction &rhs)
    : m_lhs(lhs)
    , m_relation(relation)
    , m_rhs(rhs)
{

}

ConstraintConjuction::ConstraintConjuction(const std::vector<Constraint> &constraints)
{
    for (std::size_t i = 1; i < constraints.size(); i++) {
        if (constraints[i - 1].m_lhs.size() != constraints[i].m_lhs.size()) {
            throw std::invalid_argument("All constraints in the conjuction must have the same number of variables - pad with 0 coefficients if needed");
        }
    }

    m_constraints = constraints;
}

bool ConstraintConjuction::is_satisfiable() const
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
        if (constraint.m_relation == Constraint::Relation::EQ && constraint.m_rhs != Fraction{0}) {
            return false;
        }
        if (constraint.m_relation == Constraint::Relation::LT && constraint.m_rhs <= Fraction{0}) {
            return false;
        }
        if (constraint.m_relation == Constraint::Relation::GT && constraint.m_rhs >= Fraction{0}) {
            return false;
        }
    }

    return true;
}

bool ConstraintConjuction::eliminate_variable_by_equality(std::vector<Constraint> &conjuction, std::size_t var_index) const
{
    for (std::size_t i = 0; i < conjuction.size(); i++) {
        if (conjuction[i].m_relation == Constraint::Relation::EQ) {
            const auto coef = conjuction[i].m_lhs[var_index];
            if (coef == Fraction{0}) {
                continue;
            }

            for (std::size_t j = 0; j < conjuction.size(); j++) {
                if (j == i || conjuction[j].m_lhs[var_index] == Fraction{0}) {
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

void ConstraintConjuction::eliminate_variable_by_inequality(std::vector<Constraint> &conjuction, std::size_t var_index) const
{
    std::vector<std::size_t> lt_inequalities, gt_inequalities;
    for (std::size_t i = 0; i < conjuction.size(); i++) {
        const auto coef = conjuction[i].m_lhs[var_index];
        if (coef == Fraction{0}) {
            continue;
        }

        if (conjuction[i].m_relation == Constraint::Relation::LT) {
            if (coef > Fraction{0}) {
                lt_inequalities.push_back(i);
            } else {
                gt_inequalities.push_back(i);
            }
        } else if (conjuction[i].m_relation == Constraint::Relation::GT) {
            if (coef > Fraction{0}) {
                gt_inequalities.push_back(i);
            } else {
                lt_inequalities.push_back(i);
            }
        }
    }

    for (const auto lt_idx : lt_inequalities) {
        for (const auto gt_idx : gt_inequalities) {
            std::vector<Fraction> new_ineq_lhs(conjuction[gt_idx].m_lhs.size());
            Fraction new_ineq_rhs;

            for (std::size_t k = 0; k < new_ineq_lhs.size(); k++) {
                new_ineq_lhs[k] = conjuction[lt_idx].m_lhs[k] / conjuction[lt_idx].m_lhs[var_index] - conjuction[gt_idx].m_lhs[k] / conjuction[gt_idx].m_lhs[var_index];
            }
            new_ineq_rhs = conjuction[lt_idx].m_rhs / conjuction[lt_idx].m_lhs[var_index] - conjuction[gt_idx].m_rhs / conjuction[gt_idx].m_lhs[var_index];

            conjuction.push_back(Constraint{
               new_ineq_lhs, Constraint::Relation::LT, new_ineq_rhs
            });
        }
    }

    std::vector<std::size_t> to_remove;
    std::merge(lt_inequalities.cbegin(), lt_inequalities.cend(), gt_inequalities.cbegin(), gt_inequalities.cend(), std::back_inserter(to_remove));
    for (std::size_t i = 0, rem = 0; i < conjuction.size(); i++) {
        conjuction[i - rem] = std::move(conjuction[i]);
        if (rem < to_remove.size() && i == to_remove[rem]) {
            rem++;
        }
    }
    conjuction.erase(conjuction.cbegin() + conjuction.size() - to_remove.size(), conjuction.cend());
}
