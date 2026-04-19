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

    dbase_t* self_;

    void handsFreeAsset() {

        auto self_ = this->self();

        // Check if /tmp/dhome_wake.sock exists. 
        // If it does then remove it. 
        // rm /tmp/dhome_wake.sock

        ::unlink("/tmp/dhome_wake.sock");

        std::string cmd = "/usr/bin/python /home/drake/Documents/dHome/src/audio/wakeWord.py &";
        self_->info("Hello DragonPad");
        self_->info("Trying to launch wake word listener.");
        self_->info("cmd: " + cmd);

        //Start Wake Word Listener
        pid_t wakeWordPid_ = self_->dhome::util::systemCmd<dbase_t,dtraits_t>::run(cmd);

        // Retry connect until Python is ready
        typename dtraits_t::error e = dtraits_t::error::kError;
        for(int i = 0; i < 10 && e == dtraits_t::error::kError; ++i) {
            ::sleep(1);
            e = self_->connectSocket("/tmp/dhome_wake.sock");
        }

        if(e == dtraits_t::error::kError) {
            self_->error("Error! Couldn't connect to wake word socket!");
            return;
        }

        self_->info("Connected to wake word socket.");

        self_->dhome::audio::wakeWord<dbase_t,dtraits_t>::listen();

        self_->kill(wakeWordPid_);

        self_->info("Wake word detected!");
        self_->say("Hello there.");

        sleep(1);

        std::string transcript;

        self_->dhome::audio::stt<dbase_t,dtraits_t>::listen(transcript);

        self_->info("Recorded: " + transcript);

    }

    typename dtraits_t::error onMessage(const std::string& msg, const std::string& ip, uint16_t port) {
        auto self_ = this->self();
        self_->info("Received: " + msg + " from " + ip);
        return dtraits_t::error::kNoError;
    }

    void run() {

        auto self_ = this->self();

        self_->listenOnPort(dPort_, &dragonPad<dbase,dtraits>::onMessage);

        self_->waitForInput("Press any key to continue");

    }

    pid_t wakeWordPid_;
    uint16_t dPort_ = 64209; // 49152-65535

};

};

#endif