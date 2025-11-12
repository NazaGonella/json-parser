#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "json_parser.h"

/* helper: find pair by key */
static JSONPair* find_pair(JSONObject* obj, const char* key) {
    if (!obj) return NULL;
    for (size_t i = 0; i < obj->count; ++i) {
        if (obj->pairs[i].key && strcmp(obj->pairs[i].key, key) == 0)
            return &obj->pairs[i];
    }
    return NULL;
}

/* approx equal for doubles */
static int double_eq(double a, double b) {
    double diff = a - b;
    if (diff < 0) diff = -diff;
    return diff < 1e-9;
}

static void test_basic(void) {
    JSONObject obj = {};
    int rc = JSONParse("tests/basic.json", &obj);
    assert(rc == 0);

    JSONPair* p = find_pair(&obj, "message");
    assert(p && p->value.type == JSON_VALUE_STRING);
    assert(strcmp(p->value.value.string, "Hello \"world\"\\nLine2 and backslash \\\\") == 0);

    p = find_pair(&obj, "number");
    assert(p && p->value.type == JSON_VALUE_NUMBER);
    assert(double_eq(p->value.value.number, -123.456));

    p = find_pair(&obj, "integer");
    assert(p && p->value.type == JSON_VALUE_NUMBER);
    assert(double_eq(p->value.value.number, 42.0));

    p = find_pair(&obj, "flag_true");
    assert(p && p->value.type == JSON_VALUE_BOOL && p->value.value.boolean == true);

    p = find_pair(&obj, "flag_false");
    assert(p && p->value.type == JSON_VALUE_BOOL && p->value.value.boolean == false);

    p = find_pair(&obj, "null_value");
    assert(p && p->value.type == JSON_VALUE_NULL);

    p = find_pair(&obj, "array");
    assert(p && p->value.type == JSON_VALUE_ARRAY);
    assert(p->value.value.array->count == 5);

    p = find_pair(&obj, "empty_object");
    assert(p && p->value.type == JSON_VALUE_OBJECT);
    assert(p->value.value.object->count == 0);

    p = find_pair(&obj, "empty_array");
    assert(p && p->value.type == JSON_VALUE_ARRAY);
    assert(p->value.value.array->count == 0);

    JSONDestroyObject(&obj);
    printf("test_basic: OK\n");
}

static void test_escapes_and_unicode(void) {
    JSONObject obj = {};
    int rc = JSONParse("tests/escapes.json", &obj);
    assert(rc == 0);

    JSONPair* p = find_pair(&obj, "s");
    assert(p && p->value.type == JSON_VALUE_STRING);
    /* Parser doesn't decode \u escapes; expect raw \u sequences */
    assert(strstr(p->value.value.string, "Smile") != NULL);
    assert(strstr(p->value.value.string, "\\u263A") != NULL);
    assert(strstr(p->value.value.string, "\\u4F60") != NULL);
    assert(strstr(p->value.value.string, "\\u597D") != NULL);

    JSONDestroyObject(&obj);
    printf("test_escapes_and_unicode: OK\n");
}

static void test_nested_array_object(void) {
    JSONObject obj = {};
    int rc = JSONParse("tests/array_nested.json", &obj);
    assert(rc == 0);

    JSONPair* p = find_pair(&obj, "arr");
    assert(p && p->value.type == JSON_VALUE_ARRAY);
    JSONArray* arr = p->value.value.array;
    assert(arr->count == 4);

    /* second element is an object with key "k" -> "v" */
    assert(arr->values[1].type == JSON_VALUE_OBJECT);
    JSONPair* q = find_pair(arr->values[1].value.object, "k");
    assert(q && q->value.type == JSON_VALUE_STRING);
    assert(strcmp(q->value.value.string, "v") == 0);

    /* third element is an array with booleans */
    assert(arr->values[2].type == JSON_VALUE_ARRAY);
    JSONArray* inner = arr->values[2].value.array;
    assert(inner->count == 2);
    assert(inner->values[0].type == JSON_VALUE_BOOL && inner->values[0].value.boolean == true);
    assert(inner->values[1].type == JSON_VALUE_BOOL && inner->values[1].value.boolean == false);

    JSONDestroyObject(&obj);
    printf("test_nested_array_object: OK\n");
}

int main(void) {
    printf("Running parser tests...\n");
    test_basic();
    test_escapes_and_unicode();
    test_nested_array_object();
    printf("ALL TESTS PASSED\n");
    return 0;
}