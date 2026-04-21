#ifndef __DHomeSocket__
#define __DHomeSocket__


namespace dhome::net {

template<typename dbase, typename dtraits>
class unixSocket : public dhome::util::mixinBase<unixSocket<dbase,dtraits>> {
public:
    using typeTag = Net;
    using type    = unixSocket;
    using dbase_t = dbase;
    using dtraits_t = dtraits;

    unixSocket() {}
    ~unixSocket() { if(fd_ >= 0) ::close(fd_); }

    typename dtraits_t::error connectSocket(const char* path) {
        fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if(fd_ < 0) return dtraits_t::error::kError;

        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        ::strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

        if(::connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(fd_);
            fd_ = -1;
            return dtraits_t::error::kError;
        }

        return dtraits_t::error::kNoError;
    }

    typename dtraits_t::error read(uint8_t& byte) {
        if(::recv(fd_, &byte, 1, 0) <= 0) return dtraits_t::error::kError;
        return dtraits_t::error::kNoError;
    }

private:
    int fd_ = -1;
};

} // namespace dhome::util
#endif