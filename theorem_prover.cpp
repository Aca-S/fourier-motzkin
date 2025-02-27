#include "theorem_prover.hpp"
#include "fourier_motzkin.hpp"

#include <stdexcept>
#include <map>

std::map<std::string, std::size_t> collect_variables(std::shared_ptr<Formula> formula);

bool TheoremProver::is_theorem(const std::string &fol_formula) const
{
    auto formula = m_driver.parse(fol_formula);
    if (!formula) {
        throw std::invalid_argument("Parsing failed: \"" + fol_formula + "\" is not a valid first order logic formula");
    }

    formula = close(pnf(formula));
    std::cout << formula_to_string(*formula) << std::endl;

    const auto var_map = collect_variables(formula);
    for (const auto &[k, v] : var_map) {
        std::cout << k << ": " << v << std::endl;
    }

    return false;
}

std::map<std::string, std::size_t> collect_variables(std::shared_ptr<Formula> formula)
{
    std::map<std::string, std::size_t> var_map;

    std::size_t counter = 0;
    auto current = formula;
    while (true) {
        if (const auto *uni = std::get_if<UniversalQuantification>(current.get())) {
            var_map[uni->var_symbol] = counter++;
            current = uni->formula;
        } else if (const auto *exi = std::get_if<ExistentialQuantification>(current.get())) {
            var_map[exi->var_symbol] = counter++;
            current = exi->formula;
        } else {
            break;
        }
    }

    return var_map;
}
