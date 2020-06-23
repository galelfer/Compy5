
#ifndef COMPY3_PARSER_H
#define COMPY3_PARSER_H

#ifndef FOR_COMPI_PARSER_H
#define FOR_COMPI_PARSER_H

#include "hw3_output.hpp"
#include "bp.hpp"
#define YYSTYPE Node*
extern int yylineno;

static CodeBuffer& CB = CodeBuffer::instance();


class Node {
public:
    string name;
    string type;
    string value;
    string reg;
    Node() = default;
    Node(const string &name, const string &type, const string &value, const string &reg) : name(name), type(type), value(value), reg(reg) {}
    Node(const string &name, const string &type, const string &reg) : name(name), type(type), value("") , reg(reg) {}
};

#endif //FOR_COMPI_PARSER_H


#endif //COMPY3_PARSER_H
