
#include "hw3_aux.h"

#include <cstring>


using namespace std;

static string replace(string str, string substr1, string substr2) {
    for (size_t index = str.find(substr1, 0);
         index != string::npos && substr1.length(); index = str.find(substr1, index + substr2.length()))
        str.replace(index, substr1.length(), substr2);
    return str;
}

const arg *symbol::get_var(const string &unique_name, bool isFunc) {
    for (auto const &table: t_stack) {
        for (auto const &entry : table) {
            if (entry.name == unique_name) {
                if (isFunc && (entry.type == "INT" || entry.type == "BYTE" || entry.type == "BOOL"))
                    continue;
                return &entry;
            }
        }
    }
    return nullptr;
}

Node *symbol::makeNodeFromID(const string &id, int lineno) {
    const arg *var = get_var(id, false);
    if (var != nullptr) {
        string ptr = freshVar();
        string reg = freshVar();
        CB.emit(ptr + " = getelementptr [50 x i32], [50 x i32]*, " +  llvm_stack_reg + ", i32 0, i32 " + to_string(var->offset));
        CB.emit(reg +" = load i32, i32* " + ptr);
        return new Node(var->name, var->type, reg);
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

void symbol::forceIntoReg(Node* node) {
    if(node->type == "BYTE") {
        CB.emit(node->reg +" = trunc i32 " + bstoi(node->value) + "to i8");
    } else if (node->type == "STRING") {
        CB.emit(node->reg +" = " + node->value + "\00");
    } else CB.emit(node->reg +" = add i32 0, " + node->value);
}

void symbol::add_var(const string &name, const string &type, bool isFunc, int lineno) {
    if (get_var(name, isFunc) != nullptr) {
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

static void tokenize(string const &str, const char *delim, vector<string> &out) {
    char *token = strtok(const_cast<char *>(str.c_str()), delim);
    while (token != nullptr) {
        out.emplace_back(string(token));
        token = strtok(nullptr, delim);
    }
}

void symbol::decl_func(const string &name, const string &type, const string &ret_val, string &arg1, int lineno) {
    vector<string> in_out, types, args; //accept type like: int,float,string->void
    tokenize(arg1, ",", args);
    tokenize(type, ",", types);
    if (args.size() != types.size()) {
        output::errorPrototypeMismatch(lineno, name, types);
        exit(-1);
    }
    string func_type = output::makeFunctionType(ret_val, types);
    add_func(name, func_type, lineno);
    add_scope();
    CB.emit("define " + ret_val + " @" + name + "(" + type + ") {");
    void initRegIdx();
    input_llvm_stack_reg = freshVar();
    string input_size = to_string(args.size());
    if (args.size() > 0)
        CB.emit(input_llvm_stack_reg + " = alloca [" + input_size + " x i32]");
    llvm_stack_reg = freshVar();
    CB.emit(llvm_stack_reg + " = alloca [50 x i32]");

    for (int i = 0; i < (int) args.size(); i++) {
        const arg *tmp = get_var_type(args[i], types[i]);
        if (tmp != nullptr) {
            output::errorDef(lineno, args[i]);
            exit(-1);
        }
        t_stack.back().emplace_back(arg(args[i], types[i], -i - 1));
        string address = freshVar();
        CB.emit(address + " = getelementptr [" + input_size + " x i32], [" + input_size + " x i32]* " +
                input_llvm_stack_reg + ",i32 0, i32 " + to_string(i));
        //CB.emit("store i32 " + args[i]->name + ", i32* " + address); //TODO: change to tmp->reg or tmp->val...?
    }
}

void symbol::PrintScope(table scope) {
    for (int i = 0; i < scope.size(); ++i) {
        output::printID(scope[i].name, scope[i].offset, scope[i].type);
    }
}

void symbol::init_global_table() {
    vector<string> print, printi;
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
        if (type == "INT" && get_var_type(name, "BYTE") != nullptr) {
            output::errorMismatch(lineno);
            exit(-1);
        }
    } else if (get_var_type(name, type) == nullptr) {
        output::errorUndef(lineno, name);
        exit(-1);
    }

}

void symbol::init_var_in_llvmStack(const string &name, const string &type, int lineno) {
    string valueReg = freshVar();
    if (type == "INT" || type == "BYTE") {
        CB.emit("valueReg = add i32 0 , 0");
    } else {
        // TODO: init valueReg to false!!
    }
    assign_value(name, type, lineno, valueReg);
}

void symbol::assign_value(const string &name, const string &type, int lineno, const string &reg) {

    const arg *arg1 = get_var_type(name, type);
    cout << arg1 << endl;
    string varReg = freshVar();
    CB.emit(varReg + " = getelementprt [50 x i32], [50 x i32]*, " + llvm_stack_reg + ", i32 0, i32 " +
            to_string(arg1->offset));
    CB.emit("store i32 " + reg + ", i32* " + varReg);
}


void symbol::assign_check_types(const string &type1, const string &type2, int lineno) {
    if ((type1 == "INT" && type2 == "BYTE") || (type1 == type2))
        return;
    else {
        output::errorMismatch(lineno);
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
    const arg *f = get_var(func_name, true);
    if (f == nullptr) {
        output::errorUndefFunc(lineno, func_name);
        exit(-1);
    }
    vector<string> expected_args, received_args, exp_in_out, empty_in;
    string exp_types_as_string = replace(f->type, "(", ""), rec_args_types_as_string = args_types;
    exp_types_as_string = replace(exp_types_as_string, ")", "");

    tokenize(exp_types_as_string, "->", exp_in_out);
    if (exp_in_out.size() == 1) {
        if (args_types == "") {
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

    for (int i = 0; i < (int) expected_args.size(); i++) {
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

void symbol::onlyOneMain(int lineno, const string &name) {
    if ((this->get_var(name, true) != nullptr) && name == "main") {
        output::errorDef(lineno, name);
        exit(-1);
    }
}

void symbol::check_valid_b(const string &name, int lineno) {
    if (stoi(name) > 255) {
        output::errorByteTooLarge(lineno, name);
        exit(-1);
    }

}

void symbol::init_llvm_stack() {
    CB.emitGlobal("declare i32 @printf(i8*, ...)");
    CB.emitGlobal("declare void @exit(i32)");
//constants decl
    CB.emitGlobal("@.stzero = constant [23 x i8] c\"Error division by zero\\00\"");
    CB.emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    CB.emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
//print functions decl
    CB.emitGlobal("define void @print(i8*) {");
    CB.emitGlobal(
            "call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0), i8* %0)");
    CB.emitGlobal("ret void");
    CB.emitGlobal("}");

    CB.emitGlobal("define void @printi(i32) {");
    CB.emitGlobal(
            "call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0), i32 %0)");
    CB.emitGlobal("ret void");
    CB.emitGlobal("}");

}

void symbol::init_truelist(Node* node){
    int true_label = CB.emit("br label @");
    node->truelist=  CB.makelist({true_label,FIRST});
}

void symbol::init_falselist(Node *node) {
    int false_label = CB.emit("br label @");
    node->falselist=  CB.makelist({false_label,FIRST});
}