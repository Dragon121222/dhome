#ifndef __DHomeMixinBaseClass__
#define __DHomeMixinBaseClass__

namespace dhome::util {

template <typename dtypeT>
struct mixinBase {
public:

    using dtype_t    = dtypeT;

    mixinBase() {}
    ~mixinBase() {}

    template<typename Self = dtype_t>
    typename Self::dbase_t* self() {
        return static_cast<typename Self::dbase_t*>(this);
    }

};

}

#endif