#include "json_parser.h"

int main() {
    JSONObject obj = {};
    JSONParse("text.json", &obj);
}