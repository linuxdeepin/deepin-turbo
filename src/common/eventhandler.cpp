#include "eventhandler.h"
#include "connection.h"
#include "logger.h"
#include "booster.h"
#include <sys/socket.h>
#include <QtConcurrentRun>
#include <MApplication>

int EventHandler::m_sighupFd[2];
struct sigaction EventHandler::m_oldSigAction;

EventHandler::EventHandler(Booster* parent) : m_item(0), m_parent(parent)
{
}

void EventHandler::runEventLoop()
{
    Logger::logError(" EventHandler::runEventLoop() ");

    // Exit from event loop when invoker is ready to connect
    connect(this, SIGNAL(connectionAccepted()), QApplication::instance() , SLOT(quit()));
    connect(this, SIGNAL(connectionRejected()), QApplication::instance() , SLOT(quit()));

    // Enable theme change handler
    m_item = new MGConfItem(MEEGOTOUCH_THEME_GCONF_KEY, 0);
    connect(m_item, SIGNAL(valueChanged()), this, SLOT(notifyThemeChange()));

    // Start another thread to listen connection from invoker
    QtConcurrent::run(this, &EventHandler::accept);

    // Create socket pair for SIGHUP
    bool handlerIsSet = false;
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, m_sighupFd))
    {
        Logger::logError("EventHandler: Couldn't create HUP socketpair");
    }
    else
    {
        // Install signal handler e.g. to exit cleanly if launcher dies.
        // This is a problem because MBooster runs a Qt event loop.
        EventHandler::setupUnixSignalHandlers();

        // Install a socket notifier on the socket
        connect(new QSocketNotifier(m_sighupFd[1], QSocketNotifier::Read, this),
                SIGNAL(activated(int)), this, SLOT(handleSigHup()));

        handlerIsSet = true;
    }

    // Run event loop so MApplication and MApplicationWindow objects can receive notifications
    MApplication::exec();

    // Disable theme change handler
    disconnect(m_item, 0, this, 0);
    delete m_item;
    m_item = NULL;

    // Restore signal handlers to previous values
    if (handlerIsSet)
    {
        restoreUnixSignalHandlers();
    }
}

void EventHandler::accept()
{
    if (m_parent->connection()->accept(m_parent->appData()))
    {
        emit connectionAccepted();
    }
    else
    {
        emit connectionRejected();
    }

}

void EventHandler::notifyThemeChange()
{
    MApplication::quit();
    ::_exit(EXIT_SUCCESS);
}

//
// All this signal handling code is taken from Qt's Best Practices:
// http://doc.qt.nokia.com/latest/unix-signals.html
//

void EventHandler::hupSignalHandler(int)
{
    Logger::logError(" EventHandler::hupSignalHandler(int) ");

    char a = 1;
    ::write(m_sighupFd[0], &a, sizeof(a));
}

void EventHandler::handleSigHup()
{
    MApplication::quit();
    ::_exit(EXIT_SUCCESS);
}

bool EventHandler::setupUnixSignalHandlers()
{
    struct sigaction hup;

    hup.sa_handler = hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, &m_oldSigAction) > 0)
    {
        return false;
    }

    return true;
}

bool EventHandler::restoreUnixSignalHandlers()
{
    if (sigaction(SIGHUP, &m_oldSigAction, 0) > 0)
    {
        return false;
    }

    return true;
}
