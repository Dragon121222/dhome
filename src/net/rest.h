#ifndef __DHomeRest__
#define __DHomeRest__

namespace dhome::net {

template<
    typename dbase,
    typename dtraits
>
class rest : public dhome::util::mixinBase<rest<dbase,dtraits>> {
public: 
    using typeTag = Net;
    using type    = rest;
    using dbase_t = dbase;
    using dtraits_t = dtraits;
    
    rest() {}
    ~rest() {}

};

}

#endif