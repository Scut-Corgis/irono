#include "../../net/TcpServer.h"
#include <unordered_set>
#include <list>

class EchoServer {
public:
    EchoServer(irono::EventLoop* loop,
                const irono::InetAddress& listenAddr,
                int idleSeconds);

    void start();

private:
    void onConnection(const irono::TcpConnectionPtr& conn);

    void onMessage(const irono::TcpConnectionPtr& conn,
                    irono::Buffer* buf,
                    irono::Timestamp time);

    void onTimer();

    void dumpConnectionBuckets();

    typedef std::weak_ptr<irono::TcpConnection> WeakTcpConnectionPtr;

    struct Entry : public irono::copyable {
        explicit Entry(const WeakTcpConnectionPtr& weakConn)
        : weakConn_(weakConn)
        {}

        ~Entry() {
        irono::TcpConnectionPtr conn = weakConn_.lock();
        if (conn) {
            conn->shutdown();
        }
        }

        WeakTcpConnectionPtr weakConn_;
    };

    // 自己设计的circular_buffer，不采用boost的库了
    template<typename T>
    class circular_buffer {
    public:
        circular_buffer(int bucketSize) : bucketSize_(bucketSize) {
            int n = bucketSize;
            while (n--) {
                buffers_.push_back(T());
            }
        }
        // 时间轮指针移动，对应list尾端桶全部析构，在前端插入新的空桶
        void Tick() {
            buffers_.pop_back();
            buffers_.push_front(T());
        }
        int size() const{
            int res = 0;
            for (auto iter = buffers_.begin(); iter != buffers_.end(); iter++) {
                res += (*iter).size();
            }
            return res;
        }
        typename std::list<T>::iterator begin() {
            return buffers_.begin();
        }
        typename  std::list<T>::iterator end() {
            return buffers_.end();
        }
    private:
        std::list<T> buffers_;
        int bucketSize_;
    };

    typedef std::shared_ptr<Entry> EntryPtr;
    typedef std::weak_ptr<Entry> WeakEntryPtr;
    typedef std::unordered_set<EntryPtr> Bucket;
    typedef circular_buffer<Bucket> WeakConnectionList;

    irono::TcpServer server_;
    WeakConnectionList connectionBuckets_;
};

