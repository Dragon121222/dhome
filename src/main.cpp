#include "mainInclude.h"

namespace dhome {

class app;

struct app_traits {

    enum app_fsm_states {
        START,
        READ_CONFIG,
        START_DRAGON_FIRE,
        START_DRAGON_PI_5,
        START_DRAGON_PI_6,
        START_DRAGON_PAD,
        START_DRAGON_ZEPHYRUS,
        LISTEN,
        END
    };

    constexpr static const char* configFilePath_ = "/etc/dhome/dhome.conf";

    enum error {
        kNoError,
        kError
    };
    using state_t           = app_fsm_states;
    using stateFunc_t       = state_t (app::*)(); // still needs app forward-decl
    using stateMap_t        = std::map<app_fsm_states, stateFunc_t>;
    using deviceNameMap_t   = std::map<std::string, state_t>;
    using error_t           = error;
};

using app_t = dhome::util::mixin<
    dhome::util::fsm<app,app_traits>,
    dhome::util::terminalInput<app,app_traits>,
    dhome::util::tui<app,app_traits>,
    // dhome::util::log<app,app_traits>,
    dhome::util::yaml<app,app_traits>,
    dhome::util::systemCmd<app,app_traits>,
    dhome::net::unixSocket<app,app_traits>,
    dhome::net::tcp<app,app_traits>,
    dhome::net::ip<app,app_traits>,
    dhome::device::dragonPad<app,app_traits>,
    dhome::audio::wakeWord<app,app_traits>,
    dhome::audio::tts<app,app_traits>,
    dhome::audio::stt<app,app_traits>
>;

class app : public app_t {
public: 
    using state_t         = app_traits::app_fsm_states;
    using stateMap_t      = app_traits::stateMap_t;
    using deviceNameMap_t = app_traits::deviceNameMap_t;
    using error_t         = app_traits::error;
app() {}
~app() {}

const stateMap_t stateMap_ = {
    {state_t::START,            &app::start_},
    {state_t::READ_CONFIG,      &app::readConfig_},
    {state_t::START_DRAGON_PAD, &app::runDragonPad_}
};

const deviceNameMap_t deviceNameMap_ = {
    {"dragonFire",      state_t::START_DRAGON_FIRE},
    {"dragonPi5",       state_t::START_DRAGON_PI_5},
    {"dragonPi6",       state_t::START_DRAGON_PI_6},
    {"dragonPad",       state_t::START_DRAGON_PAD},
    {"dragonZephyrus",  state_t::START_DRAGON_ZEPHYRUS}
};

state_t start_() {
    info("Starting App");
    return state_t::READ_CONFIG;
}

state_t readConfig_() {
    info("Reading Config File:");
    
    auto e = readConfig<std::string>("systemName", app_traits::configFilePath_, systemName_);
    if(e == error_t::kError) {
        error("Could not read systemName from config file!");
        return state_t::END;
    }

    info("System Name: " + systemName_);
    state_t nextState; 
    e = dhome::util::getValFromMap<error_t>(deviceNameMap_, systemName_, nextState);
    if(e == error_t::kError) {
        error("Could not get next state from systemName: " + systemName_ + "!");
        return state_t::END;
    }

    info("Next State Found!");
    return nextState;
}

state_t runDragonFire_() {
    info("Starting Dragon Fire App");

    return state_t::END;
}

state_t runDragonPad_() {
    info("Starting Dragon Pad App");

    this->as<dhome::device::Device>::run();

    return state_t::END;
}



const stateMap_t& getStateMap() {
    return stateMap_;
}
    std::string systemName_;
};



}

int main() {

    dhome::app x;

    x.as<dhome::util::Util>::run();


    return 0;
}