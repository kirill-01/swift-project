#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "../swift-corelib/wampclient.h"
#include "../swift-corelib/swiftcore.h"

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
    QCoreApplication::setApplicationVersion("1.0.275");

    // Allow only one instance per host
    QLockFile lockFile(QDir::temp().absoluteFilePath( QString(QCoreApplication::applicationName()+".lock") ) );
    if(!lockFile.tryLock(100)){
       qWarning() << "Another instance is already running";
       return 1;
    }

    if ( QFile::exists( "/opt/swift-bot/modules/orderbooks/orderbooks.dist" ) ) {
        QSettings dist_settings( "/opt/swift-bot/modules/orderbooks/orderbooks.dist", QSettings::IniFormat );
        QSettings current_settings( "/opt/swift-bot/modules/orderbooks/orderbooks.ini", QSettings::IniFormat );
        const QStringList dist_keys( dist_settings.allKeys() );
        for( auto it = dist_keys.begin(); it != dist_keys.end(); it++ ) {
            if ( !current_settings.contains( *it ) ) {
                current_settings.setValue( *it, dist_settings.value( *it ) );
            }
        }
        current_settings.sync();
        QFile::remove( "/opt/swift-bot/modules/orderbooks/orderbooks.dist" );
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

    static QString app_dir( parser.value( targetDirectoryOption ) );

    // Get stored settings
    QSettings settings(app_dir+"/settings.ini", QSettings::IniFormat );


    // MySQL db
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName( settings.value(SETTINGS_NAME_MYSQL_HOST ).toString() );
    db.setPort( settings.value(SETTINGS_NAME_MYSQL_PORT ).toInt() );
    db.setUserName( settings.value(SETTINGS_NAME_MYSQL_USER).toString() );
    db.setPassword( settings.value(SETTINGS_NAME_MYSQL_PASSWORD ).toString() );
    db.setDatabaseName( settings.value(SETTINGS_NAME_MYSQL_DBNAME ).toString() );

    if ( !db.open() ) {
        qWarning() << "MySQL database error: ";
        qWarning() << db.lastError().text();
        return 1;
    }

    // Wamp client
    WampClient * wamp_client = new WampClient(
       settings.value(SETTINGS_NAME_WAMP_REALM,"swift").toString(),
       settings.value(SETTINGS_NAME_WAMP_HOME,"localhost").toString(),
       settings.value(SETTINGS_NAME_WAMP_PORT, 8081).toInt(),
       settings.value(SETTINGS_NAME_WAMP_DEBUG, false).toBool());

    NodesController * node_controller = new NodesController();
    //QObject::connect( node_controller, &NodesController::pairsRate, writer, &RatesWriter::onRates, Qt::QueuedConnection );
    //QObject::connect( node_controller, &NodesController::clientExited, &a, &QCoreApplication::quit );
    //QObject::connect( &a, &QCoreApplication::aboutToQuit, ratesThread, &QThread::quit );

    QObject::connect( node_controller, &NodesController::orderbooksAccumulated, wamp_client, &WampClient::publishMessage, Qt::QueuedConnection );

    OrderBooksProxy * orderbooks_proxy = new OrderBooksProxy();
    QObject::connect( wamp_client, &WampClient::clientConnected, orderbooks_proxy, &OrderBooksProxy::onWampSession );
    QObject::connect( wamp_client, &WampClient::clientdiconnected, [&a](){
        qWarning() << "WAMP client disconnected. Exiting.";
        a.quit();
    });
    QObject::connect( node_controller, &NodesController::orderbooksAccumulated, orderbooks_proxy, &OrderBooksProxy::mergeOrderbooks );
    wamp_client->startClient();

    QStringList exchanges;
    QSqlQuery q("SELECT * FROM exchanges WHERE is_enabled=1");
    if ( q.exec() ) {
        while ( q.next() ) {
            exchanges.push_back( q.value("name").toString() );
        }
    }
    q.finish();

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
