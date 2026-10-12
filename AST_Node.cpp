#include "AST_Node.h"

string AST_NodeTypeToString(AST_NodeType type) {
    switch (type) {
        // meta stuff
        case AST_NodeType::PROGRAM: return "PROGRAM";
        case AST_NodeType::BLOCK: return "BLOCK";

        // functions
        case AST_NodeType::FUNC_DECL: return "FUNC_DECL";
        case AST_NodeType::FUNC_DEF: return "FUNC_DEF";
        case AST_NodeType::FUNC_CALL: return "FUNC_CALL";

        // param & param list
        case AST_NodeType::PARAM: return "PARAM";
        case AST_NodeType::PARAM_LIST: return "PARAM_LIST";

        // literals
        case AST_NodeType::LITERAL: return "LITERAL";

        // variable
        case AST_NodeType::VAR: return "VAR";

        // expressions and operators
        case AST_NodeType::EXPR_STMNT: return "EXPR_STMNT";
        case AST_NodeType::UNARY_EXPR: return "UNARY_EXPR";
        case AST_NodeType::BINARY_EXPR: return "BINARY_EXPR";
        case AST_NodeType::CAST: return "CAST";

        // arrays
        case AST_NodeType::ARRAY_DECL: return "ARRAY_DECL";
        case AST_NodeType::ARRAY_ACCESS: return "ARRAY_ACCESS";

        // variable init/decl
        case AST_NodeType::VAR_DECL_INIT: return "VAR_DECL_INIT";

        // assign
        case AST_NodeType::ASSIGN_STMNT: return "ASSIGN_STMNT";

        // if statement
        case AST_NodeType::IF: return "IF";
        case AST_NodeType::ELSE: return "ELSE";
        case AST_NodeType::IF_STMNT: return "IF_STMNT";

        // while statement
        case AST_NodeType::WHILE_STMNT: return "WHILE_STMNT";

        // return statement
        case AST_NodeType::RETURN_STMNT: return "RETURN_STMNT";

        default: return "UNKNOWN_NODE_TYPE";
    }
}

