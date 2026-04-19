#ifndef __DHomeLog__
#define __DHomeLog__

namespace dhome::util {

template<typename dbase, typename dtraits>
class log : public dhome::util::mixinBase<log<dbase,dtraits>>  {
    public: 
    using typeTag       = Util;
    using type          = log;
    using dbase_t       = dbase;
    using dtraits_t     = dtraits;

    log() {}
    ~log() {}

    void info(std::string msg, std::string prefix = "", std::string postfix = "\n") {
        std::cout << prefix << msg << postfix;
    }

    void warn(std::string msg, std::string prefix = "", std::string postfix = "\n") {
        std::cout << prefix << msg << postfix;
    }

    void error(std::string msg, std::string prefix = "", std::string postfix = "\n") {
        std::cout << prefix << msg << postfix;
    }

};

};

#endif