#include "json_parser.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


static void SkipWhitespace(FILE* fd);
static size_t JSONStringLength(FILE* fd);
static void JSONParseObject(FILE* fd, JSONObject* obj);
static void JSONParseString(FILE* fd, JSONObject* obj, char* buffer, const size_t bufferSize);


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
            printf("value: %s\n\n", pair->value.value.string);
        }
    }
    else {
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

    char buffer[1024];
    int bufferLen = 0;

    bool inValue = false;

    int pairIndex = 0;
    obj->pairs = malloc(sizeof(JSONPair));

    for (;;) {
        SkipWhitespace(fd);
        c = fgetc(fd);

        switch (c) {
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

            case '"': {
                size_t bufferSize = JSONStringLength(fd);
                char buffer[bufferSize+1];  // +1 for null terminator

                JSONParseString(fd, obj, buffer, bufferSize+1);

                if (inValue) {
                    obj->pairs[pairIndex].value.type = JSON_VALUE_STRING;
                    obj->pairs[pairIndex].value.value.string = strdup(buffer);
                } else {
                    obj->pairs[pairIndex].key = strdup(buffer);
                }
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


static void JSONParseString(FILE* fd, JSONObject* obj, char* buffer, const size_t bufferSize) {
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

