#ifndef THEOREM_PROVER_HPP
#define THEOREM_PROVER_HPP

#include "fol_driver.hpp"

#include <string>

class TheoremProver
{
public:
    bool is_theorem(const std::string &fol_formula) const;

private:
    mutable FOLDriver m_driver;
};

#endif // THEOREM_PROVER_HPP
