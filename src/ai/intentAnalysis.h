#ifndef __DHomeIntentAnalysis__
#define __DHomeIntentAnalysis__

namespace dhome::ai {

template<
    typename dbase,
    typename dtraits
>
class intentAnalysis : public dhome::util::mixinBase<intentAnalysis<dbase,dtraits>> {
public: 
    using typeTag = Ai;
    using type    = intentAnalysis;
    using dbase_t = dbase;
    using dtraits_t = dtraits;

    intentAnalysis() {}
    ~intentAnalysis() {}


};

}

#endif