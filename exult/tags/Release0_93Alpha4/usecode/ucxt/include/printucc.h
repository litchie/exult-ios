
#ifndef PRINTUCC_H
#define PRINTUCC_H

#include "ucc.h"
#include <map>
#include <vector>

void output_ucfunc(const unsigned int uc_funcid,
                   const map<unsigned int, string, less<unsigned int> > &uc_data,
                   const unsigned int &uc_argc, const unsigned int &uc_localc,
                   const vector<unsigned int> &uc_externs, const vector<UCc> &uc_codes,
                   const char** func_table);
void indent(ostream &o);
string extern_tostr(const unsigned int uc_extern);

#endif

