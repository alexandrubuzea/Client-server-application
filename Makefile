CC = g++
CFLAGS = -Wall -Wextra -std=c++17

build: server subscriber

server:
	$(CC) server.cpp database.cpp client.cpp message.cpp server_utils.cpp -o server $(CFLAGS)

subscriber:
	$(CC) subscriber.cpp -o subscriber $(CFLAGS)

clean:
	rm -f subscriber server
.PHONY: clean