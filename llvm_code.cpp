#include "llvm_code.h"
//Global Vars
int REG_IDX=0;
int STR_IDX=0;
int ARG_IDX=0;

string crush_code() {
    return CB.genLabel();
}

string freshVar(){
    return "%reg"+to_string(REG_IDX++);
}

string freshArg(){
    return "%arg"+to_string(ARG_IDX++);
}

string freshStr(){
    return "@.str."+to_string(STR_IDX++);
}

string emitString(string &s){
    string label = freshStr();
    //s.pop_back();
    s[s.size()-1]='\\';
    s+="00\"";
    CB.emitGlobal(label + " = constant [" + to_string(s.size()-4) + " x i8] c" + s );
    return label;
}

void initRegIdx(){
    REG_IDX=0;
}

string bstoi(string bs) {
    if(!bs.empty())
        bs.pop_back();
    return bs;
}

void arithmitic_ops(string op , string regRes , string reg_a , string reg_b , bool to_trunc){
    if(to_trunc){
        string tmp = freshVar();
        CB.emit(tmp + " = " + op + " i32 " + reg_a + ", " + reg_b);
        CB.emit(regRes + " = and i32 " + tmp + ", 255");

    }
    else
        CB.emit(regRes + " = " + op + " i32 " + reg_a + ", " + reg_b);

}

void add(string reg_a, string reg_b, Node* ret , bool to_trunc){
    arithmitic_ops("add" , ret->reg ,reg_a , reg_b , to_trunc );
}

void sub(string reg_a, string reg_b, Node* ret , bool to_trunc){
    arithmitic_ops("sub" , ret->reg ,reg_a , reg_b , to_trunc );
}


void mul(string reg_a, string reg_b, Node* ret , bool to_trunc){
    arithmitic_ops("mul" , ret->reg ,reg_a , reg_b , to_trunc );
}

void div(string reg_a, string reg_b, Node* ret , bool to_trunc){
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
    string divByZ = freshVar();
    CB.emit("call void @print(i8* getelementptr ([23 x i8], [23 x i8]* @.stzero, i32 0, i32 0))");
    CB.emit(divByZ + " = add i32 0 , -1");
    CB.emit("call void (i32) @exit(i32 " + divByZ + ")");
    CB.bpatch(CB.makelist({line2, FIRST}), CC);

    int line3 = CB.emit("br label @");
    string OK = CB.genLabel();
    CB.bpatch(CB.makelist({line3,FIRST}), OK);

    CB.bpatch(CB.makelist({line2, SECOND}), OK);
    arithmitic_ops("sdiv" , ret->reg ,reg_a , reg_b , to_trunc );
}

void BINOP_proc(Node* ret, Node* arg1, Node* op, Node* arg2) {
    bool to_trunc=false;
    if(arg1->type=="BYTE" && arg2->type=="BYTE") to_trunc=true;
    if(op->name == "+") {
        add(arg1->reg, arg2->reg, ret , to_trunc);
    } else if(op->name == "-") {
        sub(arg1->reg, arg2->reg, ret , to_trunc);
    } else if(op->name == "*") {
        mul(arg1->reg, arg2->reg, ret , to_trunc);
    } else {
        div(arg1->reg, arg2->reg, ret , to_trunc);
    }
}


