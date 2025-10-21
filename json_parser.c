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
        printf("pair 0: \n");
        printf("key: %s\n", obj->pairs[0].key);
        printf("value: %s\n", obj->pairs[0].value.value.string);
        printf("pair 1: \n");
        printf("key: %s\n", obj->pairs[1].key);
        printf("value: %s\n", obj->pairs[1].value.value.string);
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
    int c;
    int bufferLen = 0;

    while ((c = fgetc(fd)) != EOF) {
        if (c == '"') {
            break;
        }
        bufferLen++;
    }
    return bufferLen;
}


static void JSONParseObject(FILE* fd, JSONObject* obj) {
    int c;

    bool inString = false;
    char buffer[1024];
    int bufferLen = 0;

    bool inValue = false;

    int pairIndex = 0;
    obj->pairs = malloc(sizeof(JSONPair));

    for (;;) {
        SkipWhitespace(fd);
        c = fgetc(fd);

        switch (c) {
            case '}':
                if (pairIndex == 0 && obj->pairs[0].key == NULL) {
                    free(obj->pairs);
                    obj->pairs = NULL;
                } else {
                    obj->count = pairIndex + 1;
                }
                return;

            case EOF:
                return;

            case '"':
                if (inString) {
                    buffer[bufferLen] = '\0';

                    if (inValue) {
                        obj->pairs[pairIndex].value.type = JSON_VALUE_STRING;
                        obj->pairs[pairIndex].value.value.string = strdup(buffer);
                    } else {
                        obj->pairs[pairIndex].key = strdup(buffer);
                    }

                    bufferLen = 0;
                    inString = false;
                }
                else {
                    inString = true;
                }
                break;
            
            case ':':
                inValue = true;
                break;
            
            case ',':
                pairIndex++;
                obj->pairs = realloc(obj->pairs, sizeof(JSONPair) * (pairIndex + 1));
                inValue = false;
                break;
            
            default:
                if (inString) {
                    buffer[bufferLen] = c;
                    bufferLen++;
                }
                break;
        }
    }
}


static void JSONParseString(FILE* fd, JSONObject* obj, char* buffer, const size_t bufferSize) {
    int c;

    int bufferLen = 0;

    while ((c = fgetc(fd)) != EOF) {
        if (c == '"') {
            buffer[bufferLen] = '\0';
            return;
        }
        if (bufferLen < bufferSize - 1) buffer[bufferLen++] = c;
    }
}

