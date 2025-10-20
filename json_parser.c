#include "json_parser.h"
#include <stdbool.h>
#include <stdlib.h>


static void SkipWhitespace(FILE* fd);
static void JSONParseObject(FILE* fd, JSONObject* obj);
static void JSONParseString(FILE* fd, JSONObject* obj);


int JSONParse(const char* path, JSONObject* obj) {
    FILE *fd = fopen(path, "r");

    if (!fd) return -1;

    SkipWhitespace(fd);

    int c = fgetc(fd);
    if (c == '{') {
        JSONParseObject(fd, obj);
        return 0;
    }
    else if (c == EOF) {
        return 0;
    }
    else {
        return -1;
    }

    fclose(fd);
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

static void JSONParseObject(FILE* fd, JSONObject* obj) {
    int c;

    bool inString = false;
    char buffer[1024];
    int bufferLen = 0;

    bool inValue = false;

    int pairIndex = 0;

    // obj->pairs = malloc(sizeof(JSONPair));

    for (;;) {
        SkipWhitespace(fd);
        c = fgetc(fd);

        switch (c) {
            case '}':
                return;

            case EOF:
                return;

            case '"':
                if (inString) {
                    buffer[bufferLen] = '\0';
                    if (!inValue) {
                        printf("key: %s\n", buffer);
                    } else {
                        printf("value: %s\n\n", buffer);
                        inValue = false;
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
