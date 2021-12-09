#include "codec.h"

#include "../../base/Logging.h"
#include "../../base/Mutex.h"
#include "../../net/EventLoopThread.h"
#include "../../net/TcpClient.h"

#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace irono;
using namespace std;

class ChatClient : noncopyable {
public:
    ChatClient(EventLoop* loop, const InetAddress& serverAddr)
        : client_(loop, serverAddr),
          codec_(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3))
    {
        client_.setConnectionCallback( bind(&ChatClient::onConnection, this, _1));
        client_.setMessageCallback( bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
        client_.enableRetry();
    }

    void connect() {
        client_.connect();
    }

    void disconnect() {
        client_.disconnect();
    }

    void write(const string& message) {
        MutexLockGuard lock(mutex_);
        if (connection_) {
            codec_.send(connection_, message);
        }
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        LOG_INFO << conn->localAddress().toHostPort() << " -> "
                << conn->peerAddress().toHostPort() << " is "
                << (conn->connected() ? "UP" : "DOWN");

        MutexLockGuard lock(mutex_);
        if (conn->connected()) {
            connection_ = conn;
        }
        else {
            connection_.reset();
        }
    }
    //printf()线程安全
    void onStringMessage(const TcpConnectionPtr&,
                        const string& message,
                        Timestamp)
    {
        printf("<<< %s\n", message.c_str());
    }

    TcpClient client_;
    LengthHeaderCodec codec_;
    MutexLock mutex_;
    TcpConnectionPtr connection_ ;
};

int main(int argc, char* argv[]) {
    LOG_INFO << "pid = " << getpid();
    if (argc > 2) {
        EventLoopThread loopThread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress serverAddr(argv[1], port);

        ChatClient client(loopThread.startLoop(), serverAddr);
        client.connect();
        std::string line;
        while (std::getline(std::cin, line)) {
            client.write(line);
        }
        client.disconnect();
        sleep(3);  // wait for disconnect, see ace/logging/client.cc
    }
    else {
        printf("Usage: %s host_ip port\n", argv[0]);
    }
}

