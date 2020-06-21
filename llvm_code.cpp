#include "llvm_code.h"
//Global Vars
int REG_IDX=1;

string crush_code() {
    string CC = CB.genLabel();
}

string freshVar(){
    return "%reg"+to_string(REG_IDX++);
}

string bstoi(string bs) {
    if(!bs.empty())
        bs.pop_back();
    return bs;
}

void add(string reg_a, string reg_b, Node* ret){
    CB.emit(ret->reg + " = add i32 " + reg_a + ", " + reg_b);
}

void sub(string reg_a, string reg_b, Node* ret){
    CB.emit(ret->reg + " = sub i32 " + reg_a + ", " + reg_b);
}


void mul(string reg_a, string reg_b, Node* ret){
    CB.emit(ret->reg + " = mul i32 " + reg_a + ", " + reg_b);
}

void div(string reg_a, string reg_b, Node* ret){
/*
 * the llvm code should look like:
 * %t1 = icmp eq i32 %1, 0
 * br i1 %t1, label _{CC}, label _{OK}
 * OK:   %t2 = sdiv %0, %1
 */
    string t1 = freshVar();
    string CC = crush_code();
    CB.emit(t1 + " = icmp eq i32 " + reg_b + " 0");
    int line2 = CB.emit("br i1 " + t1 + ", label @, label @");
    CB.bpatch(CB.makelist({line2, SECOND}), CC);
    string OK = CB.genLabel();
    CB.bpatch(CB.makelist({line2, FIRST}), OK);
    CB.emit(ret->reg + " = sdiv i32 " + reg_a + ", " + reg_b);
}

void BINOP_proc(Node* ret, Node* arg1, Node* op, Node* arg2) {
    string a = (arg1->type == "int") ? arg1->value : bstoi(arg1->value);
    string b = (arg2->type == "int") ? arg2->value : bstoi(arg2->value);
    CB.emit(arg1->reg + " = i32 " + a );
    CB.emit(arg2->reg + " = i32 " + b );
    if(op->name == "+") {
        add(arg1->reg, arg2->reg, ret);
    } else if(op->name == "-") {
        sub(arg1->reg, arg2->reg, ret);
    } else if(op->name == "*") {
        mul(arg1->reg, arg2->reg, ret);
    } else {
        div(arg1->reg, arg2->reg, ret);
    }
}


