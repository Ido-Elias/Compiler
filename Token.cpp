#include "Token.h"

#include <iostream>
#include <vector>

#include "AST_Node.h"

string tokenTypeToString(TokenType type) {
    switch (type) {
        // --- Types & Keywords ---
        case TokenType::INT: return "int";
        case TokenType::FLOAT: return "float";
        case TokenType::CHAR: return "char";
        case TokenType::BOOL: return "bool";
        case TokenType::VOID: return "void";
        case TokenType::IF: return "if";
        case TokenType::ELSE: return "else";
        case TokenType::WHILE: return "while";
        case TokenType::RETURN: return "return";

        // --- Identifiers & Literals ---
        case TokenType::IDENTIFIER: return "identifier";
        case TokenType::INT_LITERAL: return "int_literal";
        case TokenType::FLOAT_LITERAL: return "float_literal";
        case TokenType::CHAR_LITERAL: return "char_literal";
        case TokenType::TRUE: return "true";
        case TokenType::FALSE: return "false";
        case TokenType::NULL_PTR: return "nullptr"; // Or "null" based on your language

        // --- Single Character Punctuation ---
        case TokenType::LEFT_PAREN: return "(";
        case TokenType::RIGHT_PAREN: return ")";
        case TokenType::LEFT_BRACE: return "{";
        case TokenType::RIGHT_BRACE: return "}";
        case TokenType::LEFT_BRACKET: return "[";
        case TokenType::RIGHT_BRACKET: return "]";
        case TokenType::COMMA: return ",";
        case TokenType::SEMICOLON: return ";";

        // --- Operators ---
        case TokenType::ASSIGN: return "=";
        case TokenType::EQ: return "==";
        case TokenType::NOT: return "!";
        case TokenType::NE: return "!=";
        case TokenType::GREATER: return ">";
        case TokenType::GE: return ">=";
        case TokenType::LESS: return "<";
        case TokenType::LE: return "<=";
        case TokenType::PLUS: return "+";
        case TokenType::PLUS_EQ: return "+=";
        case TokenType::MINUS: return "-";
        case TokenType::MINUS_EQ: return "-=";
        case TokenType::STAR: return "*";
        case TokenType::STAR_EQ: return "*=";
        case TokenType::SLASH: return "/";
        case TokenType::SLASH_EQ: return "/=";
        case TokenType::MOD: return "%";
        case TokenType::MOD_EQ: return "%=";
        case TokenType::OR: return "||";
        case TokenType::AND: return "&&";
        case TokenType::ADDRESS: return "&";

        // --- Meta ---
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::ERROR: return "error";

        default: return "UNKNOWN";
    }
}

bool isValued(TokenType type) {
    return
        type == TokenType::ERROR ||
        type == TokenType::IDENTIFIER ||
        type == TokenType::INT_LITERAL ||
        type == TokenType::FLOAT_LITERAL ||
        type == TokenType::CHAR_LITERAL;
}

void PrintTokens(const vector<Token>& tokens) {
    for(const auto& it : tokens) {
        cout << it.toString() << endl;
    }
}

bool isVarDataType(TokenType type) {
    return
    type == TokenType::INT ||
    type == TokenType::CHAR ||
    type == TokenType::BOOL ||
    type == TokenType::FLOAT;
}

bool isDataType(TokenType type) {
    return isVarDataType(type) || type == TokenType::VOID;
}

bool isLiteral(TokenType type) {
    return
        type == TokenType::INT_LITERAL ||
        type == TokenType::TRUE || type == TokenType::FALSE ||
        type == TokenType::FLOAT_LITERAL ||
        type == TokenType::CHAR_LITERAL ||
        type == TokenType::NULL_PTR;
}

bool isAssignOp(TokenType type) {
    return
        type == TokenType::ASSIGN ||
        type == TokenType::PLUS_EQ ||
        type == TokenType::MINUS_EQ ||
        type == TokenType::STAR_EQ ||
        type == TokenType::SLASH_EQ ||
        type == TokenType::MOD_EQ;
}