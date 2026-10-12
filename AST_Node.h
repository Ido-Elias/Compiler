#ifndef AST_NODE_H
#define AST_NODE_H

#include <memory>

#include "DataType.h"
#include "Token.h"

enum class AST_NodeType {
    //meta stuff
    PROGRAM,
    BLOCK,

    //functions
    FUNC_DECL,
    FUNC_DEF,
    FUNC_CALL,

    //param & param list
    PARAM,
    PARAM_LIST,

    //literals
    LITERAL,

    //variable
    VAR,

    //expressions
    EXPR_STMNT,
    UNARY_EXPR,
    BINARY_EXPR,
    CAST,

    //arrays
    ARRAY_DECL,
    ARRAY_ACCESS,

    //variable init/decl
    VAR_DECL_INIT,

    //assign
    ASSIGN_STMNT,

    //if statement
    IF,
    ELSE,
    IF_STMNT,

    //while statement
    WHILE_STMNT,

    //return statement
    RETURN_STMNT
};

string AST_NodeTypeToString(AST_NodeType type);




struct AST_Node {
    AST_NodeType type;
    Token mainToken;
    vector<unique_ptr<AST_Node>> children;

    DataType data_type = DataType::NONE;
    size_t ptrCount = 0;

    AST_Node(AST_NodeType type, const Token& mainToken = Token(), DataType data_type = DataType::NONE, size_t ptrCount = 0)
        : type(type), mainToken(mainToken), data_type(data_type), ptrCount(ptrCount) {}


    AST_Node(AST_NodeType type, const Token& mainToken, unique_ptr<AST_Node> child1, unique_ptr<AST_Node> child2) //for stuff with two children
        : type(type), mainToken(mainToken) {
        children.emplace_back(move(child1));
        children.emplace_back(move(child2));
    }

    ~AST_Node() = default;

    string toStringRec(int depth) const {
        string ret = "";
        ret += AST_NodeTypeToString(type);
        if(mainToken.type != TokenType::NONE) ret += " main token: " + tokenTypeToString(mainToken.type);
        ret += isValued(mainToken.type)? " (" + mainToken.lexeme + ")" : ""; //optional value specifier
        ret += "\n";
        for(const auto& it : children) {
            for(int i = 0; i <= depth; i++) {
                ret += "  |";
            }
            ret += " ";
            ret += it->toStringRec(depth+1);
        }

        return ret;
    }

    string toString() const {
        return toStringRec(0);
    }



};

#endif //AST_NODE_H
