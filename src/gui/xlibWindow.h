#ifndef __DHomeXLibWindow__
#define __DHomeXLibWindow__

namespace dhome::gui {

// --- TextBehavior -------------------------------------------------------------

enum class TextBehavior { Wrap, Scroll };

// --- TextBox traits -----------------------------------------------------------

struct DefaultTextBoxTraits {
    struct Color { uint16_t r, g, b, a; };
    static constexpr Color        fg       = {0x0000, 0x0000, 0xffff, 0xffff};
    static constexpr Color        bg       = {0x1a1a, 0x1a1a, 0x1a1a, 0xffff};
    static constexpr Color        border   = {0x8888, 0x8888, 0x8888, 0xffff};
    static constexpr const char*  font     = "monospace:size=18";
    static constexpr int          padding  = 0;
    static constexpr TextBehavior behavior = TextBehavior::Wrap;
};

// --- Border function type -----------------------------------------------------

using BorderFn = void(*)(Display*, Drawable, GC, int, int, int, int);

inline void defaultBorder_(Display* dpy, Drawable d, GC gc,
                            int x, int y, int w, int h) {
    XDrawRectangle(dpy, d, gc, x, y, w - 1, h - 1);
}

// --- xlibWindow ---------------------------------------------------------------

template<typename dbase, typename dtraits>
class xlibWindow : public dhome::util::mixinBase<xlibWindow<dbase,dtraits>> {
public:
    using typeTag   = Gui;
    using type      = xlibWindow;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    // -------------------------------------------------------------------------
    // TextBox — templated on a traits struct that controls font/color/padding
    // -------------------------------------------------------------------------
    template<typename TBTraits = DefaultTextBoxTraits>
    struct TextBox {
        std::string    text;
        int            x = 0, y = 0;
        int            width = 0, height = 0;
        int            padding          = TBTraits::padding;
        XftFont*       font             = nullptr;
        XftColor       fg               = {};
        unsigned long  bg_pixel         = 0;
        unsigned long  border_pixel     = 0;
        unsigned long  sel_border_pixel = 0;
        BorderFn       drawBorder       = defaultBorder_;
        TextBehavior   behavior         = TBTraits::behavior;
        int            scrollLine       = 0;
        bool           selected         = false;
    };

    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------
    void addTextBox(int x, int y, int w, int h, std::string_view text,
                    TextBehavior behavior = TextBehavior::Wrap,
                    BorderFn border = defaultBorder_) {
        pendingBoxes_.push_back({std::string(text), x, y, w, h, behavior, border});
    }

    // Size specified in character rows/columns; pixel dimensions computed from font metrics.
    void addGridTextBox(int col, int row, int cols, int rows, std::string_view text,
                        TextBehavior behavior = TextBehavior::Wrap,
                        BorderFn border = defaultBorder_) {
        pendingGridBoxes_.push_back({std::string(text), col, row, cols, rows, behavior, border});
    }

    typename dtraits_t::error openWindow() {
        auto self_ = this->self();
        self_->template info<dhome::gui::Gui>("Starting GUI");

        WindowCtx ctx;
        if (initDisplay_(ctx)  != dtraits_t::error::kNoError) return dtraits_t::error::kError;
        if (initWindow_(ctx)   != dtraits_t::error::kNoError) return dtraits_t::error::kError;
        if (initGraphics_(ctx) != dtraits_t::error::kNoError) return dtraits_t::error::kError;
        runEventLoop_(ctx);
        cleanup_(ctx);
        return dtraits_t::error::kNoError;
    }

private:
    // -------------------------------------------------------------------------
    // Pending specs (queued before the window opens)
    // -------------------------------------------------------------------------
    struct TextBoxSpec {
        std::string  text;
        int          x, y, width, height;
        TextBehavior behavior   = TextBehavior::Wrap;
        BorderFn     drawBorder = defaultBorder_;
    };

    struct GridTextBoxSpec {
        std::string  text;
        int          col, row, cols, rows;
        TextBehavior behavior   = TextBehavior::Wrap;
        BorderFn     drawBorder = defaultBorder_;
    };

    std::vector<TextBoxSpec>     pendingBoxes_;
    std::vector<GridTextBoxSpec> pendingGridBoxes_;

    // -------------------------------------------------------------------------
    // Window state
    // -------------------------------------------------------------------------
    struct WindowCtx {
        Display*  dpy         = nullptr;
        int       screen      = 0;
        Window    win         = 0;
        GC        gc          = nullptr;
        XftFont*  font        = nullptr;
        XftDraw*  draw        = nullptr;
        XftColor  color       = {};
        Atom      wm_delete   = 0;
        int       width       = 800;
        int       height      = 600;
        int       selectedBox = -1;
        std::vector<TextBox<DefaultTextBoxTraits>> textBoxes;
    };

    // -------------------------------------------------------------------------
    // Init / teardown
    // -------------------------------------------------------------------------
    typename dtraits_t::error initDisplay_(WindowCtx& ctx) {
        ctx.dpy = XOpenDisplay(nullptr);
        if (!ctx.dpy) {
            this->self()->template error<dhome::gui::Gui>("Can't create display pointer.");
            return dtraits_t::error::kError;
        }
        ctx.screen = DefaultScreen(ctx.dpy);
        return dtraits_t::error::kNoError;
    }

    typename dtraits_t::error initWindow_(WindowCtx& ctx) {
        Window root = RootWindow(ctx.dpy, ctx.screen);
        ctx.win = XCreateSimpleWindow(
            ctx.dpy, root,
            100, 100, ctx.width, ctx.height,
            1,
            BlackPixel(ctx.dpy, ctx.screen),
            BlackPixel(ctx.dpy, ctx.screen)
        );
        XSelectInput(ctx.dpy, ctx.win,
            ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask);
        ctx.wm_delete = XInternAtom(ctx.dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(ctx.dpy, ctx.win, &ctx.wm_delete, 1);
        XStoreName(ctx.dpy, ctx.win, "xlib window");
        XMapWindow(ctx.dpy, ctx.win);
        return dtraits_t::error::kNoError;
    }

    typename dtraits_t::error initGraphics_(WindowCtx& ctx) {
        ctx.gc   = XCreateGC(ctx.dpy, ctx.win, 0, nullptr);
        ctx.font = XftFontOpenName(ctx.dpy, ctx.screen, "monospace:size=24");
        ctx.draw = XftDrawCreate(ctx.dpy, ctx.win,
                      DefaultVisual(ctx.dpy, ctx.screen),
                      DefaultColormap(ctx.dpy, ctx.screen));

        XRenderColor rc = {0xffff, 0, 0, 0xffff};
        XftColorAllocValue(ctx.dpy,
            DefaultVisual(ctx.dpy, ctx.screen),
            DefaultColormap(ctx.dpy, ctx.screen),
            &rc, &ctx.color);

        for (auto& spec : pendingBoxes_) {
            auto tb       = makeTextBox_(ctx, spec.x, spec.y, spec.width, spec.height, spec.text);
            tb.drawBorder = spec.drawBorder;
            tb.behavior   = spec.behavior;
            ctx.textBoxes.push_back(std::move(tb));
        }

        auto [cw, ch] = cellSize_(ctx, DefaultTextBoxTraits::font);
        for (auto& spec : pendingGridBoxes_) {
            auto tb       = makeTextBox_(ctx,
                                spec.col  * cw, spec.row  * ch,
                                spec.cols * cw, spec.rows * ch,
                                spec.text);
            tb.drawBorder = spec.drawBorder;
            tb.behavior   = spec.behavior;
            ctx.textBoxes.push_back(std::move(tb));
        }

        return dtraits_t::error::kNoError;
    }

    void cleanup_(WindowCtx& ctx) {
        for (auto& tb : ctx.textBoxes)
            freeTextBox_(ctx, tb);
        XftColorFree(ctx.dpy, DefaultVisual(ctx.dpy, ctx.screen),
                     DefaultColormap(ctx.dpy, ctx.screen), &ctx.color);
        XftDrawDestroy(ctx.draw);
        XftFontClose(ctx.dpy, ctx.font);
        XFreeGC(ctx.dpy, ctx.gc);
        XDestroyWindow(ctx.dpy, ctx.win);
        XCloseDisplay(ctx.dpy);
    }

    // -------------------------------------------------------------------------
    // TextBox helpers
    // -------------------------------------------------------------------------
    std::pair<int,int> cellSize_(WindowCtx& ctx, const char* fontName) {
        XftFont* f = XftFontOpenName(ctx.dpy, ctx.screen, fontName);
        XGlyphInfo gi;
        XftTextExtents8(ctx.dpy, f, (const FcChar8*)"W", 1, &gi);
        int cw = gi.xOff;
        int ch = f->ascent + f->descent;
        XftFontClose(ctx.dpy, f);
        return {cw, ch};
    }

    template<typename TBTraits = DefaultTextBoxTraits>
    TextBox<TBTraits> makeTextBox_(WindowCtx& ctx,
                                   int x, int y, int w, int h,
                                   std::string_view text) {
        TextBox<TBTraits> tb;
        tb.text  = std::string(text);
        tb.x = x; tb.y = y; tb.width = w; tb.height = h;

        tb.font = XftFontOpenName(ctx.dpy, ctx.screen, TBTraits::font);

        XRenderColor rc_fg = {TBTraits::fg.r, TBTraits::fg.g, TBTraits::fg.b, TBTraits::fg.a};
        XftColorAllocValue(ctx.dpy, DefaultVisual(ctx.dpy, ctx.screen),
                           DefaultColormap(ctx.dpy, ctx.screen), &rc_fg, &tb.fg);

        auto allocPixel = [&](const auto& c) -> unsigned long {
            XColor xc = {};
            xc.red = c.r; xc.green = c.g; xc.blue = c.b;
            xc.flags = DoRed | DoGreen | DoBlue;
            XAllocColor(ctx.dpy, DefaultColormap(ctx.dpy, ctx.screen), &xc);
            return xc.pixel;
        };
        tb.bg_pixel         = allocPixel(TBTraits::bg);
        tb.border_pixel     = allocPixel(TBTraits::border);
        tb.sel_border_pixel = allocPixel(TBTraits::fg);

        return tb;
    }

    // Splits text into lines that fit within (boxWidth - 2*padding), breaking at spaces.
    std::vector<std::string> wrapText_(WindowCtx& ctx, XftFont* font,
                                       int boxWidth, int padding,
                                       const std::string& text) {
        int maxW = boxWidth - 2 * padding;
        std::vector<std::string> lines;
        std::string remaining = text;
        while (!remaining.empty()) {
            int lo = 1, hi = (int)remaining.size(), best = 1;
            while (lo <= hi) {
                int mid = (lo + hi) / 2;
                XGlyphInfo gi;
                XftTextExtents8(ctx.dpy, font,
                                (const FcChar8*)remaining.c_str(), mid, &gi);
                if (gi.xOff <= maxW) { best = mid; lo = mid + 1; }
                else                 { hi   = mid - 1; }
            }
            if (best < (int)remaining.size()) {
                auto sp = remaining.rfind(' ', best);
                if (sp != std::string::npos && sp > 0) best = (int)sp + 1;
            }
            lines.push_back(remaining.substr(0, best));
            remaining = remaining.substr(best);
        }
        return lines;
    }

    template<typename TBTraits = DefaultTextBoxTraits>
    void drawTextBox_(WindowCtx& ctx, const TextBox<TBTraits>& tb) {
        XSetForeground(ctx.dpy, ctx.gc, tb.bg_pixel);
        XFillRectangle(ctx.dpy, ctx.win, ctx.gc, tb.x, tb.y, tb.width, tb.height);

        XSetForeground(ctx.dpy, ctx.gc, tb.selected ? tb.sel_border_pixel : tb.border_pixel);
        tb.drawBorder(ctx.dpy, ctx.win, ctx.gc, tb.x, tb.y, tb.width, tb.height);

        auto lines     = wrapText_(ctx, tb.font, tb.width, tb.padding, tb.text);
        int  lineH     = tb.font->ascent + tb.font->descent;
        int  startLine = (tb.behavior == TextBehavior::Scroll) ? tb.scrollLine : 0;

        int visibleLines = 0;
        for (int i = startLine; i < (int)lines.size(); ++i) {
            if (tb.padding + (visibleLines + 1) * lineH > tb.height - tb.padding) break;
            ++visibleLines;
        }
        int blockH = visibleLines * lineH;
        int y      = tb.y + (tb.height - blockH) / 2 + tb.font->ascent;

        for (int i = startLine; i < startLine + visibleLines; ++i) {
            XGlyphInfo gi;
            XftTextExtents8(ctx.dpy, tb.font,
                            (const FcChar8*)lines[i].c_str(), lines[i].size(), &gi);
            int x = tb.x + (tb.width - gi.xOff) / 2;
            XftDrawString8(ctx.draw, &tb.fg, tb.font, x, y,
                           (const FcChar8*)lines[i].c_str(), lines[i].size());
            y += lineH;
        }
    }

    template<typename TBTraits = DefaultTextBoxTraits>
    void freeTextBox_(WindowCtx& ctx, TextBox<TBTraits>& tb) {
        XftColorFree(ctx.dpy, DefaultVisual(ctx.dpy, ctx.screen),
                     DefaultColormap(ctx.dpy, ctx.screen), &tb.fg);
        if (tb.font) { XftFontClose(ctx.dpy, tb.font); tb.font = nullptr; }
    }

    // -------------------------------------------------------------------------
    // Drawing / event handling
    // -------------------------------------------------------------------------
    void draw_(WindowCtx& ctx) {
        for (auto& tb : ctx.textBoxes)
            drawTextBox_(ctx, tb);
    }

    void onResize_(WindowCtx& ctx, int w, int h) {
        ctx.width  = w;
        ctx.height = h;
    }

    bool handleKey_(WindowCtx& ctx, XKeyEvent& ev) {
        char buf[8];
        KeySym ks;
        XLookupString(&ev, buf, sizeof(buf), &ks, nullptr);

        if (ks == XK_Escape || ks == XK_q) return false;

        if (ks == XK_Tab && !ctx.textBoxes.empty()) {
            if (ctx.selectedBox >= 0)
                ctx.textBoxes[ctx.selectedBox].selected = false;
            ctx.selectedBox = (ctx.selectedBox + 1) % (int)ctx.textBoxes.size();
            ctx.textBoxes[ctx.selectedBox].selected = true;
            XClearArea(ctx.dpy, ctx.win, 0, 0, 0, 0, True);
            return true;
        }

        if (ctx.selectedBox >= 0) {
            auto& tb = ctx.textBoxes[ctx.selectedBox];
            if (tb.behavior == TextBehavior::Scroll) {
                if (ks == XK_Down) {
                    ++tb.scrollLine;
                    XClearArea(ctx.dpy, ctx.win, 0, 0, 0, 0, True);
                } else if (ks == XK_Up) {
                    tb.scrollLine = std::max(0, tb.scrollLine - 1);
                    XClearArea(ctx.dpy, ctx.win, 0, 0, 0, 0, True);
                }
            }
        }

        return true;
    }

    void runEventLoop_(WindowCtx& ctx) {
        bool running = true;
        XEvent ev;
        while (running) {
            XNextEvent(ctx.dpy, &ev);
            switch (ev.type) {
                case Expose:
                    if (ev.xexpose.count == 0) draw_(ctx);
                    break;
                case KeyPress:
                    running = handleKey_(ctx, ev.xkey);
                    break;
                case ClientMessage:
                    if ((Atom)ev.xclient.data.l[0] == ctx.wm_delete)
                        running = false;
                    break;
                case ConfigureNotify:
                    onResize_(ctx, ev.xconfigure.width, ev.xconfigure.height);
                    break;
            }
        }
    }
};

} // namespace dhome::gui
#endif
