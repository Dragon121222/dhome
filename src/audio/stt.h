#ifndef __DHomeStt__
#define __DHomeStt__

namespace dhome::audio {

template<typename dbase, typename dtraits>
class stt : public dhome::util::mixinBase<stt<dbase,dtraits>> {
public:
    using typeTag   = Audio;
    using type      = stt;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    stt() {}
    ~stt() {}

    typename dtraits_t::error listen(std::string& transcript) {
        auto self_ = this->self();

        // trigger recording
        uint8_t trigger = 0x01;
        if(::send(sttFd_, &trigger, 1, 0) <= 0) return dtraits_t::error::kError;

        // read 4-byte length
        uint32_t len = 0;
        if(::recv(sttFd_, &len, 4, MSG_WAITALL) != 4) return dtraits_t::error::kError;

        // read transcript
        std::string buf(len, '\0');
        if(::recv(sttFd_, buf.data(), len, MSG_WAITALL) != (ssize_t)len) return dtraits_t::error::kError;
        while(!buf.empty() && (buf.back() == '\n' || buf.back() == '\r'))
            buf.pop_back();
        transcript = buf;
        return dtraits_t::error::kNoError;
    }

    int sttFd_ = -1;
};
}
#endif