#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <swiftbot.h>

#include <QLockFile>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlError>
#include "orderbooksproxy.h"
#include "orderbooksnode.h"
#include "nodescontroller.h"
#include <QThread>

static QVector<QSharedPointer<OrderbooksNode>> _orderbooks_nodes;
static QVector<QSharedPointer<QThread>> _orderbooks_threads;

typedef QPair<QString,quint32> TradePair;
typedef QHash<QString, QList<TradePair>> ActivePairs;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("orderbooks");
    QCoreApplication::setApplicationVersion("1.0.493");

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    // App commandline options
    QCommandLineParser parser;
    parser.setApplicationDescription("SwiftBot orderbooks proxy module");
    parser.addHelpOption();
    parser.addVersionOption();

    // Home dir option
    QCommandLineOption targetDirectoryOption(QStringList() << "d" << "home-dir", "Application home directory","home-dir");
    targetDirectoryOption.setDefaultValue( APP_DIR );
    parser.addOption(targetDirectoryOption);
    parser.process(a);

    // MySQL db
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName( SwiftBot::appParam(SETTINGS_NAME_MYSQL_HOST ).toString() );
    db.setPort( SwiftBot::appParam(SETTINGS_NAME_MYSQL_PORT ).toInt() );
    db.setUserName( SwiftBot::appParam(SETTINGS_NAME_MYSQL_USER).toString() );
    db.setPassword( SwiftBot::appParam(SETTINGS_NAME_MYSQL_PASSWORD ).toString() );
    db.setDatabaseName( SwiftBot::appParam(SETTINGS_NAME_MYSQL_DBNAME ).toString() );

    if ( !db.open() ) {
        qWarning() << "MySQL database error: ";
        qWarning() << db.lastError().text();
        return 1;
    }
    SwiftBot::initWampClient();

    NodesController * node_controller = new NodesController();
    QObject::connect( node_controller, &NodesController::pairsRate, [&]( const QMap<quint32,double> &rates ) {
       QString prep_query("INSERT INTO `rates` (`pair_id`,`rate`) VALUES ");
       QStringList parts;
       for ( auto it = rates.begin(); it != rates.end(); it++ ) {
           parts.push_back( "("+QString::number(it.key() )+", "+QString::number(it.value(), 'f', 8 )+")" );
       }
       QSqlQuery q( prep_query + parts.join(",") );
       if ( !q.exec() ) {
           qWarning() << q.lastError().text();
       }
    });
    //QObject::connect( node_controller, &NodesController::clientExited, &a, &QCoreApplication::quit );
    //QObject::connect( &a, &QCoreApplication::aboutToQuit, ratesThread, &QThread::quit );

    QObject::connect( node_controller, &NodesController::orderbooksAccumulated, wamp_client.data(), &WampClient::publishMessage, Qt::QueuedConnection );

    OrderBooksProxy * orderbooks_proxy = new OrderBooksProxy();
    QObject::connect( wamp_client.data(), &WampClient::clientConnected, orderbooks_proxy, &OrderBooksProxy::onWampSession );
    QObject::connect( wamp_client.data(), &WampClient::clientdiconnected, [&a]() {
        qWarning() << "WAMP client disconnected. Exiting.";
        a.quit();
    });

    QTimer * watchdog = new QTimer();
    watchdog->setInterval( 30000 );
    QObject::connect( watchdog, &QTimer::timeout, [](){
        wamp_client->publish( FEED_WATCHDOG, { QCoreApplication::applicationName().replace("swift-","")});
    });
    watchdog->start();

    QObject::connect( node_controller, &NodesController::orderbooksAccumulated, orderbooks_proxy, &OrderBooksProxy::mergeOrderbooks, Qt::QueuedConnection );


    QStringList exchanges;
    QSqlQuery q;
    if ( q.exec("SELECT * FROM exchanges WHERE is_enabled=1") ) {
        while ( q.next() ) {
            exchanges.push_back( q.value("name").toString() );
        }
    }
    q.finish();
    wamp_client->startClient();
    for ( auto it = exchanges.begin(); it != exchanges.end(); it++ ) {
        const quint32 eid = SwiftCore::getAssets()->getExchangeId( *it );
        const QList<quint32> _pids( SwiftCore::getAssets()->getExchangePairs( eid ) );
        QList<QPair<QString, quint32> > _pairs;
        for( auto itp = _pids.begin(); itp != _pids.end(); itp++ ) {
            if ( SwiftCore::getAssets()->isMarketActive( *itp) ) {
                _pairs.push_back({SwiftCore::getAssets()->getMarketName( *itp ), *itp });
            }
        }

        QThread * _nodethread = new QThread();
        _nodethread->setObjectName( *it );
        OrderbooksNode * _nodeitem  = new OrderbooksNode();
        QObject::connect( _nodeitem, &OrderbooksNode::nodeError,  node_controller, &NodesController::onNodeError );

        QObject::connect( _nodethread, &QThread::started, _nodeitem, &OrderbooksNode::startNode);
        QObject::connect( _nodethread, &QThread::started, node_controller, &NodesController::nodeThreadStarted );
        QObject::connect( _nodethread, &QThread::finished, node_controller, &NodesController::nodeThreadFinished );

        QObject::connect( node_controller, &NodesController::startNodes, _nodethread, &QThread::start );

        QObject::connect( node_controller, &NodesController::stopNodes, _nodeitem, &OrderbooksNode::stopNode );
        QObject::connect( _nodeitem, &OrderbooksNode::orderbooks,  node_controller, &NodesController::onOrderBooks );

        _nodeitem->setExchange(*it, _pairs );

        _nodeitem->moveToThread( _nodethread );
        QSharedPointer<QThread> _nodethread_ptr( std::move(_nodethread) );
        QSharedPointer<OrderbooksNode> _nodeitem_ptr( std::move( _nodeitem ) );
        _orderbooks_threads.push_back( std::move( _nodethread_ptr ) );
        _orderbooks_nodes.push_back( std::move( _nodeitem_ptr ) );
    }
    node_controller->startAllNodes();
    return a.exec();
}
