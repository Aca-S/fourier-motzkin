#include "fourier_motzkin.hpp"
#include "fraction.hpp"
#include "ordered_field_fol_ast.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    // Constraint c1 {
    //     {Fraction {2}, Fraction {3}, Fraction {-1}}, Constraint::Relation::GT, Fraction {0}
    // };
    //
    // Constraint c2 {
    //     {Fraction {1}, Fraction {-1}, Fraction {0}}, Constraint::Relation::GT, Fraction {0}
    // };
    //
    // Constraint c3 {
    //     {Fraction {-3}, Fraction {0}, Fraction {1}}, Constraint::Relation::EQ, Fraction {0}
    // };
    //
    // Constraint c4 {
    //     {Fraction {0}, Fraction {1}, Fraction {0}}, Constraint::Relation::LT, Fraction {0}
    // };
    //
    // ConstraintConjuction cc {
    //     {c1, c2, c3, c4}
    // };

    // Constraint c1 {
    //     {Fraction{1}, Fraction{1}}, Constraint::Relation::EQ, Fraction{4}
    // };
    //
    // Constraint c2 {
    //     {Fraction{2}, Fraction{1}}, Constraint::Relation::EQ, Fraction{6}
    // };
    //
    // ConstraintConjuction cc {
    //     {c1, c2}
    // };

    Constraint<Fraction> c1 {
        {Fraction {1}, Fraction {1}}, Constraint<Fraction>::Relation::GT, Fraction {8}
    };

    Constraint<Fraction> c2 {
        {Fraction {1}, Fraction {1}}, Constraint<Fraction>::Relation::LT, Fraction {7}
    };

    ConstraintConjuction<Fraction> cc {
        {c1, c2}
    };

    // Constraint<double> c1 {
    //     {1, 1}, Constraint<double>::Relation::GT, 8
    // };
    //
    // Constraint<double> c2 {
    //     {1, 1}, Constraint<double>::Relation::LT, 7
    // };
    //
    // ConstraintConjuction<double> cc {
    //     {c1, c2}
    // };

    const auto is_sat = cc.is_satisfiable();
    std::cout << "Is satisfiable: " << (is_sat ? "true" : "false") << std::endl;

    return 0;
}
