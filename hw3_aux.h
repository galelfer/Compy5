#ifndef COMPY3_HW3_AUX_H
#define COMPY3_HW3_AUX_H


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

    symbol() {
        table global_scope;
        t_stack.emplace_back(global_scope);
    }

    const arg* get_var(const string &unique_name, int lineno , bool checkBeforeAddOp);

    const arg* get_var_type(const string &name, const string &type, int lineno , bool checkBeforeAddOp);

    string larger(const string &type1, const string &type2);

    void assign(const string &name, const string &type, int lineno);

    void check_types(const string &type1, const string &type2, int lineno);

    void add_var(const string &name, const string &type, bool isFunc, int lineno);

    void add_scope();

    bool remove_scope();

    void decl_func(const string& name, const string& type, const string& ret_val , string& arg1 , int lineno);

    void PrintScope(table scope);

    void init_global_table();

    void does_main_exist();

    string funcType(const string &func_name, const string &args_types, int lineno);

    void insideLoop(int loopsCnt , string kind , int lineno);

};


#endif //COMPY3_HW3_AUX_H