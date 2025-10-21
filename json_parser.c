#include "json_parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static void SkipWhitespace(FILE* fd);
static size_t JSONStringLength(FILE* fd);   // does not care for escape sequences, so results in slightly larger buffers
static void JSONParseObject(FILE* fd, JSONObject* obj);
static void JSONParseString(FILE* fd, char* buffer, const size_t bufferSize);
static void JSONParseNumber(FILE* fd, const size_t bufferSize);
static bool JSONParseBoolean(FILE* fd, bool value); // returns true if the parse is valid
static bool JSONParseNull(FILE* fd); // returns true if the parse is valid


int JSONParse(const char* path, JSONObject* obj) {
    FILE *fd = fopen(path, "r");

    if (!fd) return -1;

    SkipWhitespace(fd);

    int c = fgetc(fd);
    if (c == '{') {
        JSONParseObject(fd, obj);
        for (int i = 0; i < obj->count; i++){
            JSONPair *pair = &obj->pairs[i];
            printf("pair %d: \n", i);
            printf("key: %s\n", pair->key);
            switch (pair->value.type) {
                case JSON_VALUE_STRING  : printf("value: %s\n\n", pair->value.value.string); break;
                case JSON_VALUE_NUMBER  : break;
                case JSON_VALUE_OBJECT  : break;
                case JSON_VALUE_ARRAY   : break;
                case JSON_VALUE_BOOL    : printf("value: %d\n\n", pair->value.value.boolean); break;
                case JSON_VALUE_NULL    : printf("value: null\n\n"); break;
            }
        }
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
            case 9 : continue; break;       // Horizontal tab
            case 10: continue; break;       // Linefeed
            case 13: continue; break;       // Carriage return
            case 32: continue; break;       // Space
            default:
                ungetc(c, fd);
                return;
        }
    }
}


static size_t JSONStringLength(FILE* fd) {
    long pos = ftell(fd);           // save position

    size_t bufferLen = 0;
    int c;

    while ((c = fgetc(fd)) != EOF) {
        if (c == '"') {
            break;
        }
        bufferLen++;
    }

    fseek(fd, pos, SEEK_SET);       // restore position
    return bufferLen;
}


static void JSONParseObject(FILE* fd, JSONObject* obj) {
    int c;

    bool inValue = false;

    int pairIndex = obj->count;
    if (obj->count == 0)
        obj->pairs = malloc(sizeof(JSONPair));
    else {
        obj->pairs = realloc(obj->pairs, obj->count + 1);
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
            } return;

            case EOF: {

            } return;

            // String
            case '"': {
                size_t bufferSize = JSONStringLength(fd);
                char buffer[bufferSize+1];  // +1 for null terminator

                JSONParseString(fd, buffer, bufferSize+1);

                if (inValue) {
                    obj->pairs[pairIndex].value.type = JSON_VALUE_STRING;
                    obj->pairs[pairIndex].value.value.string = strdup(buffer);
                } else {
                    obj->pairs[pairIndex].key = strdup(buffer);
                }
            } break;

            // Object
            case '{' : {
                JSONParseObject(fd, obj);
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

            } break;
        }
    }
}


static void JSONParseString(FILE* fd, char* buffer, const size_t bufferSize) {
    int c;

    int bufferLen = 0;
    bool escape = false;

    while ((c = fgetc(fd)) != EOF) {
        if (c == '"' && !escape) {
            buffer[bufferLen] = '\0';
            return;
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
}


static bool JSONParseBoolean(FILE* fd, bool value) {
    int c;

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
    int c;

    if (fgetc(fd) != 'u') return false;
    if (fgetc(fd) != 'l') return false;
    if (fgetc(fd) != 'l') return false;

    return true;
}