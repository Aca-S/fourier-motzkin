%option noyywrap
%option nounput
%option noinput

%{
#include "fol_driver.hpp"
#include "fol_parser.tab.hpp"

#include <cstdlib>
%}

%%

\-?(0|[1-9][0-9]*) {
    return yy::parser::make_INT_T(std::atoi(yytext));
}

[a-z] {
    return yy::parser::make_VAR_T(*yytext);
}

[+\-*/()] {
    return *yytext;
}

[=<>] {
    return *yytext;
}

"<=" {
    return yy::parser::make_LEQ_T();
}

">=" {
    return yy::parser::make_GEQ_T();
}

"!=" {
    return yy::parser::make_NEQ_T();
}

[~&\|!\?\.] {
    return *yytext;
}

"=>" {
    return yy::parser::make_IMPL_T();
}

"<=>" {
    return yy::parser::make_EQUI_T();
}

[ \t\r]+ {

}

%%

void FOLDriver::string_scan_init(const std::string &formula)
{
    yy_scan_string(formula.c_str());
}

void FOLDriver::string_scan_deinit()
{
    yy_delete_buffer(YY_CURRENT_BUFFER);
}
