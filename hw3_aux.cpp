//
// Created by ezoabi on 31/05/2020.
//

#include "hw3_output.hpp"
#include "hw3_aux.h"
#include <cstring>


using namespace std;

const arg* symbol::get_var(const string &unique_name){
    for (auto const &table: t_stack) {
        for (auto const &entry : table) {
            if (entry.name == unique_name) {
                return &entry;
            }
        }
    }
    return nullptr;
}

Node* symbol::makeNodeFromID(const string& id, int lineno) {
    const arg* var = get_var(id);
    if(var != nullptr)
        return new Node(var->name, var->type);

    output::errorUndef(lineno, id);
    exit(-1);
}

const arg* symbol::get_var_type(const string& name, const string& type) {
    for (auto const &table: t_stack) {
        for (auto const &entry : table) {
            if ((entry.name == name) && (entry.type == type)) {
                return &entry;
            }
        }
    }
    return nullptr;
}

void symbol::add_var(const string& name, const string& type, bool isFunc, int lineno) {
    if (get_var_type(name, type) != nullptr) {
        output::errorDef(lineno, name);
        exit(-1);
    }
    ll offset;
    if (isFunc) {
        offset = 0;
    } else {
        offset = o_stack.back();
    }
    o_stack.back() = offset + 1;
    t_stack.back().emplace_back( arg(name, type, offset));
}

void symbol::add_func(const string& name, const string& type , int lineno) {
    add_var(name, type, true, lineno);
}

void symbol::add_scope() {
    t_stack.emplace_back(table());
    o_stack.emplace_back(o_stack.back());
}

bool symbol::remove_scope() {
    if(t_stack.empty() || o_stack.empty())
        return false;
    t_stack.pop_back();
    o_stack.pop_back();
    return true;
}

static void tokenize(string const &str, const char* delim, vector<string>& out)
{
    char *token = strtok(const_cast<char*>(str.c_str()), delim);
    while(token != nullptr) {
        out.emplace_back(string(token));
        token = strtok(nullptr, delim);
    }
}

void symbol::decl_func(const string& name, const string& type, const string& ret_val , string& arg1 , int lineno) {
    vector<string> in_out, types,args; //accept type like: int,float,string->void
    tokenize(arg1,",",args);
    tokenize(type , "," ,types );
    if(args.size() != types.size()) {
        output::errorPrototypeMismatch(lineno, name, types);
        exit(-1);
    }
    string func_type=output::makeFunctionType(ret_val,types);
    add_func(name , func_type , lineno);
    for(int i=0; i < (int)args.size(); i++) {
        (t_stack.back().emplace_back( arg(types[i], args[i], -i-1)));
    }
}

void symbol::PrintScope(table scope){
    for (int i = 0; i <scope.size() ; ++i) {
        output::printID(scope[i].name,0,scope[i].type);
    }
}

void symbol::init_global_table(){
    vector<string> print, printi ;
    this->t_stack[0].emplace_back( arg("print","(STRING)->VOID",0));
    this->t_stack[0].emplace_back( arg("printi","(INT)->VOID",0));
}


void symbol::does_main_exist(){
    for(int j=0 ; j< this->t_stack[0].size() ; j++){
        if(this->t_stack[0][j].name=="main" && this->t_stack[0][j].type=="()->VOID" ){
            return;
        }
    }

    output::errorMainMissing();
    exit(-1);
}


void symbol::assign(const string &name, const string &type, int lineno) {
    if (get_var_type(name, type) == nullptr) {
        output::errorUndef(lineno, name);
        exit(-1);
    }
}

void symbol::check_types(const string &type1, const string &type2, int lineno) {
    if((type1 == "int" && type2 == "byte") || (type2 == "int" && type1 == "byte") || (type1 == type2) )
        return;
    else{
        output::errorMismatch(lineno);
        exit(-1);
    }
}

string symbol::larger(const string &type1, const string &type2) {
    if (type1 == "INT" || type2 == "INT")
        return "INT";
    return "BYTE";
}

string symbol::funcType(const string &func_name, const string &args_types, int lineno) {
    const arg* f = get_var(func_name);
    if (f == nullptr) {
        output::errorUndef(lineno, func_name);
        exit(-1);
    }
    vector <string> in_out;
    tokenize(f->type, "->", in_out);
    //TODO: switch all "byte" with "int".
    string braced_args_types = "(" + args_types + ")";
    if (in_out[0] == braced_args_types)
        return in_out[1];

    vector <string> types;
    int len = in_out[0].size();
    in_out[0].erase(in_out[0].begin() + len-1);
    in_out[0].erase(in_out[0].begin());
    //TODO: remove first and last char from in_out[0].
    tokenize(in_out[0], ",", types);
    output::errorPrototypeMismatch(lineno, func_name, types);
    exit(-1);
}

void symbol::insideLoop(int loopsCnt ,  string kind , int lineno){
    if(loopsCnt==0 ){
        if(kind=="continue") output::errorUnexpectedContinue(lineno);
        if(kind=="break") output::errorUnexpectedBreak(lineno);
        exit(-1);
    }
}