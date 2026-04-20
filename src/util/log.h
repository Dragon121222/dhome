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
        std::cout << timestamp_() << " [INFO]  " << prefix << msg << postfix;
    }

    void warn(std::string msg, std::string prefix = "", std::string postfix = "\n") {
        std::cout << timestamp_() << " [WARN]  " << prefix << msg << postfix;
    }

    void error(std::string msg, std::string prefix = "", std::string postfix = "\n") {
        std::cout << timestamp_() << " [ERROR] " << prefix << msg << postfix;
    }

    private:
    std::string timestamp_() {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        std::ostringstream ss;
        ss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

};

};

#endif