//该类为中间层，相当于一个解编码器，对用户隐藏了解编码过程，每次只将一条一条的信息传递给使用者
//即一条消息后调用用户string类型回调 StringMessageCallback
#pragma once
#include "../../base/Logging.h"
#include "../../base/noncopyable.h"
#include "../../net/Buffer.h"
#include "../../net/TcpConnection.h"
#include "netinet/in.h"
class LengthHeaderCodec : irono::noncopyable {
public:
    typedef std::function<void (const irono::TcpConnectionPtr&,
                                    const std::string& message,
                                    irono::Timestamp)> StringMessageCallback;

    explicit LengthHeaderCodec(const StringMessageCallback& cb)
        : messageCallback_(cb)
    {}

    //去头部长度，解码出实际信息
    void onMessage(const irono::TcpConnectionPtr& conn,
                    irono::Buffer* buf,
                    irono::Timestamp receiveTime)
    {
        // kHeaderLen == 4
        while (buf->readableBytes() >= kHeaderLen) {
            const void* data = buf->peek();
            int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
            const int32_t len = ntohl(be32);
            if (len > 65536 || len < 0) {
                LOG_ERROR << "Invalid length " << len;
                conn->shutdown();
                break;
            }
            else if (buf->readableBytes() >= len + kHeaderLen) {
                buf->retrieve(kHeaderLen);
                std::string message(buf->peek(), len);
                messageCallback_(conn, message, receiveTime);
                buf->retrieve(len);
            }
            else {
                break;
            }
        }
    }
    //发送时加上头部，即实际发送消息长度
    void send(irono::TcpConnectionPtr conn, const std::string message) {
        irono::Buffer buf;
        buf.append(message.data(), message.size());
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = htonl(len);
        buf.prepend(&be32, sizeof be32);
        conn->send(&buf);
    }

private:
    StringMessageCallback messageCallback_;
    const static size_t kHeaderLen = sizeof(int32_t);
};

