#ifndef __DHomeStt__
#define __DHomeStt__

namespace dhome::audio {

template<typename dbase, typename dtraits>
class stt : public dhome::util::mixinBase<stt<dbase,dtraits>> {
public:
    using typeTag   = Audio;
    using type      = stt;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    stt() {}
    ~stt() {}

    typename dtraits_t::error listen(std::string& transcript) {
        auto self_ = this->self();
        // Record 5 seconds of audio to wav
        std::string recordCmd = "arecord -r 16000 -f S16_LE -c 1 -d 5 /tmp/dhome_stt.wav";
        if(self_->dhome::util::systemCmd<dbase_t,dtraits_t>::run(recordCmd) != dtraits_t::error::kNoError)
            return dtraits_t::error::kError;

        // Run whisper inference
        std::string whisperCmd = "~/whisper.cpp/main -m ~/whisper.cpp/models/ggml-base.en.bin "
                                 "-f /tmp/dhome_stt.wav --no-timestamps -otxt "
                                 "-of /tmp/dhome_stt 2>/dev/null";
        if(self_->dhome::util::systemCmd<dbase_t,dtraits_t>::run(whisperCmd) != dtraits_t::error::kNoError)
            return dtraits_t::error::kError;

        // Read transcript
        std::ifstream f("/tmp/dhome_stt.txt");
        if(!f.is_open()) return dtraits_t::error::kError;
        std::getline(f, transcript);
        return dtraits_t::error::kNoError;
    }
};
}
#endif