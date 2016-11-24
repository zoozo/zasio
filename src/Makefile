UP_CC = g++
LIB = -lboost_system -lboost_thread -lpthread
INC = 

all:    server client
server:
	${UP_CC} -Wall -g -O2 -std=c++11 ${INC} $@.cpp -o $@ ${LIB}
client:
	${UP_CC} -Wall -g -O2 -std=c++11 ${INC} $@.cpp -o $@ ${LIB}

clean:
	@rm -rf *.o *.so
