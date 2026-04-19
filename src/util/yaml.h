#ifndef __DHomeYaml__
#define __DHomeYaml__

namespace dhome::util {

template<typename dbase, typename dtraits>
class yaml : public dhome::util::mixinBase<yaml<dbase,dtraits>>  {
    public: 
    using typeTag       = Util;
    using type          = yaml;
    using dbase_t       = dbase;
    using dtraits_t     = dtraits;

    yaml() {}
    ~yaml() {}

    template<typename ValueT>
    typename dtraits_t::error readConfig(const std::string& key, const std::string& pathToConfig, ValueT& val) {
        try {
            auto node = YAML::LoadFile(pathToConfig);
            val = node[key].as<ValueT>();   
            return dtraits_t::error::kNoError;
        } catch(const YAML::Exception& e) { 
            return dtraits_t::error::kError;
        }
    }

};

}

#endif