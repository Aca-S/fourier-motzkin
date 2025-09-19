#include "fol_driver.hpp"

std::shared_ptr<Formula> FOLDriver::parse(const std::string &formula)
{
    string_scan_init(formula);
    yy::parser parser(*this);
    int res = parser();
    string_scan_deinit();

    return res == 0 ? std::move(m_ast) : nullptr;
}
