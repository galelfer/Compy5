#ifndef COMPY5_LLVM_CODE_H
#define COMPY5_LLVM_CODE_H

#include <string>
#include <iostream>
#include "parser.h"

using namespace std;



//Funcs decl

string freshVar();
string bstoi(string bs);
string emitString(string &s);
void initRegIdx();
void BINOP_proc(Node* ret, Node* arg1, Node* op, Node* arg2);





#endif //COMPY5_LLVM_CODE_H
