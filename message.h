#ifndef MESSAGE_H_
#define MESSAGE_H_

#include <iostream>
#include <string>
#include <unistd.h>
#include "server_utils.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class message {
    private:
        std::string udp_ip;
        uint16_t port;
        std::string topic;
        std::string type;
        std::string content;
    
    public:
        message(udp_packet_t *packet, sockaddr_in *udp_client);
        std::string get_udp_ip();
        uint16_t get_port();
        std::string get_topic();
        std::string get_type();
        std::string get_content();
        std::string compose();
};

#endif