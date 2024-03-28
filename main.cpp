#include <iostream>
#include "parsing/tokenize.h"
#include "parsing/parse.h"

int main() {

    std::string source = R"V0G0N(
    int a = 4;
    int main(){
        int x = 0;
        int y = 5;
        int z = x + y;
        a += z;

        return 0;
    }
    )V0G0N";

    // & + 5 * 4 - 7 4

    std::vector<Token> tokens = tokenize(source);

    std::vector<Token*> tokenPtrs;
    tokenPtrs.reserve(tokens.size());
    for (Token& token : tokens) {
        tokenPtrs.push_back(&token);
    }
    // print all tokens
    for (Token* token : tokenPtrs) {
        std::cout << token->toString() << std::endl;
    }

    printf("\n\n");

    Scope scope = Scope(nullptr);
    TokenIterator iter(tokenPtrs);

    std::vector<Token*> parsed = parse(iter, &scope);
    for (Token* token : parsed) {
        std::cout << token->toString() << std::endl;
    }

    return 0;
}
