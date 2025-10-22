#include "json_parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static void SkipWhitespace(FILE* fd);
static size_t JSONStringLength(FILE* fd);   // does not care for escape sequences, so results in slightly larger buffers
static size_t JSONNumberLength(FILE* fd);
static bool JSONParseObject(FILE* fd, JSONObject* obj);
static bool JSONParseString(FILE* fd, char* buffer, const size_t bufferSize);
static void JSONParseInteger(FILE* fd, int* number, const size_t bufferSize);
static void JSONParseFraction(FILE* fd, double* number, const size_t bufferSize);
static bool JSONParseBoolean(FILE* fd, bool value); // returns true if the parse is valid
static bool JSONParseNull(FILE* fd); // returns true if the parse is valid
static void JSONPrintObject(JSONObject* obj, int indent);


int JSONParse(const char* path, JSONObject* obj) {
    FILE *fd = fopen(path, "r");

    if (!fd) return -1;

    SkipWhitespace(fd);

    int c = fgetc(fd);
    if (c == '{') {
        JSONParseObject(fd, obj);
        // for (int i = 0; i < obj->count; i++){
        //     JSONPair *pair = &obj->pairs[i];
        //     printf("pair %d: \n", i);
        //     printf("key: %s\n", pair->key);
        //     switch (pair->value.type) {
        //         case JSON_VALUE_STRING  : printf("value: %s\n\n", pair->value.value.string); break;
        //         case JSON_VALUE_NUMBER  : printf("value: %f\n\n", pair->value.value.number); break;;
        //         case JSON_VALUE_OBJECT  : break;
        //         case JSON_VALUE_ARRAY   : break;
        //         case JSON_VALUE_BOOL    : printf("value: %d\n\n", pair->value.value.boolean); break;
        //         case JSON_VALUE_NULL    : printf("value: null\n\n"); break;
        //     }
        // }
        printf("{\n");
        JSONPrintObject(obj, 1);
        printf("}\n");

    }
    else {
        fclose(fd);
        return -1;
    }

    fclose(fd);
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

    size_t bufferLen = 0;
    int c = fgetc(fd);

    if (c != '"') {
        fseek(fd, pos, SEEK_SET);
        return 0;
    }


    while ((c = fgetc(fd)) != EOF) {
        if (c == '"') {
            break;
        }
        bufferLen++;
    }

    fseek(fd, pos, SEEK_SET);       // restore position
    return bufferLen;
}


static size_t JSONNumberLength(FILE* fd) {
    long pos = ftell(fd);           // save position

    size_t bufferLen = 0;
    int c;

    while ((c = fgetc(fd)) != EOF) {
        if (c == '.' || c == ',' || c == '}' || c == '\t' || c == '\n' || c == '\r' || c == ' ') {
            break;
        }
        bufferLen++;
    }

    fseek(fd, pos, SEEK_SET);       // restore position
    return bufferLen;
}


static bool JSONParseObject(FILE* fd, JSONObject* obj) {
    int c;

    bool inValue = false;

    int pairIndex = obj->count;

    if (obj->count == 0)
        obj->pairs = malloc(sizeof(JSONPair));
    else {
        obj->pairs = realloc(obj->pairs, sizeof(JSONPair) * (obj->count + 1));
    }

    for (;;) {
        SkipWhitespace(fd);
        c = fgetc(fd);

        switch (c) {
            // End of object
            case '}': {
                if (pairIndex == 0 && obj->pairs[0].key == NULL) {
                    free(obj->pairs);
                    obj->pairs = NULL;
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
                } else {
                    obj->pairs[pairIndex].key = strdup(buffer);
                }
            } break;

            // Object
            case '{' : {
                JSONObject newObj = {};
                if (!JSONParseObject(fd, &newObj)) {
                    return false;
                }
                obj->pairs[pairIndex].value.type = JSON_VALUE_OBJECT;
                obj->pairs[pairIndex].value.value.object = newObj;
            } break;

            // Array
            case '[' : {

            } break;

            // true
            case 't' : {
                JSONParseBoolean(fd, true);

                obj->pairs[pairIndex].value.type = JSON_VALUE_BOOL;
                obj->pairs[pairIndex].value.value.boolean = true;
            } break;

            // false
            case 'f' : {
                JSONParseBoolean(fd, true);

                obj->pairs[pairIndex].value.type = JSON_VALUE_BOOL;
                obj->pairs[pairIndex].value.value.boolean = false;
            } break;

            // null
            case 'n' : {
                ungetc(c, fd);
                JSONParseNull(fd);

                obj->pairs[pairIndex].value.type = JSON_VALUE_NULL;
            } break;
            
            case ':': {
                inValue = true;
            } break;
            
            case ',': {
                pairIndex++;
                obj->pairs = realloc(obj->pairs, sizeof(JSONPair) * (pairIndex + 1));
                inValue = false;
            } break;
            
            default: {
                // Number
                if (c == '-' || (c >= '0' && c <= '9')) {
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

                    obj->pairs[pairIndex].value.type = JSON_VALUE_NUMBER;
                    obj->pairs[pairIndex].value.value.number = isNegative ? -result : result;
                }
            } break;
        }
    }
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
        if (fgetc(fd) != 'r') return false;
        if (fgetc(fd) != 'u') return false;
        if (fgetc(fd) != 'e') return false;
        return true;
    } else if (value == false) {
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


static void JSONPrintObject(JSONObject* obj, int indent) {
    for (size_t i = 0; i < obj->count; i++) {
        for (int j = 0; j < indent; j++) printf("  ");
        JSONPair* pair = &obj->pairs[i];
        printf("\"%s\": ", pair->key);

        switch (pair->value.type) {
            case JSON_VALUE_STRING:
                printf("\"%s\"", pair->value.value.string);
                break;
            case JSON_VALUE_NUMBER:
                printf("%g", pair->value.value.number);
                break;
            case JSON_VALUE_BOOL:
                printf(pair->value.value.boolean ? "true" : "false");
                break;
            case JSON_VALUE_NULL:
                printf("null");
                break;
            case JSON_VALUE_OBJECT:
                printf("{\n");
                JSONPrintObject(&pair->value.value.object, indent + 1);
                for (int j = 0; j < indent; j++) printf("  ");
                printf("}");
                break;
            default:
                printf("<?>");
                break;
        }

        if (i < obj->count - 1)
            printf(",");
        printf("\n");
    }
}
