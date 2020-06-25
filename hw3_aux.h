#ifndef COMPY3_HW3_AUX_H
#define COMPY3_HW3_AUX_H

#include "llvm_code.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;
typedef long long ll;

class arg {
public:
    string name;
    string type;
    ll offset;

    arg(const string name, const string type, ll offset) : name(name), type(type), offset(offset) {}
};

typedef vector <arg> table;

class symbol {

public:
    vector <table> t_stack;
    vector <ll> o_stack;
    string llvm_stack_reg;
    string input_llvm_stack_reg;

    symbol() {
        table global_scope;
        t_stack.emplace_back(global_scope);
        o_stack.emplace_back(0);
    }

    Node* makeNodeFromID(const string& id, int lineno);

    const arg* get_var(const string &unique_name, bool isFunc);

    const arg* get_var_type(const string &name, const string &type);

    string larger(const string &type1, const string &type2);

    void assign(const string &name, const string &type, int lineno);

    void check_types(const string &type1, const string &type2, int lineno);

    void add_var(const string &name, const string &type, bool isFunc, int lineno);

    void add_func(const string& name, const string& type , int lineno);

    void add_scope();

    bool remove_scope();

    void decl_func(const string& name, const string& type, const string& ret_val , string& arg1 , int lineno);

    void finishDeclFunc(string &type);

    void PrintScope(table scope);

    void init_global_table();

    void does_main_exist();

    string funcType(const string &func_name, const string &args_types, int lineno);

    void insideLoop(int loopsCnt , string kind , int lineno);

    void onlyOneMain(int lineno , const string &name);

    void check_valid_b(const string &name , int lineno);

    void assign_check_types(const string &type1, const string &type2, int lineno);

    void assign_value(const string &name, const string &type, int lineno , const string &reg);

    void forceIntoReg(Node* node);

    void init_llvm_stack();

    void init_var_in_llvmStack(const string &name, const string &type, int lineno);

    void boolean_evaluation(Node* exp);

    void init_truelist(Node* node);

    void init_falselist(Node* node);

    void swap_truelist_falselist(Node* resExp , Node* exp);

    string switch_relop(string rel);

    string to_i32(vector<string> types);

    void and_backpatch( Node* res , Node* first , Node* second , string marker_label);

    void or_backpatch( Node* res , Node* first , Node* second , string marker_label);

    void relop_evaluation(Node* res , string op , string arg1 , string arg2);

    void bool_evaluation_for_call(Node* node);

    string ret_value(string ret);

    void if_backpatching(Node* res , Node* exp , Node* statement , string marker_label);

    void if_else_backpatch(Node* res , Node* exp  , Node* statement1 , Node* N , Node* statement2 , Node* M2 ,string M1_label);

    void exit_loop(Node* res);

    void skip_loop(Node* res);

    void while_backpatch(Node* res , Node* exp , Node* statement , Node* marker1 , Node* marker2);

    void while_else_backpatch(Node* res , Node* exp , Node* statement1 , Node* statement2 , Node* marker1 , Node* marker2 , Node* marker3 , Node* skip_marker);
};


#endif //COMPY3_HW3_AUX_H