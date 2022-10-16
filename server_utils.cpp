#include "server_utils.h"

void parse_udp_packet(udp_packet_t *packet, char *buffer)
{
    memset(packet, 0, sizeof(*packet));
    memcpy(packet->topic, buffer, TOPIC_SIZE);
    packet->type = buffer[TOPIC_SIZE];

    if (packet->type == MY_INT_VALUE) { // INT
        memcpy(packet->content, buffer + TOPIC_SIZE + 1, 1 + sizeof(uint32_t));
        return;
    }

    if (packet->type == MY_SHR_VALUE) {
        memcpy(packet->content, buffer + TOPIC_SIZE + 1, sizeof(uint16_t));
        return;
    }

    if (packet->type == MY_FLT_VALUE) {
        memcpy(packet->content, buffer + TOPIC_SIZE + 1, 1 + sizeof(uint32_t) + sizeof(uint8_t));
        return;
    }

    if (packet->type == MY_STR_VALUE) {
        memcpy(packet->content, buffer + TOPIC_SIZE + 1, CONTENT_SIZE - 1);
        return;
    }
}