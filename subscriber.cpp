#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>

#include "utils.h"
#include "server_utils.h"

using namespace std;

int main(int argc, char **argv)
{
    int sockfd;

    if (argc < 4) {
        fprintf(stderr, "Usage: %s SERVER_ADDRESS SERVER_PORT\n", argv[0]);
        return 0;
    }

    struct sockaddr_in serv_addr;

    fd_set read_fds, temp_fds;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "Cannot create subscriber TCP socket !\n");

    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    
    int rc = inet_aton(argv[2], &serv_addr.sin_addr);
    DIE(rc < 0, "inet_aton error\n");

    rc = connect(sockfd, (sockaddr *)&serv_addr, sizeof(serv_addr));
    DIE(rc < 0, "Connect error! \n");
    int fdmax = std::max(STDIN_FILENO, sockfd);

    int flag = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

    char buff[MAX_LEN];
    memset(buff, 0, MAX_LEN);

    tcp_client_intro_t *msg = (tcp_client_intro_t *)buff;
    msg->len = strlen(argv[1]) + 1;
    memset(msg->id, 0, ID_SIZE);
    memcpy(msg->id, argv[1], msg->len + 1);
    
    rc = send(sockfd, buff, MAX_LEN, 0);
    DIE(rc < 0, "Send error !\n");

    
    while (1) {
        temp_fds = read_fds;
        rc = select(fdmax + 1, &temp_fds, NULL, NULL, NULL);
        DIE(rc < 0, "Error at select() call !\n");

        if (rc == 0)
            break;

        if (FD_ISSET(STDIN_FILENO, &temp_fds)) {
            // from STDIN
            char buffer[COMMAND_SIZE];
            fgets(buffer, COMMAND_SIZE, stdin);
            if (buffer[strlen(buffer) - 1] == '\n')
                buffer[strlen(buffer) - 1] = '\0';
            
            const char delim[] = " ";
            char *token = strtok(buffer, delim);

            memset(buff, 0, MAX_LEN);

            if (!strcmp(token, "subscribe")) {
                char *topic = strtok(NULL, delim);
                tcp_client_msg_t *msg2 = (tcp_client_msg_t *)buff;
                msg2->len = sizeof(msg2->store_forward) + sizeof(msg2->type) + strlen(topic) + 1;
                memcpy(msg2->topic, topic, TOPIC_SIZE);
                msg2->store_forward = atoi(strtok(NULL, delim));
                msg2->type = SUB;

                cout << "Subscribed to " << msg2->topic << ".\n";
 
                rc = send(sockfd, buff, MAX_LEN, 0);
                DIE(rc < 0, "Subscription send error");
                continue;
            }

            if (!strcmp(token, "unsubscribe")) {
                char *topic = strtok(NULL, delim);
                tcp_client_msg_t *msg2 = (tcp_client_msg_t *)buff;
                msg2->len = sizeof(msg2->store_forward) + sizeof(msg2->type) + strlen(topic) + 1;
                memcpy(msg2->topic, topic, TOPIC_SIZE);
                msg2->store_forward = 0;
                msg2->type = UNSUB;

                cout << "Unsubscribed from " << msg2->topic << ".\n";

                rc = send(sockfd, buff, MAX_LEN, 0);
                DIE(rc < 0, "Unsubscription send error");
                continue;
            }

            if (!strcmp(token, "exit")) {
                tcp_client_msg_t *msg2 = (tcp_client_msg_t *)buff;
                msg2->len = sizeof(msg2->type);
                msg2->type = EXIT;

                rc = send(sockfd, buff, MAX_LEN, 0);
                DIE(rc < 0, "Exit send error!\n");
                break;
            }

        } else if (FD_ISSET(sockfd, &temp_fds)) {
            // from socket

            memset(buff, 0, MAX_LEN);
            rc = recv(sockfd, buff, MAX_LEN, 0);
            DIE(rc < 0, "Recv error!\n");

            if (rc == 0)
                break; // exit from server
            
            tcp_server_msg_t *msg2 = (tcp_server_msg_t *)buff;
            cout << msg2->message << "\n";
        }
    }

    close(sockfd);

    return 0;
}