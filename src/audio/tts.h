#ifndef __DHomeTts__
#define __DHomeTts__

namespace dhome::audio {
template<typename dbase, typename dtraits>
class tts : public dhome::util::mixinBase<tts<dbase,dtraits>> {
public:
    using typeTag   = Audio;
    using type      = tts;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    tts() {}
    ~tts() {}

    typename dtraits_t::error say(const std::string& text) {
        auto self_ = this->self();
        std::string escaped = text;
        size_t pos = 0;
        while((pos = escaped.find('"', pos)) != std::string::npos) {
            escaped.replace(pos, 1, "\\\"");
            pos += 2;
        }
        std::string cmd = "echo \"" + escaped + "\" | "
                        "piper-tts --model /home/drake/.local/share/piper/en_GB-northern_english_male-medium.onnx "
                        "--output-raw | "
                        "sox -r 22050 -e signed -b 16 -c 1 -t raw - -t raw - pitch -50 reverb 10 | "
                        "aplay -r 22050 -f S16_LE -t raw - &";
        return self_->dhome::util::systemCmd<dbase_t,dtraits_t>::run(cmd);
    }

};
}
#endif

