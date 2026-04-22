#ifndef __DHomeSystem__
#define __DHomeSystem__

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
            ::setpgid(0, 0);
            int devnull = ::open("/dev/null", O_WRONLY);
            ::dup2(devnull, STDOUT_FILENO);
            ::dup2(devnull, STDERR_FILENO);
            ::close(devnull);
            ::setsid();
            ::execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);
            ::_exit(1);
        }
        ::setpgid(pid, pid);
        return pid;
    }

    typename dtraits_t::error capture(const std::string& cmd, std::string& output) {
        FILE* pipe = ::popen(cmd.c_str(), "r");
        if(!pipe) return dtraits_t::error::kError;
        char buf[256];
        while(::fgets(buf, sizeof(buf), pipe))
            output += buf;
        ::pclose(pipe);
        return dtraits_t::error::kNoError;
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