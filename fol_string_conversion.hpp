#ifndef FOL_STRING_CONVERSION_HPP
#define FOL_STRING_CONVERSION_HPP

#include "fol_ast.hpp"

#include <string>
#include <memory>

std::string formula_to_string(std::shared_ptr<Formula> formula);

std::shared_ptr<Formula> string_to_formula(const std::string &formula);

#endif // FOL_STRING_CONVERSION_HPP
