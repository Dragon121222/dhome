#ifndef __DHomeWakeWord__
#define __DHomeWakeWord__

namespace dhome::audio {

template<typename dbase, typename dtraits>
class wakeWord : public dhome::util::mixinBase<wakeWord<dbase,dtraits>> {
public: 
    using typeTag = Audio;
    using type    = wakeWord;
    using dbase_t = dbase;
    using dtraits_t = dtraits;
    
    wakeWord() {}
    ~wakeWord() {}

    typename dtraits_t::error listen() {
        auto self_ = this->self();

        uint8_t byte;
        auto e = self_->read(byte);  // blocks until wake word
        if(e == dtraits_t::error::kError) {
            self_->template error<typeTag>("Socket read error!");
            return dtraits_t::error::kError;
        }

        return dtraits_t::error::kNoError;
    }


    
};

}

#endif