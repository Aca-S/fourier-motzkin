#include "theorem_prover.hpp"
#include "fourier_motzkin.hpp"

#include <stdexcept>

bool TheoremProver::is_theorem(const std::string &fol_formula) const
{
    auto formula = m_driver.parse(fol_formula);
    if (!formula) {
        throw std::invalid_argument("Parsing failed: \"" + fol_formula + "\" is not a valid first order logic formula");
    }

    formula = close(pnf(formula));

    return false;
}
