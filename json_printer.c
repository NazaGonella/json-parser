#include <stdio.h>
#include "json_parser.h"


static void JSONPrintArray(JSONArray* array, int indent);


void JSONPrintObject(JSONObject* obj, int indent) {
    printf("{\n");

    for (size_t i = 0; i < obj->count; i++) {
        JSONPair* pair = &obj->pairs[i];

        for (int j = 0; j < indent + INDENT_LEN; j++) printf(" ");

        printf("\"%s\" : ", pair->key);

        switch (pair->value.type) {

            case JSON_VALUE_STRING: {
                printf("\"%s\"", pair->value.value.string);
            } break;

            case JSON_VALUE_BOOL: {
                printf("%s", pair->value.value.boolean ? "true" : "false");
            } break;

            case JSON_VALUE_ARRAY: {
                JSONPrintArray(pair->value.value.array, indent + INDENT_LEN);
            } break;

            case JSON_VALUE_NULL: {
                printf("null");
            } break;

            case JSON_VALUE_NUMBER: {
                double num = pair->value.value.number;
                if (num == (int) num)
                    printf("%d", (int) pair->value.value.number);
                else
                    printf("%.15g", pair->value.value.number);
            } break;

            case JSON_VALUE_OBJECT: {
                JSONPrintObject(pair->value.value.object, indent + INDENT_LEN);
            } break;

        }

        if (i != obj->count - 1) printf(",");

        printf("\n");
    }

    for (int i = 0; i < indent; i++) printf(" ");

    printf("}");
}


static void JSONPrintArray(JSONArray* array, int indent) {
    printf("[\n");

    for (size_t i = 0; i < array->count; i++) {

        for (int j = 0; j < indent + INDENT_LEN; j++) printf(" ");

        switch (array->values[i].type) {

            case JSON_VALUE_STRING: {
                printf("\"%s\"", array->values[i].value.string);
            } break;

            case JSON_VALUE_BOOL: {
                printf("%s", array->values[i].value.boolean ? "true" : "false");
            } break;

            case JSON_VALUE_ARRAY: {
                JSONPrintArray(array->values[i].value.array, indent + INDENT_LEN);
            } break;

            case JSON_VALUE_NULL: {
                printf("null");
            } break;

            case JSON_VALUE_NUMBER: {
                double num = array->values[i].value.number;
                if (num == (int) num)
                    printf("%d", (int) num);
                else
                    printf("%.15g", num);
            } break;

            case JSON_VALUE_OBJECT: {
                JSONPrintObject(array->values[i].value.object, indent + INDENT_LEN);
            } break;

        }

        if (i != array->count - 1) printf(",");

        printf("\n");
    }

    for (int i = 0; i < indent; i++) printf(" ");

    printf("]");
}