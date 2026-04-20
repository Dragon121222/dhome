#ifndef __DHomeDragonPad__
#define __DHomeDragonPad__

namespace dhome::device {

template<
    typename dbase,
    typename dtraits
>
class dragonPad : public dhome::util::mixinBase<dragonPad<dbase,dtraits>> {
public: 
    using typeTag = Device;
    using type    = dragonPad;
    using dbase_t = dbase;
    using dtraits_t = dtraits;


    dragonPad() {}
    ~dragonPad() {}

    void listenForWakeWord() {
        auto self_ = this->self();

        // --- Wake Word ---
        ::unlink("/tmp/dhome_wake.sock");
        std::string wakeCmd = "/usr/bin/python /home/drake/Documents/dHome/src/audio/wakeWord.py >/dev/null 2>&1";
        // std::string wakeCmd = "/usr/bin/python /home/drake/Documents/dHome/src/audio/wakeWord.py";
        self_->template info<dhome::audio::Audio>("Launching wake word listener.");
        pid_t wakeWordPid_ = self_->dhome::util::systemCmd<dbase_t,dtraits_t>::launch(wakeCmd);

        typename dtraits_t::error e = dtraits_t::error::kError;
        for(int i = 0; i < 10 && e == dtraits_t::error::kError; ++i) {
            ::sleep(1);
            e = self_->connectSocket("/tmp/dhome_wake.sock");
        }
        if(e == dtraits_t::error::kError) {
            self_->template error<dhome::audio::Audio>("Couldn't connect to wake word socket!");
            return;
        }
        self_->template info<dhome::audio::Audio>("Connected to wake word socket.");

        // --- Main Loop ---
        self_->template info<dhome::audio::Audio>("Waiting for wake word...");
        auto state = self_->dhome::audio::wakeWord<dbase_t,dtraits_t>::listen();

        self_->dhome::util::systemCmd<dbase_t,dtraits_t>::kill(wakeWordPid_);

        self_->template info<dhome::audio::Audio>("Wake word detected!");

    }

    void listenForTts() {
        auto self_ = this->self();

        // --- STT ---
        ::unlink("/tmp/dhome_stt.sock");
        // std::string sttCmd = "/usr/bin/python /home/drake/Documents/dHome/src/audio/stt.py";
        std::string sttCmd  = "/usr/bin/python /home/drake/Documents/dHome/src/audio/stt.py >/dev/null 2>&1";
        self_->template info<dhome::audio::Audio>("Launching STT listener.");
        pid_t sttPid_ = self_->dhome::util::systemCmd<dbase_t,dtraits_t>::launch(sttCmd);

        typename dtraits_t::error e = dtraits_t::error::kError;
        for(int i = 0; i < 10 && e == dtraits_t::error::kError; ++i) {
            ::sleep(1);
            e = connectStt("/tmp/dhome_stt.sock");
        }
        if(e == dtraits_t::error::kError) {
            self_->template error<dhome::audio::Audio>("Couldn't connect to STT socket!");
            self_->dhome::util::systemCmd<dbase_t,dtraits_t>::kill(sttPid_);
            return;
        }
        self_->template info<dhome::audio::Audio>("Connected to STT socket.");

        // --- Main Loop ---
        std::string transcript;
        self_->template info<dhome::audio::Audio>("Listening for command...");
        if(self_->dhome::audio::stt<dbase_t,dtraits_t>::listen(transcript) != dtraits_t::error::kNoError) {
            self_->template error<dhome::audio::Audio>("STT failed.");
            return;
        }
        self_->template info<dhome::audio::Audio>("Recorded: " + transcript);

        self_->dhome::util::systemCmd<dbase_t,dtraits_t>::kill(sttPid_);
    }


    typename dtraits_t::error connectStt(const char* path) {
        sttFd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if(sttFd_ < 0) return dtraits_t::error::kError;
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        ::strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
        if(::connect(sttFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
            ::close(sttFd_);
            sttFd_ = -1;
            return dtraits_t::error::kError;
        }
        return dtraits_t::error::kNoError;
    }

    typename dtraits_t::error onMessage(const std::string& msg, const std::string& ip, uint16_t port) {
        auto self_ = this->self();
        self_->template info<dhome::net::Net>("Received: " + msg + " from " + ip);
        return dtraits_t::error::kNoError;
    }

    void run() {
        auto self_ = this->self();

        if(self_->portInUse(dPort_)) {
            self_->template error<dhome::net::Net>("Port: " + std::to_string(dPort_) + " is being used!");
        }

        typename dtraits_t::error e1 = self_->listenOnPort(dPort_, &dragonPad<dbase,dtraits>::onMessage);
        if(e1 == dtraits_t::error::kError) {
            self_->template info<dhome::net::Net>("Could not listen on port.");
        }

        ipAddr_ = self_->getPrimaryIp();

        sleep(1);

        typename dtraits_t::error e2 = self_->trySend("Hello There", ipAddr_, dPort_);
        if(e2 == dtraits_t::error::kError) {
            self_->template info<dhome::net::Net>("Couldn't Send Message!");
        }

        listenForWakeWord();

        self_->say("Yes?");
        sleep(10);
        listenForTts();
    }

    int sttFd_ = -1;
    uint16_t dPort_ = 64209;
    std::string ipAddr_;
};

}

#endif