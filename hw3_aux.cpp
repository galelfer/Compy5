
#include "hw3_aux.h"

#include <cstring>


using namespace std;

static string replace(string str, string substr1, string substr2)
{
    for (size_t index = str.find(substr1, 0); index != string::npos && substr1.length(); index = str.find(substr1, index + substr2.length()))
        str.replace(index, substr1.length(), substr2);
    return str;
}

const arg *symbol::get_var(const string &unique_name) {
    for (auto const &table: t_stack) {
        for (auto const &entry : table) {
            if (entry.name == unique_name) {
                return &entry;
            }
        }
    }
    return nullptr;
}

Node *symbol::makeNodeFromID(const string &id, int lineno) {
    const arg *var = get_var(id);
    if (var != nullptr) {
        string reg = freshVar();

        //TODO: Freshvar() - make a new reg for value.
        // (llvm) find ID value by it's offset in llvm_stack. - getelemntptr returns ptr. than we need to freshVar another reg for ptr.
        // (llvm) load ID value into the new reg.
        // c++: tell Node the reg_num.
        return new Node(var->name, var->type, "");
    }

    output::errorUndef(lineno, id);
    exit(-1);
}

const arg *symbol::get_var_type(const string &name, const string &type) {
    for (auto const &table: t_stack) {
        for (auto const &entry : table) {
            if ((entry.name == name) && (entry.type == type)) {
                return &entry;
            }
        }
    }
    return nullptr;
}

void symbol::add_var(const string &name, const string &type, bool isFunc, int lineno) {
    if (get_var_type(name, type) != nullptr) {
        output::errorDef(lineno, name);
        exit(-1);
    }
    ll offset;
    if (isFunc) {
        offset = 0;
    } else {
        offset = o_stack.back();
        o_stack.back() = offset + 1;
    }

    t_stack.back().emplace_back(arg(name, type, offset));
}

void symbol::add_func(const string &name, const string &type, int lineno) {
    add_var(name, type, true, lineno);
}

void symbol::add_scope() {
    t_stack.emplace_back(table());
    o_stack.emplace_back(o_stack.back());
}

bool symbol::remove_scope() {
    if (t_stack.empty() || o_stack.empty())
        return false;
    t_stack.pop_back();
    o_stack.pop_back();
    return true;
}

static void tokenize(string const &str, const char *delim, vector <string> &out) {
    char *token = strtok(const_cast<char *>(str.c_str()), delim);
    while (token != nullptr) {
        out.emplace_back(string(token));
        token = strtok(nullptr, delim);
    }
}

void symbol::decl_func(const string &name, const string &type, const string &ret_val, string &arg1, int lineno) {
    vector <string> in_out, types, args; //accept type like: int,float,string->void
    tokenize(arg1, ",", args);
    tokenize(type, ",", types);
    if (args.size() != types.size()) {
        output::errorPrototypeMismatch(lineno, name, types);
        exit(-1);
    }
    string func_type = output::makeFunctionType(ret_val, types);
    add_func(name, func_type, lineno);
    add_scope();
    //TODO: open a new stack, with enough place for args + 50 (or 2 different stacks). - > emit("alloca..") (llvm).
    // C++: update llvm_stack_reg to be it's register (and input_llvm_stack_reg as well?).
    // recommanded: init NUM counter for FreshVar() so it won't overflow.
    for (int i = 0; i < (int) args.size(); i++) {
        const arg* tmp = get_var_type(args[i],types[i]);
                if(tmp!=nullptr){
                    output::errorDef(lineno,args[i]);
                    exit(-1);
                }
        (t_stack.back().emplace_back(arg(args[i], types[i], -i - 1)));
    }
}

void symbol::PrintScope(table scope) {
    for (int i = 0; i < scope.size(); ++i) {
        output::printID(scope[i].name, scope[i].offset, scope[i].type);
    }
}

void symbol::init_global_table() {
    vector <string> print, printi;
    this->t_stack[0].emplace_back(arg("print", "(STRING)->VOID", 0));
    this->t_stack[0].emplace_back(arg("printi", "(INT)->VOID", 0));
}


void symbol::does_main_exist() {
    for (int j = 0; j < this->t_stack[0].size(); j++) {
        if (this->t_stack[0][j].name == "main" && this->t_stack[0][j].type == "()->VOID") {
            return;
        }
    }

    output::errorMainMissing();
    exit(-1);
}


void symbol::assign(const string &name, const string &type, int lineno) {
    if (type == "INT" || type == "BYTE") {
        if (get_var_type(name, "INT") == nullptr && get_var_type(name, "BYTE") == nullptr) {
            output::errorUndef(lineno, name);
            exit(-1);
        }
    } else if (get_var_type(name, type) == nullptr) {
        output::errorUndef(lineno, name);
        exit(-1);
    }
}

void symbol::check_types(const string &type1, const string &type2, int lineno) {
    if ((type1 == "INT" && type2 == "BYTE") || (type2 == "INT" && type1 == "BYTE") || (type1 == type2))
        return;
    else {
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
    const arg *f = get_var(func_name);
    if (f == nullptr) {
        output::errorUndef(lineno, func_name);
        exit(-1);
    }
    vector <string> expected_args, received_args, exp_in_out, empty_in;
    string exp_types_as_string = replace(f->type, "(", "") , rec_args_types_as_string = args_types;
    exp_types_as_string = replace(exp_types_as_string, ")", "");

    tokenize(exp_types_as_string, "->", exp_in_out);
    if (exp_in_out.size() == 1) {
        if(args_types == "") {
            return exp_in_out[0];
        } else {
            output::errorPrototypeMismatch(lineno, func_name, empty_in);
            exit(-1);
        }
    }
    tokenize(exp_in_out[0], ",", expected_args);
    tokenize(args_types, ",", received_args);

    if (expected_args.size() != received_args.size()) {
        output::errorPrototypeMismatch(lineno, func_name, expected_args);
        exit(-1);
    }

    for(int i=0; i < (int)expected_args.size(); i++) {
        if (!(expected_args[i] == received_args[i] || (expected_args[i] == "INT" && received_args[i] == "BYTE"))) {
            output::errorPrototypeMismatch(lineno, func_name, expected_args);
            exit(-1);
        }
    }
    return exp_in_out[1];
}

void symbol::insideLoop(int loopsCnt, string kind, int lineno) {
    if (loopsCnt == 0) {
        if (kind == "continue") output::errorUnexpectedContinue(lineno);
        if (kind == "break") output::errorUnexpectedBreak(lineno);
        exit(-1);
    }
}

void symbol::onlyOneMain(int lineno , const string &name){
    if((this->get_var(name)!=nullptr) && name=="main"){
        output::errorDef(lineno,name);
        exit(-1);
    }
}

void symbol::check_valid_b(const string &name , int lineno){
    if(stoi(name)>255){
        output::errorByteTooLarge(lineno,name);
        exit(-1);
    }

}