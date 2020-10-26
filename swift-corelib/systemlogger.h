#ifndef SYSTEMLOGGER_H
#define SYSTEMLOGGER_H

#include <QString>
#include <QDebug>
#include <QDateTime>

#include <swiftbot.h>

namespace SwiftBot {
#ifdef SWIFT_USE_WAMP_CLIENT
// Using SwiftBot logger
static void addLog( const QString & message, const QString & log_group = "INFO" ) {
    if ( !wamp_client.isNull() ) {
        static Wamp::Session * session = wamp_client->getSession();
        if ( session != nullptr && session->isJoined() ) {
            session->call( RPC_SERVER_LOGGER_LOGS, { SWIFT_MODULE_NAME, log_group, message });
            return;
        }
    }
    qInfo() << QDateTime::currentDateTime().time().toString() << log_group << message;
}

static void addError( const QString & message, const QString & log_group = "DANGER" ) {
    if ( !wamp_client.isNull() ) {
        static Wamp::Session * session = wamp_client->getSession();
        if ( session != nullptr && session->isJoined() ) {
            session->call( RPC_SERVER_LOGGER_ERRORS, { SWIFT_MODULE_NAME, log_group, message });
            return;
        }
    }
    qWarning() << QDateTime::currentDateTime().time().toString() << log_group << message;
}
#else
// Local output only
static void addLog( const QString & message, const QString & log_group ) {
    qInfo() << QDateTime::currentDateTime().toString() << log_group << message;
}

static void addError( const QString & message, const QString & log_group = "DANGER" ) {
    qWarning() << QDateTime::currentDateTime().toString() << log_group << message;
}
#endif
}

#endif // SYSTEMLOGGER_H
