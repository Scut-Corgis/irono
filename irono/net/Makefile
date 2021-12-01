# vpath %.cc ../base
LIB_SRC = EventLoop.cc Poller.cc Channel.cc Timer.cc TimerQueue.cc EventLoopThread.cc Socket.cc \
	SocketsOps.cc Acceptor.cc InetAddress.cc TcpConnection.cc TcpServer.cc \
	Buffer.cc
BASE_SRC = ../base/AsyncLogging.cc ../base/CountDownLatch.cc ../base/FileUtil.cc ../base/LogFile.cc ../base/Logging.cc ../base/LogStream.cc ../base/Thread.cc ../base/Timestamp.cc  
BINARIES = test1

all: $(BINARIES)

$(BINARIES):
	g++ -g -o test1 $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) -lpthread

test1: test1.cc

clean:
	rm -f $(BINARIES) core