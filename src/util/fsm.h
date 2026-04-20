#ifndef __DHomeFsm__
#define __DHomeFsm__

namespace dhome::util {

template<
    typename dbase,
    typename dtraits
>
class fsm;

template<
    typename dbase,
    typename dtraits
>
class fsm : public dhome::util::mixinBase<fsm<dbase,dtraits>> {
public: 
    using typeTag       = Util;
    using type          = fsm;
    using dbase_t       = dbase;
    using dtraits_t     = dtraits;
    using state_t       = dtraits_t::state_t;
    using stateMap_t    = dtraits_t::stateMap_t; 
    using stateFunc_t   = dtraits_t::stateFunc_t;

       

    fsm() {}
    ~fsm() {}

    void run() {

        auto self_ = this->self();

        self_->template info<typeTag>("Starting Fsm Run!");

        size_t turns = 0; 

        while(currentState_ != state_t::END) {

            self_->template info<typeTag>(std::string("Turn: ") + std::to_string(turns));

            stateFunc_t f;
            typename dtraits::error e = dhome::util::getValFromMap<typename dtraits::error>(self_->getStateMap(), currentState_, f);

            if(e == dtraits::error::kError) {
                self_->template error<typeTag>("Could not find state function associated with your currentState!");
                return;
            }

            currentState_ = (self_->*f)();

            turns++;
        }

    }

    state_t currentState_ = state_t::START;

};

};

#endif