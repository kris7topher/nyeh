#pragma once

#define NOALIGN __attribute__((packed))

typedef std::tr1::shared_ptr<std::string> Char_;
class Packet {
public:
    virtual ~Packet() {}

    virtual int pid() const = 0;
    virtual Char_ write() const = 0;
};
typedef std::tr1::shared_ptr<Packet> Packet_;

class PacketReader {
public:
    PacketReader(int id);
    virtual ~PacketReader() {}

    virtual Packet_ read(const std::string & ptr) = 0;

    static PacketReader& get(int id);

private:
    static std::map<int, PacketReader *>& readers();
};

#define NET_READER(klass, pid)                  \
    class klass##Reader : public PacketReader { \
    public:                                     \
        klass##Reader() : PacketReader(pid) {}  \
        Packet_ read(const std::string & ptr) { \
            return Packet_(new klass(ptr));     \
        }                                       \
    };                                          \
                                                \
    static klass##Reader __st_##klass##reader

class NetSocket;
typedef std::tr1::shared_ptr<NetSocket> NetSocket_;

class NetServer : boost::noncopyable {
public:
    NetServer(int port);
    ~NetServer();

    NetSocket_ accept() const;

private:
    int sock;
};


class NetSocket : boost::noncopyable {
public:
    NetSocket(const std::string & host, int port);
    ~NetSocket();

    Packet_ recv();
    void send(const Packet& pak);
    bool isValid() const { return sock; }

    friend NetSocket_ NetServer::accept() const;
private:
    NetSocket(int fd);

    int sock;

    char * buf;
    size_t bufl, bufp, read, size;
    bool is_head;
    int id;
    uint64_t hash;
};

class NetConnection : boost::noncopyable {
public:
    NetConnection(int port); //server
    NetConnection(const std::string & host, int port); // client
    ~NetConnection();

    NetSocket_ sock() {
        if (!sock_ || !sock_->isValid()) reconnect();
        return sock_;
    }

private:
    void reconnect();

    NetSocket_ sock_;
    NetServer * srv;

    const std::string host;
    const int port;
};
