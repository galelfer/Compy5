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
void BINOP_proc(Node* ret, Node* arg1, Node* op, Node* arg2);





#endif //COMPY5_LLVM_CODE_H
