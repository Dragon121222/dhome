#ifndef __DHomeUdp__
#define __DHomeUdp__

namespace dhome::net {

template<
    typename dbase,
    typename dtraits
>
class udp : public dhome::util::mixinBase<udp<dbase,dtraits>> {
public: 
    using typeTag = Net;
    using type    = udp;
    using dbase_t = dbase;
    using dtraits_t = dtraits;
    
    udp() {}
    ~udp() {}

};

}

#endif