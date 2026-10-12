#include "Lexer.h"

#include <iostream>

#define curr src[pos]

bool isWhitespace(char c) {
    return c == ' ' || c == '\t' | c == '\n';
}

void Lexer::prepare_tokenizeAll(const string& input_src) {
    this->src = input_src;
    this->src += '\0'; //add this "ending" for easy lexing
    pos = 0;
    line = 1;
    column = 1;
    success = true;
}

pair<vector<Token>, bool> Lexer::tokenizeAll(const string& input_src) {
    prepare_tokenizeAll(input_src);
    vector<Token> result;

    while(curr != '\0') {
        if(isWhitespace(curr)) {
            skip_whitespace();
        }
        else if(curr == '/' && peek() == '*') {
            handle_multi_line_comment();
        }
        else if(curr == '/' && peek() == '/') {
            handle_single_line_comment();
        }
        else if(isalpha(curr) || curr == '_') {
            handle_identifier_and_keyword(result);
        }
        else if(isdigit(curr) || curr == '.') {
            handle_number(result);
        }
        else if(curr == '\'') {
            handle_char(result);
        }
        else {
            handle_symbol_or_unknown(result);
        }
    }
    result.emplace_back(TokenType::END_OF_FILE, "END_OF_FILE", line, column);
    return pair{result, success};
}

void Lexer::handle_char(vector<Token> &result) {
    TokenType type = TokenType::CHAR_LITERAL;

    size_t starting_column = column;
    size_t starting_line = line;

    string char_literal = "";
    char_literal += curr; // Add opening '
    advance();

    // --- Case 1: Empty ('') ---
    if(curr == '\'') {
        char_literal += curr;
        ReportError(starting_line, starting_column, "Empty character literal");
        result.emplace_back(TokenType::ERROR, char_literal, starting_line, starting_column);
        advance(); // Move past second '
        return;
    }

    // --- Handle Escape Sequence ---
    if(curr == '\\') {
        char_literal += curr;
        advance();

        if(curr != 'n' && curr != 't' && curr != '0' && curr != '\'' && curr != '\\') {
            string invalid = (curr == '\n') ? "newline" : (curr == '\t') ? "tab" : string(1,curr);
            string ErrorMsg = "Invalid escape sequence '\\" + invalid + R"('. Supported: \n, \t, \0, \', \\)";
            ReportError(line, column, ErrorMsg); // Point to the specific bad character
            type = TokenType::ERROR;
        }
    }

    // Add the character (or the escaped character)
    char_literal += curr;
    advance();

    // --- Case 2: Unclosed ('a) ---
    if(curr != '\'') {
        ReportError(line, column, "Missing closing single quote for character literal");
        // We push what we have so far as an ERROR token
        result.emplace_back(TokenType::ERROR, char_literal, starting_line, starting_column);
        return;
    }

    // --- Success Case ---
    char_literal += curr; // Add closing '
    advance();

    result.emplace_back(type, char_literal, starting_line, starting_column);
}

void Lexer::handle_symbol_or_unknown(vector<Token>& result) {
    size_t starting_column = column;
    size_t starting_line = line;
    char c = curr;
    string lexeme;

    advance(); // Move past the first character

    TokenType type = TokenType::ERROR; // Default to error

    switch (c) {
        // --- 1. Single Character Punctuation ---
        case '(': type = TokenType::LEFT_PAREN; break;
        case ')': type = TokenType::RIGHT_PAREN; break;
        case '{': type = TokenType::LEFT_BRACE; break;
        case '}': type = TokenType::RIGHT_BRACE; break;
        case '[': type = TokenType::LEFT_BRACKET; break;
        case ']': type = TokenType::RIGHT_BRACKET; break;
        case ',': type = TokenType::COMMA; break;
        case ';': type = TokenType::SEMICOLON; break;
        case '%': type = TokenType::MOD; break;
        case '*': type = TokenType::STAR; break;

        // --- 2. Operators with an '=' variant ---
        case '/': type = match('=') ? TokenType::SLASH_EQ : TokenType::SLASH; break;
        case '=': type = match('=') ? TokenType::EQ : TokenType::ASSIGN; break;
        case '+': type = match('=') ? TokenType::PLUS_EQ : TokenType::PLUS; break;
        case '-': type = match('=') ? TokenType::MINUS_EQ : TokenType::MINUS; break;
        case '!': type = match('=') ? TokenType::NE : TokenType::NOT; break;
        case '<': type = match('=') ? TokenType::LE : TokenType::LESS; break;
        case '>': type = match('=') ? TokenType::GE : TokenType::GREATER; break;

        // --- 3. Strict Two-Character Logic Operators ---
        case '&':
            if (match('&')) {
                type = TokenType::AND;
            } else {
                type = TokenType::ADDRESS;
            }
            break;
        case '|':
            if (match('|')) {
                type = TokenType::OR;
            } else {
                ReportError(starting_line, starting_column, "Unexpected symbol '|'. Did you mean '||'?");
                lexeme = "|";
            }
            break;

        default:
            ReportError(starting_line, starting_column, "Unrecognized symbol: '" + string(1, c) + "'");
            lexeme = string(1, c);
        break;
    }

    // Assign lexeme for valid tokens if not already handled
    if (type != TokenType::ERROR) {
        lexeme = tokenTypeToString(type);
    }

    result.emplace_back(type, lexeme, starting_line, starting_column);
}

void Lexer::handle_identifier_and_keyword(vector<Token>& result) {
    size_t starting_column = column;

    string id_or_keyword = "";

    do {
        id_or_keyword += curr;
        advance();
    }
    while(isalnum(curr) || curr == '_');

    TokenType type = keywords.contains(id_or_keyword)?  keywords[id_or_keyword] : TokenType::IDENTIFIER;
    result.emplace_back(type, id_or_keyword, line, starting_column);
}

void Lexer::handle_number(vector<Token>& result) {
    size_t starting_line = line;
    size_t starting_column = column;
    bool dot_seen = false;

    string int_or_float = "";

    do {
        if(curr == '.') {
            if(dot_seen) {
                break;
            }
            dot_seen = true;
        }
        int_or_float += curr;
        advance();
    }
    while(isdigit(curr) || curr == '.');

    if(int_or_float == ".") { //lone dot
        string ErrorMsg = "Unexpected '.'";
        ReportError(line, column, ErrorMsg);
        result.emplace_back(TokenType::ERROR, int_or_float, starting_line, starting_column);
        return;
    }

    TokenType type = dot_seen? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL;
    result.emplace_back(type, int_or_float, starting_line, starting_column);
}

void Lexer::ReportError(size_t line, size_t column, const string& ErrorMsg) {
    success = false;

    cout << "Error";
    if(column != -1) {
        cout << " at line " << line <<  " | " << column;
    }
    cout << ": ";
    cout << ErrorMsg << endl;
}

void Lexer::skip_whitespace() {
    do {
        advance();
    }
    while(isWhitespace(curr));
}

void Lexer::handle_single_line_comment() { //just skip the whole line
    while(curr != '\0' && curr != '\n') {
        advance();
    }
}

void Lexer::handle_multi_line_comment() {
    // skip /*
    advance();
    advance();

    while(curr != '\0' && (curr != '*' || peek() != '/')) {
        advance();
    }

    if(curr ==  '\0') {
        string ErrorMsg = "Unclosed comment at end of file";
        ReportError(line, -1, ErrorMsg);
        return;
    }

    //consume */
    advance();
    advance();
}

bool Lexer::match(char expected) { //moves only if what we expect is received
    if(curr == expected) {
        advance();
        return true;
    }
    return false;
}

void Lexer::advance() {
    if(curr == '\n') {
        line++;
        column = 0;
    }

    if(curr != '\0') { //might lead to infinite loop so remember this
        pos++;
        column++;
    }
}

char Lexer::peek() {
    return pos + 1 < src.length()? src[pos + 1] : '\0';
}


