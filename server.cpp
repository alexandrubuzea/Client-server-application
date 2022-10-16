#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "server_utils.h"
#include "message.h"
#include "database.h"

using namespace std;

bool parse_stdin_command(char *buffer)
{
    fgets(buffer, MAX_LEN - 1, stdin);

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = '\0';

    if (!strcmp("exit", buffer))
        return true;
    
    cout << "Invalid command !\n";
    return false;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 0;
    }

    int udp_skt, tcp_skt;
    struct sockaddr_in udp_client, tcp_client;

    udp_client.sin_family = AF_INET;
    udp_client.sin_port = htons(atoi(argv[1]));
    udp_client.sin_addr.s_addr = INADDR_ANY;

    tcp_client.sin_family = AF_INET;
    tcp_client.sin_port = udp_client.sin_port;
    tcp_client.sin_addr.s_addr = INADDR_ANY;

    socklen_t sock_len = sizeof(struct sockaddr);

    // opening a socket for each type of client: UDP client vs TCP client
    udp_skt = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_skt < 0, "UDP socket cannot be created\n");

    tcp_skt = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_skt < 0, "TCP socket cannot be created\n");

    int flag = 1;
    setsockopt(tcp_skt, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

    int res = bind(udp_skt, (struct sockaddr *)&udp_client, sock_len);
    DIE(res < 0, "Cound not bind - UDP\n");

    res = bind(tcp_skt, (struct sockaddr *)&tcp_client, sock_len);
    DIE(res < 0, "Could not bind - TCP\n");

    res = listen(tcp_skt, MAX_CLIENTS);
    DIE(res < 0, "Listen error!\n");

    fd_set read_fds; // remains constant while running
    fd_set temp_fds; // to be filled up at each step

    // initialization
    FD_ZERO(&read_fds);
    FD_ZERO(&temp_fds);

    FD_SET(tcp_skt, &read_fds);
    FD_SET(udp_skt, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    int fdmax = max(tcp_skt, max(udp_skt, STDIN_FILENO));

    char buffer[MAX_LEN];

    bool stop = false;

    udp_packet_t packet;

    std::shared_ptr<database> db(new database());

    while (!stop) {
        temp_fds = read_fds;
        res = select(fdmax + 1, &temp_fds, NULL, NULL, NULL);
        DIE(res < 0, "Error at select() call !\n");

        for (int i = 0; i <= fdmax; ++i) {

            // I did this in order to avoid nested ifs
            bool is_set = FD_ISSET(i, &temp_fds);

            if (!is_set)
                continue;

            if (i == STDIN_FILENO) {
                stop = parse_stdin_command(buffer);
                continue;
            }

            if (i == udp_skt) {
                memset(buffer, 0, MAX_LEN);

                res = recvfrom(udp_skt, buffer, MAX_LEN, 0, (sockaddr *)&udp_client, &sock_len);
                DIE(res < 0, "Error at UDP recvfrom");
                parse_udp_packet(&packet, buffer);
                auto msg = shared_ptr<message>(new message(&packet, &udp_client));
                if (msg->get_content().size() == 0) {
                    cout << "Invalid message from UDP client!\n";
                    continue;
                }

                db->send_message(msg);

                continue;
            }

            if (i == tcp_skt) {
                int new_skt = accept(tcp_skt, (struct sockaddr *)&tcp_client, &sock_len);
                DIE(new_skt < 0, "Accept error\n");

                fdmax = max(fdmax, new_skt);

                FD_SET(new_skt, &read_fds);

                setsockopt(new_skt, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

                memset(buffer, 0, MAX_LEN);

                res = recv(new_skt, buffer, MAX_LEN, 0);
                DIE(res < 0, "Recv error\n");

                tcp_client_intro_t *msg = (tcp_client_intro_t *)buffer;
                cout << "New client " << msg->id << " connected from " << inet_ntoa(tcp_client.sin_addr) << ":" << ntohs(tcp_client.sin_port) << ".\n";

                std::string id_client = std::string(msg->id);

                if (db->clients_ids.find(id_client) == db->clients_ids.end()) {
                    std::shared_ptr<client> clt (new client(id_client, new_skt));
                    db->sockfds[new_skt] = clt;
                    db->clients_ids[id_client] = clt;
                } else {
                    if (db->clients_ids[id_client]->is_active()) {
                        cout << "Client " << msg->id << " already connected.\n";
                        close(new_skt);
                        FD_CLR(new_skt, &read_fds);
                        continue;
                    }

                    std::shared_ptr<client> clt = db->clients_ids[id_client];
                    clt->set_activity(true);
                    db->sockfds[new_skt] = clt;
                    clt->set_sockfd(new_skt);
                    clt->send_all_enqueued_messages();
                }

                continue;
            }

            // other

            int skt = i;

            // subscribe/unsubscribe/exit

            memset(buffer, 0, MAX_LEN);
            res = recv(skt, buffer, MAX_LEN, 0);
            DIE(res < 0, "Receive error!\n");

            tcp_client_msg_t *msg = (tcp_client_msg_t *)buffer;

            if (msg->type == EXIT) {
                cout << "Client " << db->sockfds[skt]->get_id() << " disconnected.\n";
                close(skt);
                FD_CLR(skt, &read_fds);

                db->sockfds[skt]->set_activity(false);
                db->sockfds.erase(skt);
            } else if (msg->type == SUB) {
                db->add_subscription(msg->topic, skt, msg->store_forward);
            } else {
                db->remove_subscription(msg->topic, skt);
            }
        }
    }

    // close all sockets #defensiveprogramming
    for (int i = 1; i <= fdmax; ++i) {
        if (FD_ISSET(i, &read_fds))
            close(i);
    }

    return 0;
}