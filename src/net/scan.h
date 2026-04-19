#ifndef __DHomeScan__
#define __DHomeScan__

namespace dhome::net {

template<
    typename dbase,
    typename dtraits
>
class scan : public dhome::util::mixinBase<scan<dbase,dtraits>> {
public: 
    using typeTag = Net;
    using type    = scan;
    using dbase_t = dbase;
    using dtraits_t = dtraits;
    
    scan() {}
    ~scan() {}

};

}

#endif