#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "json_parser.h"

void test_parse_empty_object() {
    JSONObject obj;
    int result = JSONParse("test_empty_object.json", &obj);
    assert(result == 1);
    assert(obj.count == 0);
}

void test_parse_simple_object() {
    JSONObject obj;
    int result = JSONParse("test_simple_object.json", &obj);
    assert(result == 1);
    assert(obj.count == 2);
    assert(strcmp(obj.pairs[0].key, "name") == 0);
    assert(obj.pairs[0].value.type == JSON_VALUE_STRING);
    assert(strcmp(obj.pairs[0].value.value.string, "Alice") == 0);
    assert(strcmp(obj.pairs[1].key, "age") == 0);
    assert(obj.pairs[1].value.type == JSON_VALUE_NUMBER);
    assert(obj.pairs[1].value.value.number == 30);
}

void test_parse_nested_object() {
    JSONObject obj;
    int result = JSONParse("test_nested_object.json", &obj);
    assert(result == 1);
    assert(obj.count == 1);
    assert(strcmp(obj.pairs[0].key, "person") == 0);
    assert(obj.pairs[0].value.type == JSON_VALUE_OBJECT);
    JSONObject* nested = obj.pairs[0].value.value.object;
    assert(nested->count == 2);
    assert(strcmp(nested->pairs[0].key, "first") == 0);
    assert(nested->pairs[0].value.type == JSON_VALUE_STRING);
    assert(strcmp(nested->pairs[0].value.value.string, "Alice") == 0);
    assert(strcmp(nested->pairs[1].key, "last") == 0);
    assert(nested->pairs[1].value.type == JSON_VALUE_STRING);
    assert(strcmp(nested->pairs[1].value.value.string, "Smith") == 0);
}

void test_parse_array() {
    JSONObject obj;
    int result = JSONParse("test_array.json", &obj);
    // JSONPrintObject(&obj, 0);
    // printf("obj.count: %d\n", (int) obj.count);
    assert(result == 1);
    assert(obj.count == 1);
    assert(strcmp(obj.pairs[0].key, "numbers") == 0);
    assert(obj.pairs[0].value.type == JSON_VALUE_ARRAY);
    JSONArray* array = obj.pairs[0].value.value.array;
    assert(array->count == 3);
    assert(array->values[0].type == JSON_VALUE_NUMBER && array->values[0].value.number == 1);
    assert(array->values[1].type == JSON_VALUE_NUMBER && array->values[1].value.number == 2);
    assert(array->values[2].type == JSON_VALUE_NUMBER && array->values[2].value.number == 3);
}

void test_parse_bool_and_null() {
    JSONObject obj;
    int result = JSONParse("test_bool_null.json", &obj);
    assert(result == 1);
    assert(obj.count == 2);
    assert(strcmp(obj.pairs[0].key, "active") == 0);
    assert(obj.pairs[0].value.type == JSON_VALUE_BOOL);
    assert(obj.pairs[0].value.value.boolean == 1);
    assert(strcmp(obj.pairs[1].key, "data") == 0);
    assert(obj.pairs[1].value.type == JSON_VALUE_NULL);
}

int main() {
    test_parse_empty_object();
    test_parse_simple_object();
    test_parse_nested_object();
    test_parse_array();
    test_parse_bool_and_null();
    printf("All JSON parser tests passed.\n");
    return 0;
}
