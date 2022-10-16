#ifndef UTILS_H_
#define UTILS_H_

#include <iostream>
#include <string.h>

#define DIE(assert, message)        \
do {                                \
    if (assert) {                   \
        fprintf(stderr, message);   \
        exit(-1);                   \
    }                               \
} while(0);

#define MY_INT_VALUE 0
#define MY_SHR_VALUE 1
#define MY_FLT_VALUE 2
#define MY_STR_VALUE 3

#define IP_SIZE 4

#define TOPIC_SIZE 50
#define CONTENT_SIZE 1501
#define TYPE_SIZE 20
#define MAX_LENGTH 2000
#define MAX_LEN 2000
#define ID_SIZE 50
#define COMMAND_SIZE 300
#define MAX_CLIENTS 20

#define SUB 0
#define UNSUB 1
#define EXIT 2

#endif