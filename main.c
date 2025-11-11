#include <assert.h>
#include "json_parser.h"

int main() {
    JSONObject obj = {};
    JSONParse("test.json", &obj);
    JSONPrintObject(&obj, 0);
    printf("\n");
    return 0;
}
