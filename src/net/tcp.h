#ifndef __DHomeTcp__
#define __DHomeTcp__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <functional>
#include <map>
#include <string>
#include <thread>

namespace dhome::net {

enum class TcpFlags {
    KeepListening,
    ListenOnce
};

struct TcpConnection {
    int fd = -1;
    std::string ip;
    uint16_t port;
    std::string name;
};

template<typename dbase, typename dtraits>
class tcp : public dhome::util::mixinBase<tcp<dbase,dtraits>> {
public:
    using typeTag   = Net;
    using type      = tcp;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;
    using Callback = typename dtraits_t::error (dbase_t::*)(const std::string& msg, const std::string& ip, uint16_t port);

    tcp() {}
    ~tcp() {
        for(auto& [name, conn] : connections_)
            if(conn.fd >= 0) ::close(conn.fd);
    }

    // blocks up to tryCnt * sleep(1) seconds per attempt
    typename dtraits_t::error connect(const std::string& ip, uint16_t port,
                  const std::string& name = "",
                  int tryCnt = 5) {
        int fd = -1;
        for(int i = 0; i < tryCnt && fd < 0; ++i) {
            fd = ::socket(AF_INET, SOCK_STREAM, 0);
            if(fd < 0) { ::sleep(1); continue; }
            sockaddr_in addr{};
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            ::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
            if(::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                ::close(fd);
                fd = -1;
                ::sleep(1);
            }
        }
        if(fd < 0) return dtraits_t::error::kError;
        std::string key = name.empty() ? ip + ":" + std::to_string(port) : name;
        connections_[key] = { fd, ip, port, key };
        return dtraits_t::error::kNoError;
    }

    // Non-Blocking
    typename dtraits_t::error breakConnectionByName(const std::string& name) {
        auto it = connections_.find(name);
        if(it == connections_.end()) return dtraits_t::error::kError;
        ::close(it->second.fd);
        connections_.erase(it);
        return dtraits_t::error::kNoError;
    }

    // Non-Blocking
    typename dtraits_t::error breakConnection(const std::string& ip, uint16_t port) {
        return breakConnectionByName(ip + ":" + std::to_string(port));
    }

    // Non-Blocking
    typename dtraits_t::error breakConnectionByIp(const std::string& ip) {
        for(auto it = connections_.begin(); it != connections_.end(); ) {
            if(it->second.ip == ip) {
                ::close(it->second.fd);
                it = connections_.erase(it);
            } else ++it;
        }
        return dtraits_t::error::kNoError;
    }

    // Non-Blocking
    typename dtraits_t::error listenOnPort(uint16_t port, Callback callback,
                    TcpFlags flags = TcpFlags::KeepListening) {
        dbase_t* self_ = this->self();
        self_->template info<typeTag>("Listening On Port: " + std::to_string(port));
        int serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
        if(serverFd < 0) return dtraits_t::error::kError;
        int opt = 1;
        ::setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if(::bind(serverFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(serverFd);
            return dtraits_t::error::kError;
        }
        ::listen(serverFd, 10);

        auto handler = [serverFd, callback, flags, self_]() {
            do {
                sockaddr_in clientAddr{};
                socklen_t len = sizeof(clientAddr);
                int clientFd = ::accept(serverFd, reinterpret_cast<sockaddr*>(&clientAddr), &len);
                if(clientFd < 0) break;
                char buf[4096]{};
                ssize_t n = ::recv(clientFd, buf, sizeof(buf) - 1, 0);
                if(n > 0) {
                    std::string ip = ::inet_ntoa(clientAddr.sin_addr);
                    uint16_t port  = ntohs(clientAddr.sin_port);
                    (self_->*callback)(std::string(buf, n), ip, port);
                }
                ::close(clientFd);
            } while(flags == TcpFlags::KeepListening);
            ::close(serverFd);
        };
        std::thread(handler).detach();
        return dtraits_t::error::kNoError;
    }

    // blocks up to tryCnt * sleep(1) on retry
    typename dtraits_t::error trySend(const std::string& msg, const std::string& ip, uint16_t port, int tryCnt = 5) {
        std::string key = ip + ":" + std::to_string(port);
        auto it = connections_.find(key);
        if(it == connections_.end()) {
            if(connect(ip, port, key, tryCnt) != dtraits_t::error::kNoError)
                return dtraits_t::error::kError;
            it = connections_.find(key);
        }
        for(int i = 0; i < tryCnt; ++i) {
            ssize_t n = ::send(it->second.fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
            if(n > 0) return dtraits_t::error::kNoError;
            ::sleep(1);
        }
        return dtraits_t::error::kError;
    }

    // blocks up to tryCnt * sleep(1) on retry
    typename dtraits_t::error trySend(const std::string& msg, const std::string& connectionName, int tryCnt = 5) {
        auto it = connections_.find(connectionName);
        if(it == connections_.end()) return dtraits_t::error::kError;
        for(int i = 0; i < tryCnt; ++i) {
            ssize_t n = ::send(it->second.fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
            if(n > 0) return dtraits_t::error::kNoError;
            ::sleep(1);
        }
        return dtraits_t::error::kError;
    }

    static bool portInUse(uint16_t port) {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if(fd < 0) return false;
        int opt = 1;
        ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        bool inUse = ::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0;
        ::close(fd);
        return inUse;
    }

private:
    std::map<std::string, TcpConnection> connections_;
};

} // namespace dhome::net
#endif