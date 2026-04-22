#ifndef __DHomeClaude__
#define __DHomeClaude__

namespace dhome::ai {

template<typename dbase, typename dtraits>
class claude : public dhome::util::mixinBase<claude<dbase,dtraits>> {
public:
    using typeTag   = Ai;
    using type      = claude;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    claude() {}
    ~claude() {}

    typename dtraits_t::error ask(const std::string& prompt, std::string& response) {
        auto self_ = this->self();

        std::string escaped = prompt;
        size_t pos = 0;
        while((pos = escaped.find('\'', pos)) != std::string::npos) {
            escaped.replace(pos, 1, "'\\''");
            pos += 4;
        }

        std::string cmd = "claude -p '" + escaped + "' 2>/dev/null";
        auto e = self_->dhome::util::systemCmd<dbase_t,dtraits_t>::capture(cmd, response);
        if(e != dtraits_t::error::kNoError) {
            self_->template error<typeTag>("Claude invocation failed.");
            return dtraits_t::error::kError;
        }

        while(!response.empty() && (response.back() == '\n' || response.back() == '\r'))
            response.pop_back();

        self_->template info<typeTag>("Claude: " + response);
        return dtraits_t::error::kNoError;
    }
};

}

#endif
