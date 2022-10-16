#include "database.h"

// adding a client in all components of a database
void database::add_client(std::shared_ptr<client> new_client, int sockfd)
{
    if (this->sockfds.find(sockfd) != this->sockfds.end())
        return;
    
    this->sockfds[sockfd] = new_client;
    this->clients_ids[new_client->get_id()] = new_client;
}

void database::send_message(std::shared_ptr<message> msg)
{
    for (auto &clt : this->subscribers[msg->get_topic()]) {
        clt->receive_message(msg);
    }
}

void database::erase_client(int sockfd)
{
    auto ptr = this->sockfds[sockfd];
    ptr->set_activity(false);
    this->sockfds.erase(sockfd);    
}

void database::add_subscription(std::string topic, int sockfd, bool st_fwd)
{
    auto ptr = this->sockfds[sockfd];
    
    this->subscribers[topic].insert(ptr);
    ptr->add_subscription(topic, st_fwd);
}

void database::remove_subscription(std::string topic, int sockfd)
{
    auto ptr = this->sockfds[sockfd];
    this->subscribers[topic].erase(ptr);

    ptr->remove_subscription(topic);
}