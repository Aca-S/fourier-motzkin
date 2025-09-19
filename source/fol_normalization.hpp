#ifndef FOL_NORMALIZATION_HPP
#define FOL_NORMALIZATION_HPP

#include "fol_ast.hpp"

#include <memory>

// Removes logical constants from the given formula or transforms it to a constant itself.
std::shared_ptr<Formula> simplify(std::shared_ptr<Formula> formula);

// Converts the given formula to its negation normal form.
std::shared_ptr<Formula> nnf(std::shared_ptr<Formula> formula);

// Converts the given formula to its prenex normal form.
std::shared_ptr<Formula> pnf(std::shared_ptr<Formula> formula);

// Converts the given formula to its disjunctive normal form.
std::shared_ptr<Formula> dnf(std::shared_ptr<Formula> formula);

// Converts the given formula to its closed form.
std::shared_ptr<Formula> close(std::shared_ptr<Formula> formula);

#endif // FOL_NORMALIZATION_HPP
