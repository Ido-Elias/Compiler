#include "DataType.h"

DataType TokenTypeToDataType(TokenType type) {
    switch(type) {
        case TokenType::INT: return DataType::INT;
        case TokenType::BOOL: return DataType::BOOL;
        case TokenType::CHAR: return DataType::CHAR;
        case TokenType::FLOAT: return DataType::FLOAT;
        case TokenType::VOID: return DataType::VOID;
        default: return DataType::NONE;
    }
}

bool isNumerical(DataType type) {
    return type == DataType::INT || type == DataType::FLOAT;
}
