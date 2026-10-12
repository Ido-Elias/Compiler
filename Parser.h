#ifndef PARSER_H
#define PARSER_H

#include "AST_Node.h"


class Parser {
public:
    Parser() = default;
    ~Parser() = default;

    pair<unique_ptr<AST_Node>, bool> ParseTokens(const vector<Token>& tokens);

private:
    size_t pos;
    vector<Token> tokens;
    bool success;

    void prepareParse(const vector<Token>& tokens);

    //utility
    const Token& peek();
    const Token& advance();
    bool match(TokenType type);
    bool consumeIf(TokenType type);
    const Token& expect(TokenType expected, const string& ErrorMsg);


    //meta stuff
    unique_ptr<AST_Node> parseProgram();
    unique_ptr<AST_Node> parseGlobalStmnt();
    unique_ptr<AST_Node> parseLocalStmnt();
    unique_ptr<AST_Node> parseBlock();

    //statements

    unique_ptr<AST_Node> parseIfStmnt();
    unique_ptr<AST_Node> parseIf();

    unique_ptr<AST_Node> parseWhileStmnt();
    unique_ptr<AST_Node> parseReturnStmnt();

    unique_ptr<AST_Node> parseAssignStmnt(unique_ptr<AST_Node> lvalue);

    unique_ptr<AST_Node> parseLocalDeclarations();
    pair<DataType, size_t> parseDataType();
    unique_ptr<AST_Node> parseVarDeclInit(DataType type, size_t ptrCount, const Token& id);
    unique_ptr<AST_Node> parseArrayDecl(DataType type, size_t ptrCount, const Token& id);

    unique_ptr<AST_Node> parseGlobalDeclarations();
    unique_ptr<AST_Node> parseFunc_Decl_Def(DataType type, size_t ptrCount, const Token& id);
    unique_ptr<AST_Node> parseFunc_Decl_Def_Rec(DataType type, size_t ptrCount, const Token& id, unique_ptr<AST_Node> paramList);
    void AddToParamList(unique_ptr<AST_Node>& paramList);
    unique_ptr<AST_Node> parseParam();


    //math stuff
    unique_ptr<AST_Node> parseExpr();
    unique_ptr<AST_Node> parseLogicalAnd();
    unique_ptr<AST_Node> parseEquality();
    unique_ptr<AST_Node> parseRelational();
    unique_ptr<AST_Node> parseTerm();
    unique_ptr<AST_Node> parseFactor();
    unique_ptr<AST_Node> parseUnary();
    unique_ptr<AST_Node> parsePrimary();

    unique_ptr<AST_Node> parseCast();

    unique_ptr<AST_Node> decide_FuncCall_Identifier_ArrayAccess();
    unique_ptr<AST_Node> parseFuncCall();
    unique_ptr<AST_Node> parseArrayAccess();

    unique_ptr<AST_Node> parseLiteral();

    void ReportError(size_t line, size_t column, const string& ErrorMsg);
    void synchronize(bool skipCause, const vector<TokenType>& safeTokens);

    inline static const vector<TokenType> GlobalSync = {
        //only data types can sync (only decl in top level)
        TokenType::INT,
        TokenType::CHAR,
        TokenType::BOOL,
        TokenType::FLOAT,
        TokenType::VOID
    };

    inline static const vector<TokenType> LocalSync = {
        //data types
        TokenType::INT,
        TokenType::CHAR,
        TokenType::BOOL,
        TokenType::FLOAT,

        //keywords
        TokenType::WHILE,
        TokenType::RETURN,
        TokenType::IF,

        //scope
        TokenType::LEFT_BRACE,
        TokenType::RIGHT_BRACE
    };

    inline static const vector<TokenType> ParamSync = {
        //if semicolon just leave
        TokenType::SEMICOLON,

        //retry more params
        TokenType::COMMA,

        //close header and expect ';' or '}'
        TokenType::RIGHT_PAREN,

        //try func body itself
        TokenType::LEFT_BRACE,

        //try to resync in global scale/more parameters (if successful)
        TokenType::INT,
        TokenType::CHAR,
        TokenType::BOOL,
        TokenType::FLOAT,
        TokenType::VOID
    };
};

class SyntaxError : public runtime_error {
public:
    bool skipCause;
    SyntaxError(const string& msg, bool skipCause = true)
        : runtime_error(msg), skipCause(skipCause) {}

};

#endif //PARSER_H
