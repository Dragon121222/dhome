#ifndef __DHomeMic__
#define __DHomeMic__

namespace dhome::audio {

template<typename dbase, typename dtraits>
class mic : public dhome::util::mixinBase<mic<dbase,dtraits>> {
public: 
    using typeTag = Audio;
    using type    = mic;
    using dbase_t = dbase;
    using dtraits_t = dtraits;
    
    mic() {}
    ~mic() {}

};

}

#endif