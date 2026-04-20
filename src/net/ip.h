#ifndef __DHomeIp__
#define __DHomeIp__

namespace dhome::net {

struct IpEntry {
    std::string iface;
    std::string ip;
    bool isLoopback;
};

template<typename dbase, typename dtraits>
class ip : public dhome::util::mixinBase<ip<dbase,dtraits>> {
public:
    using typeTag   = Net;
    using type      = ip;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    ip() {}
    ~ip() {}

    // Returns all IPv4 addresses
    typename dtraits_t::error getIps(std::vector<IpEntry>& out) {
        ifaddrs* ifaddr = nullptr;
        if(::getifaddrs(&ifaddr) < 0) return dtraits_t::error::kError;
        for(ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if(!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
            char buf[INET_ADDRSTRLEN]{};
            auto* sin = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
            ::inet_ntop(AF_INET, &sin->sin_addr, buf, sizeof(buf));
            out.push_back({
                ifa->ifa_name,
                std::string(buf),
                static_cast<bool>(ifa->ifa_flags & IFF_LOOPBACK)
            });
        }
        ::freeifaddrs(ifaddr);
        return dtraits_t::error::kNoError;
    }

    // Returns first non-loopback IPv4 — sensible default
    std::string getPrimaryIp() {
        auto self_ = this->self();
        std::vector<IpEntry> entries;
        if(getIps(entries) != dtraits_t::error::kNoError) return "";
        for(auto& e : entries) {
            if(!e.isLoopback) {
                self_->template info<typeTag>("Ip Addr: " + e.ip);
                return e.ip;
            }
        }

        return "";
    }
};

} // namespace dhome::net
#endif