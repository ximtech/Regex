#pragma once

#include <stdlib.h>
#include <string.h>

#define MUNIT_ENABLE_ASSERT_ALIASES
#include "munit/munit.h"

#define END_OF_TESTS { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
#define END_OF_PARAMETERS {NULL, NULL}

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))


static char *generateRandomString(size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJK...";
    char *string = malloc(size);
    if (string == NULL) return NULL;
    memset(string, 0, size);

    --size;
    for (size_t n = 0; n < size; n++) {
        int key = rand() % (int) (sizeof charset - 1);
        string[n] = charset[key];
    }
    string[size] = '\0';
    return string;
}
