%option noyywrap
%option nounput
%option noinput

%{
#include "fol_driver.hpp"
#include "fol_parser.tab.hpp"
%}

%%

[|?*+()] {
    return *yytext;
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
