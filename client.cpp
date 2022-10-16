#include "client.h"

// methods for client class

std::string client::get_id()
{
    return this->id;
}

void client::set_activity(bool activity)
{
    this->active = activity;
}

void client::add_subscription(std::string topic, bool st_fwd)
{
    this->topics[topic] = st_fwd;
}

bool client::is_active()
{
    return this->active;
}

void client::enqueue_message(std::shared_ptr<message> msg)
{
    this->waiting_messages.push(msg);
}

void client::receive_message(std::shared_ptr<message> msg)
{
    if (this->active) {
        send_message(msg);
        return;
    }

    this->enqueue_message(msg);
}

// send a message for the given client - using its socket
void client::send_message(std::shared_ptr<message> msg)
{
    char buff[MAX_LEN];
    memset(buff, 0, MAX_LEN);

    tcp_server_msg_t *tcp_msg = (tcp_server_msg_t *)buff;
    tcp_msg->len = strlen(msg->compose().c_str());

    strcpy(tcp_msg->message, msg->compose().c_str());

    int res = send(this->sockfd, buff, MAX_LEN, 0);
    DIE(res < 0, "Send error\n");
}

void client::set_sockfd(int sockfd)
{
    this->sockfd = sockfd;
}

void client::remove_subscription(std::string topic)
{
    this->topics.erase(topic);
}

client::client(std::string id_client, int sockfd)
{
    this->active = true;
    this->sockfd = sockfd;
    this->id = id_client;
}

void client::send_all_enqueued_messages()
{
    while (!this->waiting_messages.empty()) {
        auto msg = this->waiting_messages.front();
        send_message(msg);
        this->waiting_messages.pop();
    }
}