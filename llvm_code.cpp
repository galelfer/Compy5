#include "llvm_code.h"



string crush_code() {
    string CC = CB.genLabel();
}

string freshVar(){
    return "%reg"+to_string(REGS_IDX++);
}

string bstoi(string bs) {
    if(!bs.empty())
        bs.pop_back();
    return bs;
}

void add(string reg_a, string reg_b, Node* ret){
//    string ta = freshVar();
//    CB.emit(ta + " = load i32, i32* " + a.reg);
    CB.emit(ret->reg + " = add i32 " + reg_a + ", " + reg_b);
    //TODO: ret.reg = t1  //resolved line 62!!
}


void sub(string reg_a, string reg_b, Node* ret){
    CB.emit(ret->reg + " = sub i32 " + reg_a + ", " + reg_b);
}


void mul(string reg_a, string reg_b, Node* ret){
    CB.emit(ret->reg + " = mul i32 " + reg_a + ", " + reg_b);
}


void div(string a, string b, Node* ret){
/*
 * the llvm code should look like:
 * %t1 = icmp eq i32 %1, 0
 * br i1 %t1, label _{CC}, label _{OK}
 * OK:   %t2 = sdiv %0, %1
 */
    string t1 = freshVar();
    string CC = crush_code();
    CB.emit(t1 + " = icmp eq i32 " + b + " 0");
    int line2 = CB.emit("br i1 " + t1 + ", label @, label @");
    CB.bpatch(CB.makelist({line2, SECOND}), CC);
    string OK = CB.genLabel();
    CB.bpatch(CB.makelist({line2, FIRST}), OK);
    CB.emit(ret->reg + " = sdiv i32 " + a + ", " + b);
    //TODO: ret.reg = t2 //resolved in line 62!!
}

void BINOP_proc(Node* ret, Node* arg1, Node* op, Node* arg2) {
    string a = (arg1->type == "int") ? arg1->name : bstoi(arg1->name);
    string b = (arg2->type == "int") ? arg2->name : bstoi(arg2->name);
    //TODO: string reg_a = arg1.reg ### see line 66 ###
    //TODO: string reg_b = arg2.reg ### see line 67 ###
    arg1->reg = freshVar();
    arg2->reg = freshVar();
    ret->reg  = freshVar();
    CB.emit(arg1->reg + " = i32 " + a );
    CB.emit(arg2->reg + " = i32 " + b );
    if(op->name == "+") {
        add(arg1->reg, arg2->reg, ret);   // changed according to lines 66 + 67  (previously :- add(a, b, ret);)
    } else if(op->name == "-") {
        sub(arg1->reg, arg2->reg, ret);  // changed according to lines 66 + 67  (previously :- sub(a, b, ret);)
    } else if(op->name == "*") {
        mul(arg1->reg, arg2->reg, ret);  // changed according to lines 66 + 67  (previously :- mul(a, b, ret);)
    } else {
        div(arg1->reg, arg2->reg, ret); // changed according to lines 66 + 67  (previously :- div(a, b, ret);)
    }
}

