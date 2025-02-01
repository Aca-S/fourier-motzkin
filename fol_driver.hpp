#ifndef FOL_DRIVER_HPP
#define FOL_DRIVER_HPP

#include "fol_ast.hpp"
#include "fol_parser.tab.hpp"

#include <memory>

class FOLDriver
{
    // Needed for accessing m_ast from the parser.
    friend class yy::parser;

public:
    std::shared_ptr<Formula> parse(const std::string &formula);

private:
    // Implemented in the lexer file - alternatively,
    // do extern declarations for the needed lexer functions.
    void string_scan_init(const std::string &formula);
    void string_scan_deinit();

    std::shared_ptr<Formula> m_ast;
};

// By default, yylex's signature is int yylex(void),
// so we have to redefine it.
#define YY_DECL yy::parser::symbol_type yylex(FOLDriver &driver)
// Declare yylex for use in the parser.
YY_DECL;

#endif // FOL_DRIVER_HPP
