#include "swiftworker.h"
#include <QCoreApplication>

QString SwiftWorker::getConfigValue(const QString &param) {
    if ( session != nullptr && session->isJoined() ) {
        const QString res = session->call(RPC_SETTINGS_GET_PARAM,{param}).toStringList().at(0);
        return res;
    } else {
        return "0";
    }
}

void SwiftWorker::insertLog(const QString &group, const QString &message) {

    if ( session != nullptr && session->isJoined() ) {
        session->call( RPC_SERVER_LOGGER_LOGS, {getWorkerName(), group, message });
    }
}

void SwiftWorker::insertError(const QString &group, const QString &message) {
    if ( session != nullptr && session->isJoined() ) {
        session->call(RPC_SERVER_LOGGER_ERRORS, {getWorkerName(), group, message });
    }
}

SwiftWorker::SwiftWorker(QObject *parent) : QObject(parent),session(nullptr),wdtimer( new QTimer(this) )
{
    const QHostAddress &localhost = QHostAddress(QHostAddress::LocalHost);
    for (const QHostAddress &address: QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != localhost)
             hostip = address.toString();
    }
    status = 1;
    QObject::connect( this, &SwiftWorker::restartRequested, this, &SwiftWorker::onRestart );
    QObject::connect( wdtimer, &QTimer::timeout, this, &SwiftWorker::watchdog );
    wdtimer->setInterval( 25000 );
    wdtimer->start();

}

QStringList SwiftWorker::listenParams() {
    return QStringList({});
}

void SwiftWorker::onParamChanged(const QString &param, const QString &value) {
    Q_UNUSED(param)
    Q_UNUSED(value)
}

void SwiftWorker::setStatus(const quint32 &stat) {
    status = stat;
}

quint32 SwiftWorker::getStatus() const {
    return status;
}

Wamp::Session *SwiftWorker::getSession() {
    return session;
}

void SwiftWorker::setIsInited() {
    if ( session != nullptr && session->isJoined() ) {
        const QString json_data( QJsonDocument( QJsonObject({{"module",getWorkerName()},{"ts",QString::number(QDateTime::currentSecsSinceEpoch())}}) ).toJson( QJsonDocument::Compact ) );
        session->publish(FEED_EVENTS_SYSTEM, {1,json_data });
    }
}

void SwiftWorker::watchdog() {
    if ( session != nullptr && session->isJoined() ) {
        session->publish(FEED_WATCHDOG, { getWorkerName() });
    } else {
        emit restartRequested();
    }
}

void SwiftWorker::onRestart() {
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    if ( session != nullptr && session->isJoined() ) {
        const QString json_data( QJsonDocument( QJsonObject({{"module",getWorkerName()},{"ts",QString::number(QDateTime::currentSecsSinceEpoch())}}) ).toJson( QJsonDocument::Compact ) );
        session->publish(FEED_EVENTS_SYSTEM, {2,json_data });
    }
    qApp->quit();
}

void SwiftWorker::onWampSession(Wamp::Session *sess) {

    if ( session == nullptr ) {
        session = sess;
    }
    initWorker( session );
    // setIsInited();
}
