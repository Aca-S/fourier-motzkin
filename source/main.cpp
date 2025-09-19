#include "theorem_prover.hpp"

#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    TheoremProver prover(std::cout);

    std::string formula;
    std::getline(std::cin, formula);

    prover.is_theorem(formula);

    return 0;
}
