CXXFLAGS = -O2 -g -I ../.. -pthread
LIB_SRC = ../../net/*.cc
BASE_SRC = ../../base/*.cc 
BINARIES = client server

all: $(BINARIES)

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) -lpthread

client: client.cc

server: server.cc

clean:
	rm -f $(BINARIES) core