#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include "json_parser.h"


// Rudimentary Lexer
static void SkipWhitespace(FILE* fd);

// Helper functions
static size_t JSONStringLength(FILE* fd);
static size_t JSONNumberLength(FILE* fd);

// Reads fd and parses it into JSONObject
static bool JSONParseObject(FILE* fd, JSONObject* obj);
static bool JSONParseArray(FILE *fd, JSONArray* array);
static bool JSONParseString(FILE* fd, char* buffer, const size_t bufferSize);
static bool JSONParseNumber(FILE* fd, double* number);
static void JSONParseInteger(FILE* fd, int* number, const size_t bufferSize);
static void JSONParseFraction(FILE* fd, double* number, const size_t bufferSize);
static bool JSONParseBoolean(FILE* fd, bool value); // returns true if the parse is valid
static bool JSONParseNull(FILE* fd); // returns true if the parse is valid
static int JSONDestroyValue(JSONValue* value);
static int JSONDestroyArray(JSONArray* array);


int JSONParse(const char* path, JSONObject* obj) {
    FILE *fd = fopen(path, "r");

    if (!fd) return 1;

    SkipWhitespace(fd);

    int c = fgetc(fd);
    if (c == '{') {
        ungetc(c, fd);
        if (!JSONParseObject(fd, obj)) {
            JSONDestroyObject(obj);
            fclose(fd);
            return 1;
        }
    }
    else {
        fclose(fd);
        return 1;
    }

    fclose(fd);
    return 0;
}


int JSONDestroyObject(JSONObject* obj) {
    if (!obj) return 1;

    for (size_t i = 0; i < obj->count; i++) {
        free(obj->pairs[i].key);
        JSONDestroyValue(&obj->pairs[i].value);
    }
    free(obj->pairs);

    obj->pairs = NULL;
    obj->count = 0;

    return 0;
}


static int JSONDestroyArray(JSONArray* array) {
    if (!array) return 1;
    
    for (size_t i = 0; i < array->count; i++) {
        JSONValue* value = &array->values[i];
        JSONDestroyValue(value);
    }
    free(array->values);

    array->count = 0;

    return 0;
}


static int JSONDestroyValue(JSONValue* value) {
    if (!value) return 1;

    switch (value->type) {

        case JSON_VALUE_STRING: {
            free(value->value.string);
        } break;

        case JSON_VALUE_BOOL: {
        } break;

        case JSON_VALUE_ARRAY: {
            JSONDestroyArray(value->value.array);
            free(value->value.array);
            value->value.array = NULL;
            value->type = JSON_VALUE_NULL;
        } break;

        case JSON_VALUE_NULL: {
        } break;

        case JSON_VALUE_NUMBER: {
        } break;

        case JSON_VALUE_OBJECT: {
            JSONDestroyObject(value->value.object);
            free(value->value.object);
            value->value.object = NULL;
            value->type = JSON_VALUE_NULL;
        } break;
    }

    return 0;
}


static void SkipWhitespace(FILE* fd) {
    int c;
    while ((c=fgetc(fd)) != EOF) {
        switch (c) {
            case '\t': continue; break;       // Horizontal tab
            case '\n': continue; break;       // Linefeed
            case '\r': continue; break;       // Carriage return
            case ' ' : continue; break;       // Space
            default:
                ungetc(c, fd);
                return;
        }
    }
}


static size_t JSONStringLength(FILE* fd) {
    long pos = ftell(fd);           // save position

    size_t len = 0;
    int c = fgetc(fd);

    if (c != '"') {
        fseek(fd, pos, SEEK_SET);   // restore position
        return 0;
    }

    bool escape = false;
    while ((c = fgetc(fd)) != EOF) {
        if (!escape) {
            if (c == '"') break;
            if (c == '\\') {
                escape = true;
                continue;
            }
            len++;
        } else {
            len++;
            escape = false;
        }
    }

    fseek(fd, pos, SEEK_SET);       // restore position
    return len;
}


static size_t JSONNumberLength(FILE* fd) {
    long pos = ftell(fd);           // save position

    size_t bufferLen = 0;
    int c;

    while ((c = fgetc(fd)) != EOF) {
        if (c == '.'|| c == ']' || c == ',' || c == '}' || c == '\t' || c == '\n' || c == '\r' || c == ' ') {
            ungetc(c, fd);
            break;
        }
        bufferLen++;
    }

    fseek(fd, pos, SEEK_SET);       // restore position
    return bufferLen;
}


static bool JSONParseObject(FILE* fd, JSONObject* obj) {
    int c = fgetc(fd);

    if (c != '{')
        return false;

    bool inValue = false;

    int pairIndex = 0;

    JSONPair* tmp = calloc(1, sizeof(JSONPair));
    if (!tmp)
        return false;
    obj->pairs = tmp;

    for (;;) {
        SkipWhitespace(fd);
        c = fgetc(fd);

        switch (c) {
            // End of object
            case '}': {
                if (pairIndex == 0 && obj->pairs[0].key == NULL) {
                    free(obj->pairs);
                    obj->pairs = NULL;
                    obj->count = 0;
                } else {
                    obj->count = pairIndex + 1;
                }
            } return true;

            case EOF: {

            } return false;

            // String
            case '"': {
                ungetc(c, fd);

                size_t bufferSize = JSONStringLength(fd);
                if (bufferSize == 0) {
                    return false;
                }
                char buffer[bufferSize+1];  // +1 for null terminator

                if (!JSONParseString(fd, buffer, bufferSize+1)) {
                    return false;
                }

                if (inValue) {
                    obj->pairs[pairIndex].value.type = JSON_VALUE_STRING;
                    obj->pairs[pairIndex].value.value.string = strdup(buffer);

                    if (!obj->pairs[pairIndex].value.value.string)
                        return false;
                } else {
                    obj->pairs[pairIndex].key = strdup(buffer);

                    if (!obj->pairs[pairIndex].key)
                        return false;
                }
            } break;

            // Object
            case '{' : {
                ungetc(c, fd);

                JSONObject* newObj = malloc(sizeof(JSONObject));
                if (!newObj)
                    return false;

                if (!JSONParseObject(fd, newObj)) {
                    free(newObj);
                    return false;
                }

                obj->pairs[pairIndex].value.type = JSON_VALUE_OBJECT;
                obj->pairs[pairIndex].value.value.object = newObj;
            } break;

            // Array
            case '[' : {
                ungetc(c, fd);

                JSONArray* newArray = malloc(sizeof(JSONArray));
                if (!newArray)
                    return false;

                if (!JSONParseArray(fd, newArray)) {
                    free(newArray);
                    return false;
                }
                
                obj->pairs[pairIndex].value.type = JSON_VALUE_ARRAY;
                obj->pairs[pairIndex].value.value.array = newArray;
            } break;

            // true
            case 't' : {
                ungetc(c, fd);

                if (!JSONParseBoolean(fd, true))
                    return false;

                obj->pairs[pairIndex].value.type = JSON_VALUE_BOOL;
                obj->pairs[pairIndex].value.value.boolean = true;
            } break;

            // false
            case 'f' : {
                ungetc(c, fd);

                if (!JSONParseBoolean(fd, false))
                    return false;

                obj->pairs[pairIndex].value.type = JSON_VALUE_BOOL;
                obj->pairs[pairIndex].value.value.boolean = false;
            } break;

            // null
            case 'n' : {
                ungetc(c, fd);

                if (!JSONParseNull(fd))
                    return false;

                obj->pairs[pairIndex].value.type = JSON_VALUE_NULL;
            } break;
            
            case ':': {
                inValue = true;
            } break;
            
            case ',': {
                pairIndex++;
                JSONPair* tmp = realloc(obj->pairs, sizeof(JSONPair) * (pairIndex + 1));
                if (!tmp) {
                    pairIndex--;
                    return false;
                }
                obj->pairs = tmp;

                inValue = false;
            } break;
            
            // Number
            default: {
                ungetc(c, fd);

                double result = 0;

                if (!JSONParseNumber(fd, &result))
                    return false;

                obj->pairs[pairIndex].value.type = JSON_VALUE_NUMBER;
                obj->pairs[pairIndex].value.value.number = result;
            } break;
        }
    }

    return false;
}


static bool JSONParseArray(FILE *fd, JSONArray* array) {
    int c = fgetc(fd);

    if (c != '[') return false;

    int index = 0;
    bool assigned = false;

    JSONValue* tmp = malloc(sizeof(JSONValue));
    if (!tmp)
        return false;
    array->values = tmp;

    for (;;) {
        SkipWhitespace(fd);
        c = fgetc(fd);

        switch (c) {
            case ']': {
                if (!assigned) {
                    free(array->values);
                    array->values = NULL;
                    array->count = 0;
                } else {
                    array->count = index + 1;
                }
            } return true;

            // String
            case '"': {
                ungetc(c, fd);

                size_t bufferSize = JSONStringLength(fd);
                if (bufferSize == 0) {
                    return false;
                }
                char buffer[bufferSize+1];  // +1 for null terminator

                if (!JSONParseString(fd, buffer, bufferSize+1)) {
                    return false;
                }

                assigned = true;
                array->values[index].type = JSON_VALUE_STRING;
                array->values[index].value.string = strdup(buffer);

                if (!array->values[index].value.string)
                    return false;
            } break;

            // Object
            case '{' : {
                ungetc(c, fd);

                JSONObject* newObj = malloc(sizeof(JSONObject));
                if (!newObj)
                    return false;

                if (!JSONParseObject(fd, newObj))
                    return false;

                assigned = true;
                array->values[index].type = JSON_VALUE_OBJECT;
                array->values[index].value.object = newObj;
            } break;

            // Array
            case '[' : {
                ungetc(c, fd);

                JSONArray* newArray = malloc(sizeof(JSONArray));
                if (!newArray)
                    return false;

                if (!JSONParseArray(fd, newArray))
                    return false;
                
                assigned = true;
                array->values[index].type = JSON_VALUE_ARRAY;
                array->values[index].value.array = newArray;
            } break;

            // true
            case 't' : {
                ungetc(c, fd);

                if (!JSONParseBoolean(fd, true))
                    return false;

                assigned = true;
                array->values[index].type = JSON_VALUE_BOOL;
                array->values[index].value.boolean = true;
            } break;

            // false
            case 'f' : {
                ungetc(c, fd);

                if (!JSONParseBoolean(fd, false))
                    return false;

                assigned = true;
                array->values[index].type = JSON_VALUE_BOOL;
                array->values[index].value.boolean = false;
            } break;

            // null
            case 'n' : {
                ungetc(c, fd);

                if (!JSONParseNull(fd))
                    return false;

                assigned = true;
                array->values[index].type = JSON_VALUE_NULL;
            } break;
            
            case ',': {
                index++;
                JSONValue* tmp = realloc(array->values, sizeof(JSONValue) * (index + 1));
                if (!tmp) {
                    index--;
                    return false;
                }
                array->values = tmp;

                assigned = true;
            } break;
            
            // Number
            default: {
                ungetc(c, fd);

                double result = 0;

                if (!JSONParseNumber(fd, &result))
                    return false;

                assigned = true;
                array->values[index].type = JSON_VALUE_NUMBER;
                array->values[index].value.number = result;
            } break;
        }
    }

    return false;
}


static bool JSONParseString(FILE* fd, char* buffer, const size_t bufferSize) {
    int c = fgetc(fd);

    if (c != '"') return false;

    size_t bufferLen = 0;
    bool escape = false;

    while ((c = fgetc(fd)) != EOF) {
        if (c == '"' && !escape) {
            buffer[bufferLen] = '\0';
            return true;
        }


        if (escape) {
            switch (c) {
                case '"':  c = '"';  break;
                case '\\': c = '\\'; break;
                case '/':  c = '/';  break;
                case 'b':  c = '\b'; break;
                case 'f':  c = '\f'; break;
                case 'n':  c = '\n'; break;
                case 'r':  c = '\r'; break;
                case 't':  c = '\t'; break;
            }
            escape = false;
        } else if (c == '\\') {
            escape = true;
            continue;
        }

        if (bufferLen < bufferSize - 1)
            buffer[bufferLen++] = c;
    }

    buffer[bufferLen] = '\0';
    return false;
}


static bool JSONParseNumber(FILE* fd, double* number) {
    int c = fgetc(fd);
    if ( !(c == '-' || (c >= '0' && c <= '9')) )
        return false;

    double result = 0;
    bool isNegative = false;

    if (c == '-') {
        isNegative = true;
    } else {
        ungetc(c, fd);
    }

    int integer = 0;
    int integerLen = JSONNumberLength(fd);
    JSONParseInteger(fd, &integer, integerLen);

    result += (double) integer;

    double fractionary = 0;
    int peek = fgetc(fd);
    if (peek != '.') {
        ungetc(peek, fd);
    } else {
        int fractionaryLen = JSONNumberLength(fd);
        JSONParseFraction(fd, &fractionary, fractionaryLen);
        result += fractionary;
    }

    *number = isNegative ? -result : result;

    return true;
}


static void JSONParseInteger(FILE* fd, int* number, const size_t bufferSize) {
    int result = 0;
    for (size_t i = 0; i < bufferSize; i++) {
        int c = fgetc(fd);
        if (c < '0' || c > '9') break;
        int digit = c - '0';
        result = result * 10 + digit;
    }
    *number = result;
}


static void JSONParseFraction(FILE* fd, double* number, const size_t bufferSize) {
    double result = 0.0;
    double divisor = 10.0;

    for (size_t i = 0; i < bufferSize; i++) {
        int c = fgetc(fd);
        if (c < '0' || c > '9') break;
        int digit = c - '0';
        result += digit / divisor;
        divisor *= 10.0;
    }

    *number = result;
}


static bool JSONParseBoolean(FILE* fd, bool value) {
    if (value == true) {
        if (fgetc(fd) != 't') return false;
        if (fgetc(fd) != 'r') return false;
        if (fgetc(fd) != 'u') return false;
        if (fgetc(fd) != 'e') return false;
        return true;
    } else if (value == false) {
        if (fgetc(fd) != 'f') return false;
        if (fgetc(fd) != 'a') return false;
        if (fgetc(fd) != 'l') return false;
        if (fgetc(fd) != 's') return false;
        if (fgetc(fd) != 'e') return false;
        return true;
    }
    return false;
}


static bool JSONParseNull(FILE* fd) {
    if (fgetc(fd) != 'n') return false;
    if (fgetc(fd) != 'u') return false;
    if (fgetc(fd) != 'l') return false;
    if (fgetc(fd) != 'l') return false;

    return true;
}

