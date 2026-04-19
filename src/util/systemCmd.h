// #ifndef __DHomeSystem__
// #define __DHomeSystem__
// #include <cstdlib>

// namespace dhome::util {

// template<typename dbase, typename dtraits>
// class systemCmd : public dhome::util::mixinBase<systemCmd<dbase,dtraits>> {
// public:
//     using typeTag       = Util;
//     using type          = systemCmd;
//     using dbase_t       = dbase;
//     using dtraits_t     = dtraits;

//     typename dtraits_t::error run(const char* cmd) {
//         int ret = ::system(cmd);
//         if(ret != 0) return dtraits_t::error::kError;
//         return dtraits_t::error::kNoError;
//     }

//     typename dtraits_t::error run(const std::string& cmd) {
//         return run(cmd.c_str());
//     }
// };

// } // namespace dhome::util
// #endif

#ifndef __DHomeSystem__
#define __DHomeSystem__
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
namespace dhome::util {
template<typename dbase, typename dtraits>
class systemCmd : public dhome::util::mixinBase<systemCmd<dbase,dtraits>> {
public:
    using typeTag   = Util;
    using type      = systemCmd;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    typename dtraits_t::error run(const std::string& cmd) {
        int ret = ::system(cmd.c_str());
        if(ret != 0) return dtraits_t::error::kError;
        return dtraits_t::error::kNoError;
    }

    pid_t launch(const std::string& cmd) {
        pid_t pid = ::fork();
        if(pid == 0) {
            ::setpgid(0, 0); // new process group
            ::execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
            ::_exit(1);
        }
        ::setpgid(pid, pid); // set from parent too (race prevention)
        return pid;
    }

    typename dtraits_t::error kill(pid_t pid) {
        struct sigaction sa{}, old{};
        sa.sa_handler = SIG_IGN;
        ::sigaction(SIGTERM, &sa, &old);
        ::killpg(pid, SIGTERM);
        ::waitpid(-pid, nullptr, 0);
        ::sigaction(SIGTERM, &old, nullptr); // restore
        return dtraits_t::error::kNoError;
    }

    typename dtraits_t::error killHard(pid_t pid) {
        if(::killpg(pid, SIGKILL) != 0) return dtraits_t::error::kError;
        ::waitpid(-pid, nullptr, 0);
        return dtraits_t::error::kNoError;
    }
};
} // namespace dhome::util
#endif