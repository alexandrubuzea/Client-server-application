#ifndef SERVER_UTILS_H_
#define SERVER_UTILS_H_

#include <iostream>
#include "utils.h"

typedef struct {
    uint8_t topic[TOPIC_SIZE];
    uint8_t type;
    uint8_t content[CONTENT_SIZE];
} udp_packet_t;

typedef struct __attribute__ ((packed)) {
    int len;
    uint8_t type;
    uint8_t store_forward;
    char topic[TOPIC_SIZE];
} tcp_client_msg_t;

typedef struct __attribute__ ((packed)) {
    int len;
    char message[MAX_LEN];
} tcp_server_msg_t;

typedef struct __attribute__ ((packed)) {
    int len;
    char id[ID_SIZE];
} tcp_client_intro_t;

void parse_udp_packet(udp_packet_t *packet, char *buffer);

#endif