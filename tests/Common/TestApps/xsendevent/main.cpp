#include <QApplication>
#include <QX11Info>

#include <stdio.h>
#include <X11/Xlib.h>

void printUsage()
{
    printf("Usage: xsendevent [event windowid]...\n");
    printf("Supported events:\n");
    printf("   close    - sends _NET_CLOSE_WINDOW(windowid) the root window\n");
}

void sendCloseEvent(Window window)
{
    Display *display = QX11Info::display();

    Window rootWin = QX11Info::appRootWindow(QX11Info::appScreen());
    
    XEvent ev;
    memset(&ev, 0, sizeof(ev));

    Atom closeWindowAtom = XInternAtom(display, "_NET_CLOSE_WINDOW", False);

    ev.xclient.type         = ClientMessage;
    ev.xclient.window       = window;
    ev.xclient.message_type = closeWindowAtom;
    ev.xclient.format       = 32;
    ev.xclient.data.l[0]    = CurrentTime;
    ev.xclient.data.l[1]    = rootWin;
    XSendEvent(display, rootWin, False, SubstructureRedirectMask, &ev);
}

int main (int argc, char **argv)
{
    QApplication app(argc, argv); // connect to the X server
    
    for (int i = 1; i < argc; i++)
    {
        if (QString(argv[i]) == "-h" ||
            QString(argv[i]) == "-help" ||
            QString(argv[i]) == "--help"
            ) {
            printUsage();
            return 0;
        }

        if (QString(argv[i]) == "close") {
            Window window;
            sscanf(argv[++i], "%lx", &window);
            sendCloseEvent(window);
        }

    }
}
