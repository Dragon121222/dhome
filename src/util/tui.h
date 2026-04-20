#ifndef __DHomeTui__
#define __DHomeTui__

#include <ncurses.h>
#include <mutex>
#include <deque>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace dhome::util {

template<typename dbase, typename dtraits>
class tui : public dhome::util::mixinBase<tui<dbase,dtraits>> {
public:
    using typeTag   = Util;
    using type      = tui;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    static constexpr const char* PANEL_FSM    = "FSM";
    static constexpr const char* PANEL_AUDIO  = "Audio";
    static constexpr const char* PANEL_NET    = "Net";
    static constexpr const char* PANEL_DEVICE = "Device";
    static constexpr const char* PANEL_SYSTEM = "System";

    tui() {
        initscr();
        cbreak();
        noecho();
        curs_set(0);
        start_color();
        use_default_colors();

        init_pair(1, COLOR_CYAN,    -1); // panel title
        init_pair(2, COLOR_GREEN,   -1); // INFO
        init_pair(3, COLOR_YELLOW,  -1); // WARN
        init_pair(4, COLOR_RED,     -1); // ERROR
        init_pair(5, COLOR_WHITE,   -1); // normal

        buildLayout_();

        running_ = true;
        renderThread_ = std::thread([this]() { renderLoop_(); });
    }

    ~tui() {
        running_ = false;
        if(renderThread_.joinable()) renderThread_.join();
        for(auto& [name, win] : windows_) delwin(win);
        endwin();
    }

    void log(const std::string& panel, const std::string& level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& buf = buffers_[panel];
        buf.push_back({timestamp_(), level, msg});
        if(buf.size() > 200) buf.pop_front();
        dirty_ = true;
    }

    void info(const std::string& msg,  const std::string& panel = PANEL_SYSTEM) { log(panel, "INFO",  msg); }
    void warn(const std::string& msg,  const std::string& panel = PANEL_SYSTEM) { log(panel, "WARN",  msg); }
    void error(const std::string& msg, const std::string& panel = PANEL_SYSTEM) { log(panel, "ERROR", msg); }

private:
    struct LogEntry {
        std::string time;
        std::string level;
        std::string msg;
    };

    std::map<std::string, WINDOW*>              windows_;
    std::map<std::string, std::deque<LogEntry>> buffers_;
    std::mutex                                  mutex_;
    std::thread                                 renderThread_;
    std::atomic<bool>                           running_{false};
    std::atomic<bool>                           dirty_{false};

    void buildLayout_() {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        int halfRow = rows / 2;
        int halfCol = cols / 2;

        // top-left, top-right, bottom-left, bottom-right, bottom-full
        windows_[PANEL_FSM]    = newwin(halfRow,         halfCol,         0,       0);
        windows_[PANEL_AUDIO]  = newwin(halfRow,         cols - halfCol,  0,       halfCol);
        windows_[PANEL_NET]    = newwin(halfRow - 3,     halfCol,         halfRow, 0);
        windows_[PANEL_DEVICE] = newwin(halfRow - 3,     cols - halfCol,  halfRow, halfCol);
        windows_[PANEL_SYSTEM] = newwin(3,               cols,            rows-3,  0);

        for(auto& [name, _] : windows_)
            buffers_[name] = {};
    }

    void drawPanel_(const std::string& name, WINDOW* win) {
        int rows, cols;
        getmaxyx(win, rows, cols);
        werase(win);

        // border + title
        wattron(win, COLOR_PAIR(1));
        box(win, 0, 0);
        mvwprintw(win, 0, 2, " %s ", name.c_str());
        wattroff(win, COLOR_PAIR(1));

        auto& buf = buffers_[name];
        int maxLines = rows - 2;
        int start = (int)buf.size() > maxLines ? (int)buf.size() - maxLines : 0;

        int y = 1;
        for(int i = start; i < (int)buf.size(); ++i, ++y) {
            auto& e = buf[i];
            int pair = 5;
            if(e.level == "INFO")  pair = 2;
            if(e.level == "WARN")  pair = 3;
            if(e.level == "ERROR") pair = 4;

            std::string line = e.time + " " + e.msg;
            if((int)line.size() > cols - 2)
                line = line.substr(0, cols - 2);

            wattron(win, COLOR_PAIR(pair));
            mvwprintw(win, y, 1, "%s", line.c_str());
            wattroff(win, COLOR_PAIR(pair));
        }

        wrefresh(win);
    }

    void renderLoop_() {
        while(running_) {
            if(dirty_) {
                std::lock_guard<std::mutex> lock(mutex_);
                for(auto& [name, win] : windows_)
                    drawPanel_(name, win);
                dirty_ = false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    std::string timestamp_() {
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                       now.time_since_epoch()) % 1000;
        std::ostringstream ss;
        ss << std::put_time(std::localtime(&t), "%H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
};

} // namespace dhome::util
#endif