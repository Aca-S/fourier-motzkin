#include "fourier_motzkin.hpp"

int main(int argc, char *argv[])
{
    Constraint c1 {
        {Fraction {2}, Fraction {3}, Fraction {-1}}, Constraint::Relation::GT, Fraction {0}
    };

    return 0;
}
