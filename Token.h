#ifndef TOKEN_H
#define TOKEN_H

using namespace std;
#include <string>
#include <vector>

enum class  TokenType {
    //types
    INT, FLOAT, CHAR, BOOL, VOID,

    //id
    IDENTIFIER,

    //literals
    INT_LITERAL, FLOAT_LITERAL, CHAR_LITERAL, TRUE, FALSE, NULL_PTR,

    //single character
    LEFT_PAREN, // (
    RIGHT_PAREN, // )
    LEFT_BRACE, // {
    RIGHT_BRACE, // }
    LEFT_BRACKET, // [
    RIGHT_BRACKET, // ]
    COMMA, // ,
    SEMICOLON, // ;

    //operators
    ASSIGN, // =
    EQ, // ==
    NOT, // !
    NE, // !=
    GREATER, // >
    GE, // >=
    LESS, // <
    LE, // <=
    PLUS, // +
    PLUS_EQ, // +=
    MINUS, // -
    MINUS_EQ, // -=
    STAR, // *
    STAR_EQ, // *=
    SLASH, // /
    SLASH_EQ, // /=
    MOD, // %
    MOD_EQ, // %=
    OR, // ||
    AND, //&&
    ADDRESS, //& (&x)

    //other keywords
    IF,
    ELSE,
    WHILE,
    RETURN,


    //meta
    END_OF_FILE,
    ERROR,
    NONE
};


bool isValued(TokenType);
string tokenTypeToString(TokenType type);
bool isVarDataType(TokenType type);
bool isDataType(TokenType type);
bool isLiteral(TokenType type);
bool isAssignOp(TokenType type);

struct Token {
    TokenType type;
    string lexeme;
    int line;
    int column;

    Token(TokenType type = TokenType::NONE, const string& lexeme = "", int line = -1, int column = -1)
        : type(type), lexeme(lexeme), line(line), column(column) {}

    string toString() const {
        string text = lexeme.empty() ? tokenTypeToString(type) : lexeme;

        return "Line " + std::to_string(line) +
               ", Col " + std::to_string(column) +
               " | Type: " + tokenTypeToString(type) +
               (isValued(type)? " | Value: " + text: "");
    }
};

void PrintTokens(const vector<Token>& tokens);

#endif //TOKEN_H

