#ifndef COMPY5_LLVM_CODE_H
#define COMPY5_LLVM_CODE_H

#include <string>
#include <iostream>
<<<<<<< HEAD
using namespace std ;

//Global variables

int NUM=1;


//Funcs Decl

string freshVar();

=======
#include "parser.h"

using namespace std;

//Global Vars
int REGS_IDX=1;

//Funcs decl

string freshVar();
void BINOP_proc(Node* ret, Node* arg1, Node* op, Node* arg2);
>>>>>>> a4e505bc01d1fef02e204a2b1505538add0ff875




#endif //COMPY5_LLVM_CODE_H
