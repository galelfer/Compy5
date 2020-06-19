#ifndef COMPY5_LLVM_CODE_H
#define COMPY5_LLVM_CODE_H

#include <string>
#include <iostream>
#include "parser.h"


using namespace std;

//Global Vars
int REGS_IDX=1;

//Funcs decl

string freshVar();
string add(string type1 , string var1 , string type2 , string var2);
string sub(string type1 , string var1 , string type2 , string var2);
string mul(string type1 , string var1 , string type2 , string var2);
string div(string type1 , string var1 , string type2 , string var2);




#endif //COMPY5_LLVM_CODE_H
