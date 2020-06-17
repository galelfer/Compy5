
#ifndef COMPY3_PARSER_H
#define COMPY3_PARSER_H

#ifndef FOR_COMPI_PARSER_H
#define FOR_COMPI_PARSER_H

#include "hw3_output.hpp"
#define YYSTYPE Node*
extern int yylineno;


class Node {
public:
    string name;
    string type;
    string value;
    Node() = default;
    Node(const string &name, const string &type, const string &value) : name(name), type(type), value(value) {}
    Node(const string &name, const string &type) : name(name), type(type), value("") {}
};

#endif //FOR_COMPI_PARSER_H


#endif //COMPY3_PARSER_H
