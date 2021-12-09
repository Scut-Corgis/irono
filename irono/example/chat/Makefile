CXXFLAGS = -O2 -g -I ../.. -pthread
LIB_SRC = ../../net/*.cc
BASE_SRC = ../../base/*.cc 
BINARIES = client server server_threaded server_threaded_efficient

all: $(BINARIES)

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) -lpthread

client: client.cc

server: server.cc

server_threaded: server_threaded.cc

server_threaded_efficient: server_threaded_efficient.cc


clean:
	rm -f $(BINARIES) core