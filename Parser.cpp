#include "Parser.h"

#include <algorithm>
#include <complex>
#include <iostream>

#define curr tokens[pos]

pair<unique_ptr<AST_Node>, bool> Parser::ParseTokens(const vector<Token> &tokens) {
    prepareParse(tokens);

    return pair{parseProgram(), success};
}

void Parser::prepareParse(const vector<Token>& tokens) {
    this->tokens = tokens;
    success = true;
    pos = 0;
}

void Parser::ReportError(size_t line, size_t column, const string& ErrorMsg) {
    cout << "Error";
    if(column != -1) {
        cout << " at line " << line <<  " | " << column;
    }
    cout << ": ";
    cout << ErrorMsg << endl;
}

bool contains(const vector<TokenType>& types, TokenType type) {
    for(auto it : types) {
        if(it == type) return true;
    }
    return false;
}

void Parser::synchronize(bool skipCause, const vector<TokenType>& safeTokens) {
    if(skipCause && !contains(safeTokens, curr.type)) advance(); //skipping first token (usually the case)

    while(!match(TokenType::END_OF_FILE)) {
        if(contains(safeTokens, curr.type)) {
            return;
        }

        if(match(TokenType::SEMICOLON)) { //global rule, semicolon lets the parser continue safely
            advance(); //we need to know its a ';' for params
            return;
        }

        advance();
    }
}

const Token& Parser::peek() {
    return tokens.at(pos + 1);
}

const Token& Parser::advance() {
    if(tokens[pos].type != TokenType::END_OF_FILE) {
        return tokens[pos++];
    }
    return tokens[pos]; //will be eof
}

bool Parser::match(TokenType type) {
    return tokens[pos].type == type;
}

bool Parser::consumeIf(TokenType type) {
    if(match(type)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::expect(TokenType expected, const string& ErrorMsg) {
    if(tokens[pos].type != expected) {
        throw SyntaxError(ErrorMsg + ", found '" + tokenTypeToString(curr.type) + "'", false);
    }
    return advance();
}

// META & STMNTS //
unique_ptr<AST_Node> Parser::parseProgram() {
    unique_ptr<AST_Node> program = make_unique<AST_Node>(AST_NodeType::PROGRAM);

    while(!match(TokenType::END_OF_FILE)) {
        try {
            program->children.emplace_back(parseGlobalStmnt());
        }
        catch(SyntaxError& e) {
            success = false;
            ReportError(curr.line, curr.column, e.what());
            synchronize(e.skipCause, GlobalSync);
        }
    }

    return program;
}

unique_ptr<AST_Node> Parser::parseGlobalStmnt() {
    if(isDataType(curr.type)) {
        return parseGlobalDeclarations();
    }

    throw SyntaxError("Expected top-level statement, found '" + tokenTypeToString(curr.type) + "'");
}

unique_ptr<AST_Node> Parser::parseBlock() {
    unique_ptr<AST_Node> block = make_unique<AST_Node>(AST_NodeType::BLOCK);

    advance(); //skip {

    while(!match(TokenType::END_OF_FILE) && !match(TokenType::RIGHT_BRACE)) {
        try {
            block->children.emplace_back(parseLocalStmnt());
        }
        catch(SyntaxError& e) {
            success = false;
            ReportError(curr.line, curr.column, e.what());
            synchronize(e.skipCause, LocalSync);
        }
    }

    expect(TokenType::RIGHT_BRACE, "Expected '}'");

    return block;
}

unique_ptr<AST_Node> Parser::parseLocalStmnt() {
    if(match(TokenType::LEFT_BRACE)) {
        return parseBlock();
    }
    if(isVarDataType(curr.type)) {
        return parseLocalDeclarations();
    }
    if(match(TokenType::IF)) {
        return parseIfStmnt();
    }
    if(match(TokenType::WHILE)) {
        return parseWhileStmnt();
    }
    if(match(TokenType::RETURN)) {
        return parseReturnStmnt();
    }


    unique_ptr<AST_Node> expr = parseExpr(); //failure here will take us out

    if(match(TokenType::SEMICOLON)) { //just make the stmnt and end it
        unique_ptr<AST_Node> expression_statement = make_unique<AST_Node>(AST_NodeType::EXPR_STMNT);
        expression_statement->children.emplace_back(move(expr));
        return expression_statement;
    }
    if(isAssignOp(curr.type)) {
        return parseAssignStmnt(move(expr));
    }

    throw SyntaxError("Expected ';' after expression");
}

// -- ACTUAL STATEMENT IMPLEMENTATION -- //
unique_ptr<AST_Node> Parser::parseAssignStmnt(unique_ptr<AST_Node> lvalue) {
    bool isDeref = lvalue->mainToken.type == TokenType::STAR && lvalue->type == AST_NodeType::UNARY_EXPR;
    bool isVar = lvalue->type == AST_NodeType::VAR;
    bool isArrayAccess = lvalue->type == AST_NodeType::ARRAY_ACCESS;

    if(!isDeref && !isVar && !isArrayAccess) {
        throw SyntaxError("Invalid l-value");
    }

    const Token& op = advance();

    unique_ptr<AST_Node> rhs = parseExpr();

    expect(TokenType::SEMICOLON, "Expected ';'");

    unique_ptr<AST_Node> assign_statement = make_unique<AST_Node>(AST_NodeType::ASSIGN_STMNT, op);
    assign_statement->children.emplace_back(move(lvalue));
    assign_statement->children.emplace_back(move(rhs));

    return assign_statement;
}

pair<DataType, size_t> Parser::parseDataType() {
    DataType data_type = TokenTypeToDataType(advance().type); //advance and also check type

    size_t ptrCount = 0;
    while(match(TokenType::STAR)) {
        ptrCount++;
        advance();
    }

    return pair{data_type, ptrCount};
}

unique_ptr<AST_Node> Parser::parseLocalDeclarations() { //for arrays - decl, for var - decl/init
    pair<DataType, size_t> type_info = parseDataType();

    const Token& id = expect(TokenType::IDENTIFIER, "Expected identifier");

    if(match(TokenType::LEFT_BRACKET)) {
        return parseArrayDecl(type_info.first, type_info.second, id);
    }

    return parseVarDeclInit(type_info.first, type_info.second, id);
}

unique_ptr<AST_Node> Parser::parseArrayDecl(DataType type, size_t ptrCount, const Token &id){
    advance();
    const Token& size = expect(TokenType::INT_LITERAL, "Only int literal allowed in array declaration");

    unique_ptr<AST_Node> size_node = make_unique<AST_Node>(AST_NodeType::LITERAL, size);

    expect(TokenType::RIGHT_BRACKET, "Expected ']'");
    expect(TokenType::SEMICOLON, "Expected ';'");

    unique_ptr<AST_Node> array_decl = make_unique<AST_Node>(AST_NodeType::ARRAY_DECL, id, type, ptrCount);
    array_decl->children.emplace_back(move(size_node));

    return array_decl;
}

unique_ptr<AST_Node> Parser::parseVarDeclInit(DataType type, size_t ptrCount, const Token &id) {
    unique_ptr<AST_Node> variable_decl_init = make_unique<AST_Node>(AST_NodeType::VAR_DECL_INIT, id, type, ptrCount);

    if(consumeIf(TokenType::ASSIGN)) {
        unique_ptr<AST_Node> expr = parseExpr();
        variable_decl_init->children.emplace_back(move(expr));
    }

    expect(TokenType::SEMICOLON, "Expected ';'");
    return variable_decl_init;
}

unique_ptr<AST_Node> Parser::parseGlobalDeclarations() {
    pair<DataType, size_t> type_info = parseDataType();

    const Token& id = expect(TokenType::IDENTIFIER, "Expected identifier");

    if(consumeIf(TokenType::LEFT_PAREN)) { //check and skip '('
        return parseFunc_Decl_Def(type_info.first, type_info.second, id);
    }

    if(match(TokenType::LEFT_BRACKET)) { //check and skip '['
        return parseArrayDecl(type_info.first, type_info.second, id);
    }

    return parseVarDeclInit(type_info.first, type_info.second, id);
}

unique_ptr<AST_Node> Parser::parseFunc_Decl_Def(DataType type, size_t ptrCount, const Token& id) {
    unique_ptr<AST_Node> empty_list = make_unique<AST_Node>(AST_NodeType::PARAM_LIST);
    return parseFunc_Decl_Def_Rec(type, ptrCount, id, move(empty_list));
}

unique_ptr<AST_Node> Parser::parseFunc_Decl_Def_Rec(DataType type, size_t ptrCount, const Token& id, unique_ptr<AST_Node> paramList) {
    unique_ptr<AST_Node> decl_def = make_unique<AST_Node>(AST_NodeType::FUNC_DECL, id, type, ptrCount);

    try {
        AddToParamList(paramList);
        expect(TokenType::RIGHT_PAREN, "Expected ')'");
    }
    catch(SyntaxError& e) {
        success = false;
        ReportError(curr.line, curr.column, e.what());
        synchronize(e.skipCause, ParamSync);

        //preparations after syncing
        if(isVarDataType(curr.type) || consumeIf(TokenType::COMMA)) { //give the user a "second chance" to parse, we lose parsed params but who cares, its not running anyways
            return parseFunc_Decl_Def_Rec(type, ptrCount, id, move(paramList));
        }
        consumeIf(TokenType::RIGHT_PAREN); //if its a ')' lets keep going!
    }

    decl_def->children.emplace_back(move(paramList)); //add the 'finished' list

    if(consumeIf(TokenType::SEMICOLON)) { //semicolon = run away
        return decl_def;
    }
    if(match(TokenType::LEFT_BRACE)) { //here we can check for  '{' for block parsing
        decl_def->type = AST_NodeType::FUNC_DEF;

        unique_ptr<AST_Node> block = parseBlock();
        decl_def->children.emplace_back(move(block));
    }
    else {
        expect(TokenType::SEMICOLON, "Expected ';'");
    }

    return decl_def;
}

void Parser::AddToParamList(unique_ptr<AST_Node>& paramList) { //actually adding to the list (a ref &)
    if(match(TokenType::RIGHT_PAREN)) return;

    do {
        unique_ptr<AST_Node> param = parseParam();
        paramList->children.emplace_back(move(param));
        if(!match(TokenType::RIGHT_PAREN) && !match(TokenType::COMMA)) throw SyntaxError("Expected ',' or ')'");
    }
    while(consumeIf(TokenType::COMMA));
}

unique_ptr<AST_Node> Parser::parseParam() {
    if(!isVarDataType(curr.type)) {
        throw SyntaxError("Expected declaration of parameter, found '" + tokenTypeToString(curr.type) + "'");
    }

    pair<DataType, size_t> type_info = parseDataType();

    const Token& id = expect(TokenType::IDENTIFIER, "Expected identifier");

    return make_unique<AST_Node>(AST_NodeType::PARAM, id, type_info.first, type_info.second);
}

unique_ptr<AST_Node> Parser::parseReturnStmnt() {
     //we skip the 'return' inside the decl!
    unique_ptr<AST_Node> return_statement = make_unique<AST_Node>(AST_NodeType::RETURN_STMNT, advance());

    if(!match(TokenType::SEMICOLON)) {
        unique_ptr<AST_Node> expr = parseExpr();
        return_statement->children.emplace_back(move(expr));
    }

    expect(TokenType::SEMICOLON, "Expected ';'");

    return return_statement;
}

unique_ptr<AST_Node> Parser::parseWhileStmnt() {
    //we skip the 'while' inside the decl!
    unique_ptr<AST_Node> while_statement = make_unique<AST_Node>(AST_NodeType::WHILE_STMNT, advance());

    expect(TokenType::LEFT_PAREN, "Expected '('");
    unique_ptr<AST_Node> cond = parseExpr();
    expect(TokenType::RIGHT_PAREN, "Expected ')'");

    unique_ptr<AST_Node> statement = parseLocalStmnt(); //could be { ... } or even could be int x;

    while_statement->children.emplace_back(move(cond));
    while_statement->children.emplace_back(move(statement));

    return while_statement;
}

unique_ptr<AST_Node> Parser::parseIfStmnt() {
    unique_ptr<AST_Node> if_statement = make_unique<AST_Node>(AST_NodeType::IF_STMNT, curr);

    unique_ptr<AST_Node> if_part = parseIf();

    if_statement->children.emplace_back(move(if_part));

    if(!match(TokenType::ELSE)) {
        return if_statement;
    }

    //skip else keyword inside the make unique
    unique_ptr<AST_Node> else_part = make_unique<AST_Node>(AST_NodeType::ELSE, advance());

    if(match(TokenType::IF)) {
        unique_ptr<AST_Node> nested_if = parseIfStmnt();
        else_part->children.emplace_back(move(nested_if));
    }
    else {
        unique_ptr<AST_Node> statement = parseLocalStmnt();
        else_part->children.emplace_back(move(statement));
    }

    if_statement->children.emplace_back(move(else_part));

    return if_statement;
}

unique_ptr<AST_Node> Parser::parseIf() {
    //skip if keyword inside the make unique
    unique_ptr<AST_Node> if_part = make_unique<AST_Node>(AST_NodeType::IF, advance());

    expect(TokenType::LEFT_PAREN, "Expected '('");

    unique_ptr<AST_Node> cond = parseExpr();

    expect(TokenType::RIGHT_PAREN, "Expected ')'");

    unique_ptr<AST_Node> statement = parseLocalStmnt();

    if_part->children.emplace_back(move(cond));
    if_part->children.emplace_back(move(statement));

    return if_part;
}



// -- MATH STUFF -- //
unique_ptr<AST_Node> Parser::parseExpr() {
    unique_ptr<AST_Node> lhs = parseLogicalAnd();

    while(match(TokenType::OR)) {
        const Token& op = advance();; //save operator token

        unique_ptr<AST_Node> rhs = parseLogicalAnd();

        unique_ptr<AST_Node> expr = make_unique<AST_Node>(AST_NodeType::BINARY_EXPR, op);
        expr->children.emplace_back(move(lhs));
        expr->children.emplace_back(move(rhs));


        lhs = move(expr);
    }

    return lhs;
}

unique_ptr<AST_Node> Parser::parseLogicalAnd() {
    unique_ptr<AST_Node> lhs = parseEquality();

    while(match(TokenType::AND)) {
        const Token& op = advance();; //save operator token

        unique_ptr<AST_Node> rhs = parseEquality();

        unique_ptr<AST_Node> expr = make_unique<AST_Node>(AST_NodeType::BINARY_EXPR, op);
        expr->children.emplace_back(move(lhs));
        expr->children.emplace_back(move(rhs));


        lhs = move(expr);
    }

    return lhs;
}

unique_ptr<AST_Node> Parser::parseEquality() {
    unique_ptr<AST_Node> lhs = parseRelational();

    while(match(TokenType::EQ) || match(TokenType::NE)) {
        const Token& op = advance();; //save operator token

        unique_ptr<AST_Node> rhs = parseRelational();

        unique_ptr<AST_Node> expr = make_unique<AST_Node>(AST_NodeType::BINARY_EXPR, op);
        expr->children.emplace_back(move(lhs));
        expr->children.emplace_back(move(rhs));


        lhs = move(expr);
    }

    return lhs;
}

unique_ptr<AST_Node> Parser::parseRelational() {
    unique_ptr<AST_Node> lhs = parseTerm();

    while(match(TokenType::GREATER) || match(TokenType::GE) || match(TokenType::LESS) || match(TokenType::LE)) {
        const Token& op = advance(); //save operator token

        unique_ptr<AST_Node> rhs = parseTerm();

        unique_ptr<AST_Node> expr = make_unique<AST_Node>(AST_NodeType::BINARY_EXPR, op);
        expr->children.emplace_back(move(lhs));
        expr->children.emplace_back(move(rhs));


        lhs = move(expr);
    }

    return lhs;
}

unique_ptr<AST_Node> Parser::parseTerm() {
    unique_ptr<AST_Node> lhs = parseFactor();

    while(match(TokenType::PLUS) || match(TokenType::MINUS)) {
        const Token& op = advance();; //save operator token

        unique_ptr<AST_Node> rhs = parseFactor();

        unique_ptr<AST_Node> expr = make_unique<AST_Node>(AST_NodeType::BINARY_EXPR, op);
        expr->children.emplace_back(move(lhs));
        expr->children.emplace_back(move(rhs));


        lhs = move(expr);
    }

    return lhs;
}

unique_ptr<AST_Node> Parser::parseFactor() {
    unique_ptr<AST_Node> lhs = parseUnary();

    while(match(TokenType::STAR) || match(TokenType::SLASH) || match(TokenType::MOD)) {
        const Token& op = advance();; //save operator token

        unique_ptr<AST_Node> rhs = parseUnary();

        unique_ptr<AST_Node> expr = make_unique<AST_Node>(AST_NodeType::BINARY_EXPR, op);
        expr->children.emplace_back(move(lhs));
        expr->children.emplace_back(move(rhs));


        lhs = move(expr);
    }

    return lhs;
}

unique_ptr<AST_Node> Parser::parseUnary() {
    unique_ptr<AST_Node> unary;

    //basic unary operators
    if(match(TokenType::MINUS) || match(TokenType::NOT) || match(TokenType::ADDRESS) || match(TokenType::STAR)) {
        const Token& op = advance(); //save operator token

        unary = parseUnary(); //go deeper to allow chaining

        unique_ptr<AST_Node> expr = make_unique<AST_Node>(AST_NodeType::UNARY_EXPR, op);
        expr->children.emplace_back(move(unary));

        unary = move(expr);
        return unary;
    }

    // a cast
    if(match(TokenType::LEFT_PAREN) && isVarDataType(peek().type)) {
        unique_ptr<AST_Node> cast = parseCast();

        unary = parseUnary(); //go deeper to allow chaining

        cast->children.emplace_back(move(unary));

        return cast;
    }

    //base case
    return parsePrimary();
}

unique_ptr<AST_Node> Parser::parseCast() {
    advance(); //skip '('

    const Token& type_keyword = curr;

    pair<DataType, size_t> type_info = parseDataType();

    expect(TokenType::RIGHT_PAREN, "Expected ')'");

    return make_unique<AST_Node>(AST_NodeType::CAST, type_keyword , type_info.first, type_info.second);
}

unique_ptr<AST_Node> Parser::parsePrimary() {
    if(match(TokenType::LEFT_PAREN)) { // ( <expr> )
        advance();
        unique_ptr<AST_Node> expr = parseExpr();
        expect(TokenType::RIGHT_PAREN, "Expected ')'");
        return expr;
    }
    if(match(TokenType::IDENTIFIER)) { //func_call/id/array_access
        return decide_FuncCall_Identifier_ArrayAccess();
    }
    if(isLiteral(curr.type)) { //literal
        return parseLiteral();
    }

    throw SyntaxError("Expected expression, found '" + tokenTypeToString(curr.type) + "'");
}

unique_ptr<AST_Node> Parser::decide_FuncCall_Identifier_ArrayAccess() {
    if(peek().type == TokenType::LEFT_PAREN) {
        return parseFuncCall();
    }
    if(peek().type == TokenType::LEFT_BRACKET) {
        return parseArrayAccess();
    }
    return make_unique<AST_Node>(AST_NodeType::VAR, advance()); //id
}

unique_ptr<AST_Node> Parser::parseFuncCall() {
    unique_ptr<AST_Node> func_Call = make_unique<AST_Node>(AST_NodeType::FUNC_CALL, curr);
    advance(); //we know for sure this is 'id'
    advance(); //we know for sure this is '('

    if(match(TokenType::RIGHT_PAREN)) { //for foo()
        advance();
        return func_Call;
    }

    unique_ptr<AST_Node> arg = parseExpr();
    func_Call->children.emplace_back(move(arg));

    while(match(TokenType::COMMA)) {
        advance();
        arg = parseExpr();
        func_Call->children.emplace_back(move(arg));
    }

    expect(TokenType::RIGHT_PAREN, "Expected ')'");

    return func_Call;
}

unique_ptr<AST_Node> Parser::parseArrayAccess() {
    const Token& id = advance();
    advance(); //we know for sure this is '['

    unique_ptr<AST_Node> expr = parseExpr();

    expect(TokenType::RIGHT_BRACKET, "Expected ']'");

    unique_ptr<AST_Node> array_access = make_unique<AST_Node>(AST_NodeType::ARRAY_ACCESS, id);
    array_access->children.emplace_back(move(expr));
    return array_access;
}

unique_ptr<AST_Node> Parser::parseLiteral() {
    DataType data_type;

    switch(curr.type) {
        case TokenType::INT_LITERAL:
            data_type = DataType::INT;
        break;

        case TokenType::FLOAT_LITERAL:
            data_type = DataType::FLOAT;
        break;

        case TokenType::CHAR_LITERAL:
            data_type = DataType::CHAR;
        break;

        case TokenType::TRUE:
        case TokenType::FALSE:
            data_type = DataType::BOOL;
        break;

        case TokenType::NULL_PTR:
        default:
            data_type = DataType::NONE;
        break;
    }

    return make_unique<AST_Node>(AST_NodeType::LITERAL, advance(), data_type);
}