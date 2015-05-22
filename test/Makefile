# A simple Makefile to cause make to go look in the top directory. A simple
# convenience.


all: src/twoRobotWander src/clientOnPC src/serverOnHV

ifndef CXX
CXX:=g++
endif


CXXFLAGS:=-g -Wall -D_REENTRANT -fPIC -fno-exceptions 
CXXINC:=-I/usr/local/Aria/include -I/usr/local/Aria/ArNetworking/include
CXXLINK:=-L/usr/local/Aria/lib -lAria -lArNetworking -lpthread -ldl -lrt

%: %.cpp
	$(CXX) $(CXXFLAGS) $(CXXINC) $< -o $@ $(CXXLINK)	
