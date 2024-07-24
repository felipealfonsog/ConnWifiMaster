#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "connman.h"

char* run_command(const char *command) {
    FILE *fp;
    char *result = NULL;
    size_t size = 0;
    char buffer[128];

    fp = popen(command, "r");
    if (fp == NULL) {
        return strdup("Error: Unable to execute command.");
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        size_t buffer_length = strlen(buffer);
        char *new_result = realloc(result, size + buffer_length + 1);
        if (new_result == NULL) {
            free(result);
            result = strdup("Error: Memory allocation failed.");
            break;
        }
        result = new_result;
        strcpy(result + size, buffer);
        size += buffer_length;
    }

    pclose(fp);
    if (result == NULL || size == 0) {
        result = strdup("No networks found.");
    }
    return result;
}
