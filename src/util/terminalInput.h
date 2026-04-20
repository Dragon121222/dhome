#ifndef __DHomeTerminalInput__
#define __DHomeTerminalInput__
#include <iostream>
#include <string>

namespace dhome::util {

template<typename dbase, typename dtraits>
class terminalInput : public dhome::util::mixinBase<terminalInput<dbase,dtraits>> {
public:
    using typeTag   = Util;
    using type      = terminalInput;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    terminalInput() {}
    ~terminalInput() {}

    std::string waitForInput(const std::string& prompt = "") {
        auto self_ = this->self();
        if(!prompt.empty())
            self_->template info<typeTag>(prompt);
        std::string in;
        std::getline(std::cin, in);
        return in;
    }
};

} // namespace dhome::util
#endif