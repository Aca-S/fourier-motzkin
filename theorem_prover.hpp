#ifndef THEOREM_PROVER_HPP
#define THEOREM_PROVER_HPP

#include "fol_driver.hpp"

#include <string>
#include <map>

class TheoremProver
{
public:
    bool is_theorem(const std::string &fol_formula) const;

private:
    mutable FOLDriver m_driver;
};

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

#endif // THEOREM_PROVER_HPP
