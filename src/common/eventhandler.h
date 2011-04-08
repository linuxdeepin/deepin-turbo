#include <QObject>
#include <QSocketNotifier>
#include <signal.h>
#include <tr1/memory>
#include <MGConfItem>

using std::tr1::shared_ptr;

class Connection;
class Booster;

class EventHandler : public QObject
{
    Q_OBJECT

public:

    enum EventHandlerType 
    {
        QEventHandler,
        MEventHandler
    };

    //! \brief Constructor
    EventHandler(Booster* parent, EventHandlerType type);

    //! \brief Destructor
    virtual ~EventHandler() {}

    void runEventLoop();

    //! UNIX signal handler for SIGHUP
    static void hupSignalHandler(int unused);

    //! Setup UNIX signal handlers
    static bool setupUnixSignalHandlers();

    //! Restore UNIX signal handlers to previous values
    static bool restoreUnixSignalHandlers();

private:

    //! wait for socket connection
    void accept();

    //! Socket pair used to get SIGHUP
    static int m_sighupFd[2];

    //! Socket notifier used for m_sighupFd
    shared_ptr<QSocketNotifier> m_snHup;

    //! Old sigaction struct
    static struct sigaction m_oldSigAction;

    //! GConf item to listen theme change
    MGConfItem* m_item;

    // Parent object
    Booster* m_parent;

    // type of application's event loop
    const EventHandlerType m_type;

private slots:

    //! Qt signal handler for SIGHUP.
    void handleSigHup();

    //! Qt signal handler for theme change
    void notifyThemeChange();

signals:
    void connectionAccepted();
    void connectionRejected();
};
