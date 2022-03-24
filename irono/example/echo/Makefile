CXXFLAGS = -O2 -g -I ../.. -pthread
LIB_SRC = ../../net/*.cc echo.cc
BASE_SRC = ../../base/*.cc 
BINARIES = main

all: $(BINARIES)

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) -lpthread

main: main.cc

clean:
	rm -f $(BINARIES) core *.log