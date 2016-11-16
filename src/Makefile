UP_CC = g++
LIB = -lboost_system -lpthread
INC = 
CPPS = server.cpp \

main:
	${UP_CC} -Wall -g -std=c++11 ${INC} ${CPPS} -o server ${LIB}

clean:
	@rm -rf *.o *.so
