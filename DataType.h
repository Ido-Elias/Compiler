#ifndef DATATYPE_H
#define DATATYPE_H

#include "Token.h"

enum class DataType {
    INT,
    CHAR,
    BOOL,
    FLOAT,
    VOID,
    ERROR, //for error handeling
    NONE
};

DataType TokenTypeToDataType(TokenType type);
bool isNumerical(DataType type);

#endif //DATATYPE_H
