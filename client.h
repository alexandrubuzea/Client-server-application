#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
#include <unordered_map>
#include <iostream>
#include <queue>
#include <memory>
#include <string.h>

#include "message.h"
#include "utils.h"

class client {
    private:
        std::string id;
        bool active;
        std::unordered_map<std::string, bool> topics;
        std::queue<std::shared_ptr<message>> waiting_messages;
        int sockfd;

    public:
        client(std::string id_client, int sockfd);
        std::string get_id();
        void set_activity(bool activity);
        bool is_active();
        void enqueue_message(std::shared_ptr<message> msg);
        void receive_message(std::shared_ptr<message> msg);
        void set_sockfd(int sockfd);
        void add_subscription(std::string topic, bool st_fwd);
        void remove_subscription(std::string topic);
        void send_all_enqueued_messages();
        void send_message(std::shared_ptr<message> msg);
};
#endif