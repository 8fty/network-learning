cc=g++
.PHONY:all
all:calserver calclient

calclient:calClient.cc
	$(cc) -o $@ $^ -std=c++11 -ljsoncpp
calserver:calServer.cc
	$(cc) -o $@ $^ -std=c++11 -ljsoncpp
.PHONY:clean
clean:
	rm -f calclient calserver