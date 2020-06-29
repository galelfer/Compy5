
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

Node *symbol::makeNodeFromID(Node *node, int lineno) {
    const arg *var = get_var(node->name, false);
    string args_stack_size = to_string(input_llvm_stack_reg_size);
    if (var != nullptr) {
        string ptr = freshVar();
        string reg = freshVar();
        if (var->offset < 0) {
            int arg_offset = (-1 * (var->offset)) - 1;
            CB.emit(ptr + " = getelementptr [" + args_stack_size + " x i32], [" + args_stack_size + " x i32]* " +
                    input_llvm_stack_reg + ", i32 0, i32 " + to_string(arg_offset));
            if (var->type == "BOOL") {
                CB.emit(reg + " = load i32, i32* " + ptr);
                string tmp = reg;
                reg = freshVar();
                CB.emit(reg + " = icmp eq i32 " + tmp + ", 1 ");
            } else {
                CB.emit(reg + " = load i32, i32* " + ptr);
            }

        } else {
            CB.emit(ptr + " = getelementptr [50 x i32], [50 x i32]* " + llvm_stack_reg + ", i32 0, i32 " +
                    to_string(var->offset));


            if (var->type == "BOOL") {
                CB.emit(reg + " = load i32, i32* " + ptr);
                string tmp = reg;
                reg = freshVar();
                CB.emit(reg + " = icmp eq i32 " + tmp + ", 1 ");
            } else {
                CB.emit(reg + " = load i32, i32* " + ptr);
            }
        }

        Node *res = new Node(var->name, var->type, reg);
        res->truelist = node->truelist;
        res->falselist = node->falselist;
        res->nextlist = node->nextlist;
        res->breaklist = node->breaklist;
        res->continuelist = node->continuelist;
        return res;
    }

    output::errorUndef(lineno, node->name);
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

void symbol::forceIntoReg(Node *node) {
    if (node->type == "STRING") {
        node->reg = emitString(node->value);
    } else CB.emit(node->reg + " = add i32 0, " + node->value);
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

static void tokenize(string const str, const char *delim, vector<string> &out) {
    char *token = strtok(const_cast<char *>(str.c_str()), delim);
    while (token != nullptr) {
        out.emplace_back(string(token));
        token = strtok(nullptr, delim);
    }
}

void symbol::decl_func(const string &name, const string &type, const string &ret_val, string &arg1, int lineno) {
    vector<string> in_out, types, args; //accept type like: int,float,string->void
    string type1 = type;
    string arg3 = arg1;

    tokenize(arg3, ",", args);
    tokenize(type1, ",", types);
    if (args.size() != types.size()) {
        output::errorPrototypeMismatch(lineno, name, types);
        exit(-1);
    }
    string func_type = output::makeFunctionType(ret_val, types);
    add_func(name, func_type, lineno);
    add_scope();
    string input_size = to_string(args.size());

    vector<string> new_args;
    string args_list = "";
    string arg2;
    for (int i = 0; i < types.size(); i++) {

        arg2 = freshArg();
        new_args.push_back(arg2);
        if (i == (types.size() - 1))
            args_list += "i32 " + arg2;
        else
            args_list += "i32 " + arg2 + ", ";
    }


    CB.emit("define " + ret_value(ret_val) + " @" + name + "(" + args_list + ") {");
    initRegIdx();
    llvm_stack_reg = freshVar();
    input_llvm_stack_reg = freshVar();
    input_llvm_stack_reg_size = args.size();
    CB.emit(llvm_stack_reg + " = alloca [50 x i32]");
    if (args.size() > 0)
        CB.emit(input_llvm_stack_reg + " = alloca [" + to_string(args.size()) + " x i32]");
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
        CB.emit("store i32 " + new_args[i] + ", i32* " + address);

    }
}


void symbol::finishDeclFunc(string &type) {
    (type == "VOID") ? CB.emit("ret void") : CB.emit("ret i32 0");
    CB.emit("}");
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

string symbol::init_var_in_llvmStack(const string &name, const string &type, int lineno) {
    string valueReg = freshVar();
    CB.emit(valueReg + " = add i32 0 , 0");
    return assign_value(name, type, lineno, valueReg);
}

string symbol::assign_value(const string &name, const string &type, int lineno, const string &reg) {

    const arg *arg1 = get_var_type(name, type);
    string varReg = freshVar();
    if (arg1->offset < 0) {
        CB.emit(varReg + " = getelementptr [" + to_string(input_llvm_stack_reg_size) + " x i32], [" +
                to_string(input_llvm_stack_reg_size) + " x i32]* " + input_llvm_stack_reg + ", i32 0, i32 " +
                to_string((-1 * arg1->offset) - 1));
        CB.emit("store i32 " + reg + ", i32* " + varReg);
        return varReg;
    } else {
        CB.emit(varReg + " = getelementptr [50 x i32], [50 x i32]* " + llvm_stack_reg + ", i32 0, i32 " +
                to_string(arg1->offset));
        CB.emit("store i32 " + reg + ", i32* " + varReg);
        return varReg;
    }

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

    string f_type = f->type;
    vector<string> expected_args, received_args, exp_in_out, empty_in;
    string exp_types_as_string = replace(f_type, "(", ""), rec_args_types_as_string = args_types;
    exp_types_as_string = replace(exp_types_as_string, ")", "");
    tokenize(exp_types_as_string, "->", exp_in_out);

    if (exp_in_out.size() == 1) {
        if (args_types == "") {
            return exp_in_out[0];
        } else {
//            output::errorPrototypeMismatch(lineno, func_name, empty_in);
//            exit(-1);
            return "VOID";
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
//print  stacks decl


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


void symbol::init_truelist(Node *node) {
    int true_label = CB.emit("br label @   ; for true");
    node->truelist = CB.makelist({true_label, FIRST});

}

void symbol::init_falselist(Node *node) {
    int false_label = CB.emit("br label @  ; for false");
    node->falselist = CB.makelist({false_label, FIRST});
}

void symbol::boolean_evaluation(Node *exp) {
    if (exp->type != "BOOL")
        return;
    //these 3 lines are not redundant , they resolve a syntax error!!
    //int line1 = CB.emit("br label @");
    string true_label = CB.genLabel();
    //CB.bpatch(CB.makelist({line1,FIRST}), true_label);

    int from_true = CB.emit("br label @");
    string false_label = CB.genLabel();
    int from_false = CB.emit("br label @");
    CB.bpatch(exp->truelist, true_label);
    CB.bpatch(exp->falselist, false_label);

    //int line3 = CB.emit("br label @");
    string label3 = CB.genLabel();
    //CB.bpatch(CB.makelist({line3,FIRST}), label3);

    CB.bpatch(CodeBuffer::makelist({from_true, FIRST}), label3);
    CB.bpatch(CodeBuffer::makelist({from_false, FIRST}), label3);
    exp->reg = freshVar();
    CB.emit(exp->reg + " = phi i32 [ 1, %" + true_label + " ], [ 0, %" + false_label + " ]");
}

void symbol::swap_truelist_falselist(Node *resExp, Node *exp) {


//    resExp->truelist  = exp->falselist;
//    resExp->falselist = exp->truelist;

    vector<pair<int, BranchLabelIndex>> true_list = exp->truelist;
    resExp->truelist = exp->falselist;
    resExp->falselist = true_list;

}


string symbol::switch_relop(string rel) {

    if (rel == "==") return "eq";
    if (rel == "!=") return "ne";
    if (rel == "<") return "slt";
    if (rel == "<=") return "sle";
    if (rel == ">") return "sgt";
    else return "sge"; // for the relop ">="

}


void symbol::and_backpatch(Node *res, Node *first, Node *second, string marker_label) {
    CB.bpatch(first->truelist, marker_label);
    res->truelist = second->truelist;
    res->falselist = CB.merge(first->falselist, second->falselist);
}

void symbol::or_backpatch(Node *res, Node *first, Node *second, string marker_label) {
    CB.bpatch(first->falselist, marker_label);
    res->truelist = CB.merge(first->truelist, second->truelist);
    res->falselist = second->falselist;
}

void symbol::relop_evaluation(Node *res, string op, string arg1, string arg2) {
    string reg = freshVar();
    CB.emit(reg + " = icmp " + switch_relop(op) + " i32 " + arg1 + ", " + arg2);
    int line1 = CB.emit("br i1 " + reg + ", label @, label @");
    res->truelist = CB.makelist({line1, FIRST});
    res->falselist = CB.makelist({line1, SECOND});
}

void symbol::bool_evaluation_for_call(Node *node) {
    if (node->type == "BOOL") {
        int line1 = CB.emit("br i1 " + node->reg + ", label @, label @");
        node->truelist = CB.makelist({line1, FIRST});
        node->falselist = CB.makelist({line1, SECOND});
//        string reg =freshVar();
//        CB.emit(reg + " = icmp eq i32 " + node->reg + ", 1 ");
//        CB.emit("##############################################################");
//        node->reg = reg;
    }
}

string symbol::ret_value(string ret) {
    if (ret == "VOID") return "void";
    return "i32";
}

void symbol::if_backpatching(Node *res, Node *exp, Node *statement, string marker_label) {

    int line1 = CB.emit("br label @");
    string next_label = CB.genLabel();
    CB.bpatch(CB.makelist({line1, FIRST}), next_label);

    //cout<<exp->truelist.size()<<"   label : "<< marker_label <<"$$$$$$$$$$$$$$$$$$$"<<endl  ;

    CB.bpatch(exp->truelist, marker_label);
    CB.bpatch(exp->falselist, next_label);
    CB.bpatch(statement->nextlist, next_label);
    //res->nextlist=CB.merge(exp->falselist,statement->nextlist);
    //cout<<statement->breaklist.size()<<endl;
    res->breaklist = statement->breaklist;
    res->continuelist = statement->continuelist;
}

void symbol::if_else_backpatch(Node *res, Node *exp, Node *statement1, Node *N, Node *statement2, Node *M2,
                               string M1_label) {

    int line1 = CB.emit("br label @");
    string next_label = CB.genLabel();
    CB.bpatch(CB.makelist({line1, FIRST}), next_label);

    CB.bpatch(exp->truelist, M1_label);
    CB.bpatch(exp->falselist, M2->name);
    CB.bpatch(statement1->nextlist, next_label);
    CB.bpatch(N->nextlist, next_label);
    CB.bpatch(statement2->nextlist, next_label);

    res->nextlist = CB.merge(CB.merge(statement1->nextlist, N->nextlist), statement2->nextlist);
    res->breaklist = CB.merge(statement1->breaklist, statement2->breaklist);
    res->continuelist = CB.merge(statement1->continuelist, statement2->continuelist);
}

void symbol::exit_loop(Node *res) {
    int line1 = CB.emit("br label @    ; break ");
    res->breaklist = CB.makelist({line1, FIRST});
}

void symbol::skip_loop(Node *res) {
    int line1 = CB.emit("br label @   ; continue");
    res->continuelist = CB.makelist({line1, FIRST});
}

void symbol::while_backpatch(Node *res, Node *exp, Node *statement, Node *marker1, Node *marker2) {
    CB.emit("br label %" + marker1->name);
    int line1 = CB.emit("br label @");
    string next_label = CB.genLabel();
    CB.bpatch(CB.makelist({line1, FIRST}), next_label);
    CB.bpatch(statement->nextlist, marker1->name);
    CB.bpatch(exp->truelist, marker2->name);
    CB.bpatch(statement->continuelist, marker1->name);
    CB.bpatch(exp->falselist, next_label);
    CB.bpatch(statement->breaklist, next_label);

}

void
symbol::while_else_backpatch(Node *res, Node *exp, Node *statement1, Node *statement2, Node *marker1, Node *marker2,
                             Node *marker3, Node *skip_marker) {

    int line1 = CB.emit("br label @");
    string next_label = CB.genLabel();
    CB.bpatch(CB.makelist({line1, FIRST}), next_label);

    CB.bpatch(statement1->nextlist, marker1->name);
    CB.bpatch(statement2->nextlist, next_label);
    CB.bpatch(exp->truelist, marker2->name);
    CB.bpatch(statement1->continuelist, marker1->name);
    CB.bpatch(exp->falselist, marker3->name);
    CB.bpatch(statement1->breaklist, next_label);
    CB.bpatch(skip_marker->nextlist, marker1->name);

}

void symbol::function_call(const string &name, Node *explist, string resReg) {

    vector<string> args, in_out, types;
    string explist_regs = explist->reg;
    string explist_types = explist->type;
    tokenize(explist_regs, ",", args);
    tokenize(explist_types, ",", types);
    string input_size = to_string(args.size());
    const arg *func = get_var(name, true);
    string f_type = func->type;
    tokenize(f_type, "->", in_out);
    string func_arg = "";
    string retType;


    string str_size = to_string(explist->value.size() - 4);
    string input_stack = freshVar();


    for (int i = 0; i < args.size(); i++) {
        if (types[i] != "STRING") {
            if (i > 0)
                func_arg = func_arg + " ,i32 " + args[i];
            else
                func_arg = func_arg + "i32 " + args[i];
        } else {
            if (i > 0)
                func_arg = func_arg + " ,i8* " + args[i];
            else
                func_arg = func_arg + "i8* getelementptr ([" + to_string(explist->value.size() - 4) + " x i8], [" +
                           to_string(explist->value.size() - 4) + " x i8]* " + args[i] + ", i32 0, i32 0)";
        }

    }

    retType = funcType(name, explist->type, 0);
    if (retType != "VOID"){
        if(retType=="BOOL"){
           string tmp_res_reg = freshVar();
            CB.emit(tmp_res_reg + " = call i32 @" + name + "(" + func_arg + ")");
            CB.emit(resReg + " = icmp eq i32 " + tmp_res_reg + ", 1 ");
        }
        else
            CB.emit(resReg + " = call i32 @" + name + "(" + func_arg + ")");
    }

    else
        CB.emit("call void @" + name + "(" + func_arg + ")");

}

void symbol::function_call_no_args(const string &name, string resReg) {
    vector<string> in_out;
    const arg *func = get_var(name, true);
    string f_type = func->type;
    tokenize(f_type, "->", in_out);
    string retType = funcType(name, "", 0);
    if (retType != "VOID"){
        if(retType=="BOOL"){
            string tmp_res_reg = freshVar();
            CB.emit(tmp_res_reg + " = call i32 @" + name + "()");
            CB.emit(resReg + " = icmp eq i32 " + tmp_res_reg + ", 1 ");
        }
        else
            CB.emit(resReg + " = call i32 @" + name + "()");
    }

    else
        CB.emit("call void @" + name + "()");
}


