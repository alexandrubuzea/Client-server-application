#ifndef DATABASE_H_
#define DATABASE_H_

#include <iostream>
#include <unordered_map>
#include <vector>
#include <memory>
#include <set>

#include "message.h"
#include "client.h"

class database {
    public:
        std::unordered_map<std::string, std::shared_ptr<client>> clients_ids;
        std::unordered_map<std::string, std::set<std::shared_ptr<client>>> subscribers;
        std::unordered_map<int, std::shared_ptr<client>> sockfds;

        void add_client(std::shared_ptr<client> new_client, int sockfd);
        void erase_client(int sockfd);
        void send_message(std::shared_ptr<message> msg);
        void add_subscription(std::string topic, int sockfd, bool st_fwd);
        void remove_subscription(std::string topic, int sockfd);
};
#endif