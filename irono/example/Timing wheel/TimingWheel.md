这里提供实现思路不提供具体代码，因为发现单独做成库比较棘手，因为时间轮的实现和定时器 TimeQueue 是耦合的，时间轮的间隔定时需要前者的帮助。

> 更新： 在echo例子中实现了时间轮案例。

时间轮定时是很粗略的，因此只能管理一些粗略的定时事件，最常用的是剔除空闲连接，这里提供一种irono的实现。

在具体的实现中，时间轮格子里放的不是连接，而是特制的Entry struct,每个 Entry 包含TcpConnection 的 weak_ptr. Entry 的析构函数会判断连接是否还存在，如果存在则断开连接。
```c++
typedef std::weak_ptr<TcpConnection> WeakTcpConnectionPtr;
typedef std::shared_ptr<Entry> EntryPtr;
typedef std::weak_ptr<Entry> WeakEntryPtr;

struct Entry : public copyable {
    explicit Entry(const WeakTcpConnectionPtr& weakConn)
        : weakConn_(weakConn)
    {}

    ~Entry() {
        TcpConnectionPtr conn = weakConn_.lock();
        if (conn) {
            conn->shutdown();
        }
    }
    WeakTcpConnectionPtr weakConn_;
};
```
时间轮可以用`boost::circular_buffer`实现,但我自己也用list实现了一个，其中每个Bucket元素是一个 EntryPtr的 hashset。

```c++
typedef std::unordered_set<EntryPtr> Bucket;
typedef circular_buffer<Bucket> WeakConnectionList;
```

这样每次更新连接时，就没有必要去一个一个检查格子，将原来的entry对象移动到队尾的格子了，可以直接在队尾
的hashset中插入一个EntryPtr,引用计数会帮我们管理好一切。（因此使用set可以保证一个桶不会放重复的Entry）.

假如格子是一秒移动一格，则需要向TimeQueue注册时间事件

```c++
loop_->runEvery(1.0, xx你的回调函数xx);
```

回调函数里直接析构掉当前桶就好，RAII会帮我们处理好一切。

对于`boost::circular_buffer`来说，一行即可，往队尾插入空 Bucket，该数据结构会自动析构队首的桶

对于我自己实现的，调用Tick(),会pop掉尾部，插入一个新的头部

```c++
    // my implement
    connectionBuckets_.Tick();

    ——————————————————————————
    void Tick() {
        buffers_.pop_back();
        buffers_.push_front(T());
    }
```

这里有一个难点，当来新连接时，如何找到原来连接entry的weak_ptr,我借用了`boost::any<>`，再第一次连接建立时，将此weak_ptr存入TcpConnetion内的`boost::any<>`变量中，因此就可以很快的取出这个`weak_ptr`，并插入桶的新位置.了
```c++
    //newConnection
    if (conn->connected()) {
        EntryPtr entry(new Entry(conn));
        (*connectionBuckets_.begin()).insert(entry);
        WeakEntryPtr weakEntry(entry);
        conn->setContext(weakEntry);
    }
    //newMessage
    WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
    EntryPtr entry(weakEntry.lock());
    if (entry) {
        (*connectionBuckets_.begin()).insert(entry);
    }
```

