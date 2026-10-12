#include <iostream>
#include <fstream>
#include <sstream>

#include "Lexer.h"
#include "Token.h"
#include "Parser.h"

int main(int argc, char* argv[]) {
    ifstream file(argv[1]);
    if(!file) return 0;

    Lexer lexer;

    stringstream buffer;
    buffer << file.rdbuf();
    string test_source = buffer.str();

    // Run the lexer
    auto [tokens, success] = lexer.tokenizeAll(test_source);

    // Print all gathered tokens
    PrintTokens(tokens);

    // Final Status
    if (!success) {
        cout << "\n[RESULT] Lexing failed. See error tokens above." << endl;
    } else {
        cout << "\n[RESULT] Lexing completed successfully!" << endl;
    }

    Parser parser;
    cout << parser.ParseTokens(tokens).first->toString();

    return 0;
}
