#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json_parser.h"

void test_empty_object() {
    JSONObject obj;
    if (!JSONParse("test_empty_object.json", &obj)) {
        printf("test_empty_object: FAILED (parse error)\n");
        return;
    }
    if (obj.count != 0) {
        printf("test_empty_object: FAILED (count != 0)\n");
    } else {
        printf("test_empty_object: PASSED\n");
    }
    free(obj.pairs);
}

void test_simple_object() {
    JSONObject obj;
    if (!JSONParse("test_simple_object.json", &obj)) {
        printf("test_simple_object: FAILED (parse error)\n");
        return;
    }

    int found_name = 0, found_age = 0;
    for (size_t i = 0; i < obj.count; i++) {
        JSONPair *p = &obj.pairs[i];
        if (strcmp(p->key, "name") == 0 && p->value.type == JSON_VALUE_STRING &&
            strcmp(p->value.value.string, "Alice") == 0)
            found_name = 1;
        if (strcmp(p->key, "age") == 0 && p->value.type == JSON_VALUE_NUMBER &&
            p->value.value.number == 30)
            found_age = 1;
    }

    if (found_name && found_age) {
        printf("test_simple_object: PASSED\n");
    } else {
        printf("test_simple_object: FAILED (values mismatch)\n");
    }

    for (size_t i = 0; i < obj.count; i++) {
        free(obj.pairs[i].key);
        if (obj.pairs[i].value.type == JSON_VALUE_STRING)
            free((char*)obj.pairs[i].value.value.string);
    }
    free(obj.pairs);
}

void test_nested_object() {
    JSONObject obj;
    if (!JSONParse("test_nested_object.json", &obj)) {
        printf("test_nested_object: FAILED (parse error)\n");
        return;
    }

    int found_address = 0;
    for (size_t i = 0; i < obj.count; i++) {
        JSONPair *p = &obj.pairs[i];
        if (strcmp(p->key, "address") == 0 && p->value.type == JSON_VALUE_OBJECT) {
            JSONObject addr = p->value.value.object;
            if (addr.count == 2) found_address = 1;
        }
    }

    if (found_address) {
        printf("test_nested_object: PASSED\n");
    } else {
        printf("test_nested_object: FAILED (nested object mismatch)\n");
    }

    // Free memory: recursive free needed for nested objects
    for (size_t i = 0; i < obj.count; i++) {
        free(obj.pairs[i].key);
        if (obj.pairs[i].value.type == JSON_VALUE_STRING)
            free((char*)obj.pairs[i].value.value.string);
        else if (obj.pairs[i].value.type == JSON_VALUE_OBJECT) {
            JSONObject nested = obj.pairs[i].value.value.object;
            for (size_t j = 0; j < nested.count; j++) {
                free(nested.pairs[j].key);
                if (nested.pairs[j].value.type == JSON_VALUE_STRING)
                    free((char*)nested.pairs[j].value.value.string);
            }
            free(nested.pairs);
        }
    }
    free(obj.pairs);
}

int main() {
    // JSONObject obj = {};
    // JSONParse("test_nested_object.json", &obj);
    // printf("{\n");
    // JSONPrintObject(&obj, 1);
    // printf("}\n");
    // test_empty_object();
    test_nested_object();
    test_simple_object();
    return 0;
}
