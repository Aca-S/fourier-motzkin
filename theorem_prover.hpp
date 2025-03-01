#ifndef THEOREM_PROVER_HPP
#define THEOREM_PROVER_HPP

#include "fol_driver.hpp"

#include <string>
#include <ostream>
#include <map>

class VariableMapping
{
public:
    void add_variable(const std::string &variable_symbol);
    void remove_variable(const std::string &variable_symbol);

    std::size_t get_variable_number(const std::string &variable_symbol) const;
    std::string get_variable_symbol(std::size_t variable_number) const;

    std::size_t size() const;

private:
    std::map<std::string, std::size_t> m_symbol_to_number;
    std::map<std::size_t, std::string> m_number_to_symbol;
};

namespace
{
std::ostream null_stream = std::ostream(nullptr);
}

class TheoremProver
{
public:
    TheoremProver(std::ostream &log = null_stream);

    bool is_theorem(const std::string &fol_formula) const;

private:
    mutable FOLDriver m_driver;
    std::ostream &m_log;

    std::shared_ptr<Formula> eliminate_quantifiers(std::shared_ptr<Formula> formula, VariableMapping &var_map) const;
    std::shared_ptr<Formula> eliminate_variable(std::shared_ptr<Formula> base_formula, const std::string &quantified_variable, VariableMapping &var_map, bool is_existential) const;
};

#endif // THEOREM_PROVER_HPP
