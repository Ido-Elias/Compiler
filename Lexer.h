#ifndef LEXER_H
#define LEXER_H

#include <unordered_map>
#include <vector>
#include "Token.h"

class Lexer {
public:
    pair<vector<Token>, bool> tokenizeAll(const string& input_src);
    Lexer() = default;
    ~Lexer() = default;

private:
    string src;
    size_t pos;
    size_t column, line;
    bool success;

    unordered_map<std::string, TokenType> keywords = {
        {"int",    TokenType::INT},
        {"float",  TokenType::FLOAT},
        {"char",   TokenType::CHAR},
        {"bool",   TokenType::BOOL},
        {"void",   TokenType::VOID},
        {"if",     TokenType::IF},
        {"else",   TokenType::ELSE},
        {"while",  TokenType::WHILE},
        {"return", TokenType::RETURN},
        {"true",   TokenType::TRUE},
        {"false",  TokenType::FALSE},
        {"nullptr",TokenType::NULL_PTR}
    };


    void prepare_tokenizeAll(const string& input_src);

    void advance();
    char peek();
    bool match(char expected);

    void skip_whitespace();
    void handle_single_line_comment();
    void handle_multi_line_comment();
    void handle_identifier_and_keyword(vector<Token>& result);
    void handle_char(vector<Token>& result);
    void handle_symbol_or_unknown(vector<Token>& result);
    void handle_number(vector<Token>& result);
    void ReportError(size_t line, size_t column, const string& ErrorMsg);
};
#endif //LEXER_H
