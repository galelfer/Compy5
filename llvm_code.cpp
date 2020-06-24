#include "llvm_code.h"
//Global Vars
int REG_IDX=0;

string crush_code() {
    return CB.genLabel();
}

string freshVar(){
    return "%reg"+to_string(REG_IDX++);
}

void initRegIdx(){
    REG_IDX=0;
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
    CB.emit(t1 + " = icmp eq i32 " + reg_b + " , 0");
    int line2 = CB.emit("br i1 " + t1 + ", label @, label @");
    string CC = crush_code();
    CB.emit("call void @print(i8* getelementptr ([23 x i8], [23 x i8]* @.stzero, i32 0, i32 0))");
    CB.emit("%divByZ = add i32 0 , -1");
    CB.emit("call void (i32) @exit(i32 %divByZ)");
    CB.bpatch(CB.makelist({line2, FIRST}), CC);
    string OK = CB.genLabel();
    CB.bpatch(CB.makelist({line2, SECOND}), OK);
    CB.emit(ret->reg + " = sdiv i32 " + reg_a + ", " + reg_b);
}

void BINOP_proc(Node* ret, Node* arg1, Node* op, Node* arg2) {
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


