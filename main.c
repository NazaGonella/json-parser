#include "json_parser.h"

int main() {
    // FILE *file_to_read = fopen("sometext.txt", "r");
    // FILE *file_to_write = fopen("newtext.txt", "w");

    // if (file_to_read == NULL || file_to_write == NULL) {
    //     printf("One file wouldn't open!\n");
    //     return -1;
    // }

    // char c;
    // while ((c=fgetc(file_to_read)) != EOF) {
    //     if (c == '.') {
    //         c = '!';
    //     }
    //     fputc(c, file_to_write);
    // }

    // fclose(file_to_read);
    // fclose(file_to_write);
    JSONParse("text.json", NULL);
}