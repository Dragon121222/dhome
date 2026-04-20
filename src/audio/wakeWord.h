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

    typename dtraits::state_t listen() {
        auto self_ = this->self();

        uint8_t byte;
        auto e = self_->read(byte);  // blocks until wake word
        if(e == dtraits::error_t::kError) {
            self_->template error<typeTag>("Socket read error!");
            return dtraits::state_t::END;
        }

        return dtraits::state_t::END;  // loop back
    }


    
};

}

#endif