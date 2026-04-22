#ifndef __DHomeXLibWindow__
#define __DHomeXLibWindow__

namespace dhome::gui {

template<typename dbase, typename dtraits>
class xlibWindow : public dhome::util::mixinBase<xlibWindow<dbase,dtraits>> {
public:
    using typeTag   = Gui;
    using type      = xlibWindow;
    using dbase_t   = dbase;
    using dtraits_t = dtraits;

    typename dtraits_t::error openWindow() {
        auto self_ = this->self();

        self_->template info<dhome::gui::Gui>("Starting GUI");

        Display* dpy = XOpenDisplay(nullptr);
        if (!dpy) {
            self_->template error<dhome::gui::Gui>("Can't create display pointer.");
            return dtraits_t::error::kError;            
        }

        int screen = DefaultScreen(dpy);
        Window root = RootWindow(dpy, screen);

        Window win = XCreateSimpleWindow(
            dpy, root,
            100, 100,        // x, y
            800, 600,        // w, h
            1,               // border width
            BlackPixel(dpy, screen),   // border color
            BlackPixel(dpy, screen)    // background
        );

        // Select events you care about
        XSelectInput(dpy, win, ExposureMask | KeyPressMask | ButtonPressMask |
                            StructureNotifyMask);

        // Hook WM_DELETE_WINDOW so closing the window is catchable
        Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(dpy, win, &wm_delete, 1);

        XStoreName(dpy, win, "xlib window");
        XMapWindow(dpy, win);

        GC gc = XCreateGC(dpy, win, 0, nullptr);


        XftFont* font = XftFontOpenName(dpy, screen, "monospace:size=24");
        XftDraw* draw = XftDrawCreate(dpy, win, DefaultVisual(dpy, screen),
                                    DefaultColormap(dpy, screen));

        XftColor color;
        XRenderColor rc = {0xffff, 0, 0, 0xffff}; // R, G, B, A — 16-bit each
        XftColorAllocValue(dpy, DefaultVisual(dpy, screen),
                        DefaultColormap(dpy, screen), &rc, &color);

        XEvent ev;
        bool running = true;

        while (running) {
            XNextEvent(dpy, &ev);
            switch (ev.type) {
                case Expose:
                    if (ev.xexpose.count == 0) {
                        // Draw something
                        XSetForeground(dpy, gc, BlackPixel(dpy, screen));
                        // XDrawString(dpy, win, gc, 20, 30, "Hello, Xlib", 11);
                        XftDrawString8(draw, &color, font, 75, 75, (const FcChar8*)"Hello", 5);
                        XDrawLine(dpy, win, gc, 0, 0, 800, 600);
                    }
                    break;

                case KeyPress: {
                    char buf[8];
                    KeySym ks;
                    XLookupString(&ev.xkey, buf, sizeof(buf), &ks, nullptr);
                    if (ks == XK_Escape || ks == XK_q) running = false;
                    break;
                }

                case ClientMessage:
                    if ((Atom)ev.xclient.data.l[0] == wm_delete)
                        running = false;
                    break;

                case ConfigureNotify:
                    // ev.xconfigure.width / height = new size
                    break;
            }
        }

        // cleanup
        XftColorFree(dpy, DefaultVisual(dpy, screen),
                    DefaultColormap(dpy, screen), &color);
        XftDrawDestroy(draw);
        XftFontClose(dpy, font);


        XFreeGC(dpy, gc);
        XDestroyWindow(dpy, win);
        XCloseDisplay(dpy);

        return dtraits_t::error::kNoError;
    }


};

} // namespace dhome::util
#endif