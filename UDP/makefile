CC=g++
.PHONY:all clean
all:udpClient udpServer
udpClient:udpClient.cc
	$(CC) -o $@ $^ -lpthread -std=c++11
udpServer:udpServer.cc
	$(CC) -o $@ $^ -std=c++11
clean:
	rm -f udpClient udpServer
