# vpath %.cc ../base
LIB_SRC = ../../net/*.cc
BASE_SRC = ../../base/*.cc 
BINARIES = test1 test2 

all: $(BINARIES)

$(BINARIES):
	g++ -g -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) -lpthread

test1: test1.cc

test2: test2.cc

//test3: test3.cc

clean:
	rm -f $(BINARIES) core irono.log