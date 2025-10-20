#include <stdio.h>


typedef enum JSONValueType {
    JSON_VALUE_STRING,
    JSON_VALUE_NUMBER,
    JSON_VALUE_OBJECT,
    JSON_VALUE_ARRAY,
    JSON_VALUE_BOOL,
    JSON_VALUE_NULL,
} JSONValueType;


typedef struct JSONObject JSONObject;
typedef struct JSONValue JSONValue;
typedef struct JSONPair JSONPair;
typedef struct JSONArray JSONArray;


struct JSONObject {
    JSONPair* pairs;
    size_t count;
};


struct JSONArray {
    JSONValue *values;
    size_t count;
};


struct JSONValue {
    JSONValueType type;
    union value {
        const char* string;
        double number;
        JSONObject object;
        JSONArray array;
        int boolean;
    } value;
};


struct JSONPair {
    char* key;
    JSONValue value;
};


int JSONParse(const char* path, JSONObject* obj);