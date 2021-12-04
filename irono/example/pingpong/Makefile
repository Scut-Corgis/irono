# vpath %.cc ../base
CXXFLAGS = -O2 -g -I ../.. -pthread
LIB_SRC = ../../net/EventLoop.cc ../../net/Channel.cc ../../net/Timer.cc \
	../../net/TimerQueue.cc ../../net/EventLoopThread.cc ../../net/Socket.cc \
	../../net/SocketsOps.cc ../../net/Acceptor.cc ../../net/InetAddress.cc\
	../../net/TcpConnection.cc ../../net/TcpServer.cc \
	../../net/Buffer.cc ../../net/EventLoopThreadPool.cc ../../net/Connector.cc \
	../../net/TcpClient.cc ../../net/EPoller.cc
BASE_SRC = ../../base/AsyncLogging.cc ../../base/CountDownLatch.cc ../../base/FileUtil.cc ../../base/LogFile.cc \
	../../base/Logging.cc ../../base/LogStream.cc ../../base/Thread.cc ../../base/Timestamp.cc  
BINARIES = client server

all: $(BINARIES)

$(BINARIES):
	g++ $(CXXFLAGS) -o $@ $(LIB_SRC) $(BASE_SRC) $(filter %.cc,$^) -lpthread

#bench: bench.cc

client: client.cc

server: server.cc

clean:
	rm -f $(BINARIES) core