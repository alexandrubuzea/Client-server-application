#include "message.h"

// a constructor for the message received by server from UDP client

message::message(udp_packet_t *packet, sockaddr_in *udp_client)
{   
    this->udp_ip = std::string(inet_ntoa(udp_client->sin_addr));
    this->port = ntohs(udp_client->sin_port);
    this->topic = std::string((char *)packet->topic);
    
    // for each case of data type
    switch (packet->type) {
        case MY_INT_VALUE: {
            this->type = "INT";
            break;
        }

        case MY_SHR_VALUE: {
            this->type = "SHORT_REAL";
            break;
        }

        case MY_FLT_VALUE: {
            this->type = "FLOAT";
            break;
        }

        case MY_STR_VALUE: {
            this->type = "STRING";
            break;
        }
        
        default: {
            this->type = "";
            break;
        }
    }

    // convert the given data to a std::string
    switch (packet->type) {
        case MY_INT_VALUE: {
            uint8_t sign = packet->content[0];
            uint32_t num;
            memcpy(&num, packet->content + 1, sizeof(uint32_t));
            num = ntohl(num);
            if (sign)
                this->content = std::string("-") + std::to_string(num);
            else
                this->content = std::to_string(num);
            break;
        }

        case MY_SHR_VALUE: {
            uint16_t num2;
            memcpy(&num2, packet->content, sizeof(uint16_t));
            num2 = ntohs(num2);
            this->content = std::to_string(num2 / 100) + ".";
            uint8_t remainder = num2 % 100;
            if (remainder < 10)
                this->content += ("0" + std::to_string(remainder));
            else
                this->content += std::to_string(remainder);
            break;
        }

        case MY_FLT_VALUE: {
            uint8_t sign2 = packet->content[0];
            uint32_t num3;
            memcpy(&num3, packet->content + 1, sizeof(uint32_t));
            num3 = ntohl(num3);
            this->content = "";
            if (sign2)
                this->content += "-";
            uint8_t power = packet->content[1 + sizeof(uint32_t)];
            double val = (double)num3;
            for (int i = 0; i < power; ++i)
                val /= 10;
            
            this->content += std::to_string(val);
            break;
        }
        
        case MY_STR_VALUE: {
            this->content = std::string((char *)packet->content);
            break;
        }

        default: {
            this->content = "";
            break;
        }
    }
}

std::string message::get_udp_ip()
{
    return this->udp_ip;
}

std::string message::get_content()
{
    return this->content;
}

uint16_t message::get_port()
{
    return this->port;
}

std::string message::get_topic()
{
    return this->topic;
}

std::string message::get_type()
{
    return this->type;
}

std::string message::compose()
{
    std::string phrase = "";
    phrase += this->udp_ip;
    phrase += ":";
    phrase += std::to_string(this->port);
    phrase += " - ";
    phrase += this->topic;
    phrase += " - ";
    phrase += this->type;
    phrase += " - ";
    phrase += this->content;

    return phrase;
}