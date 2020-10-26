#include "resultwaiter.h"

ResultWaiter::ResultWaiter(QObject *parent) : QObject(parent), session(nullptr)
{

}

void ResultWaiter::waitFor(const quint64 &uid) {
    _asyncs.push_back( uid );
}

void ResultWaiter::onWampSession(Wamp::Session *sess) {
    session = sess;
    session->subscribe( FEED_ASYNC_RESULTS , [=]( const QVariantList&v, const QVariantMap& m ) {
        Q_UNUSED(m)
        const QString strmsg( v.at(0).toString() );
        QJsonObject j_resp( QJsonDocument::fromJson( strmsg.toUtf8() ).object() );
        const quint64 uid = j_resp.value("async_uuid").toString().toULongLong();
        if ( _asyncs.contains( uid ) ) {
            emit resultReceived( QJsonDocument( j_resp ).toJson( QJsonDocument::Indented ) );
        }
    });
}

void ResultWaiter::onKeyPress(char key) {
    if ( QString( key ) ==  "\n" ) {
        if ( session == nullptr || !session->isJoined() ) {
            qWarning() << "Not connected to SwiftBot server!";
            return;
        }
        last_commands.push_back( current_command );
        history_index = last_commands.count() - 1;
        std::cout << "> " << std::flush;
        qWarning() << "cmd: " << current_command;
        if ( current_command == "quit" || current_command == "exit" ) {
            qWarning() << "Exiting...";
            qApp->exit(0);
        } else if ( !current_command.isEmpty() ) {
            QStringList cmd( current_command.split(" ") );
            if ( cmd.count() == 2 ) {
                QVariantList prms;
                if ( cmd.at(1).at(0) == "{" || cmd.at(1).at(0) == "[" ) {
                    prms.push_back( cmd.at(1) );
                } else {
                    QStringList sep_prms( cmd.at(1).split(",") );
                    if ( sep_prms.count() > 1 ) {
                        for( auto it = sep_prms.begin(); it != sep_prms.end(); it++ ) {
                            prms.push_back( *it );
                        }
                    } else {
                        prms.push_back( cmd.at(1) );
                    }
                }
                QVariant re = session->call( cmd.at(0),  prms );

                if ( re.toULongLong() > 0 ) {
                    waitFor( re.toULongLong() );
                } else {
                    qWarning() << "Response: \n";
                    std::cout << re.toString().toStdString() << std::endl;
                    std::cout << "> " << std::flush;
                }
            } else if (cmd.count() > 2 ) {
                QVariantList prms({ current_command.section(" ", 1 ) });
                const QString comm( cmd.at(0) );
                QVariant re = session->call( comm,  prms );

                if ( re.toULongLong() > 0 ) {
                    waitFor( re.toULongLong() );
                } else {
                    qWarning() << "Response: \n";
                    std::cout << re.toString().toStdString() << std::endl;
                    std::cout << "> " << std::flush;
                }
            }
            else {
                const QString commd( cmd.at(0) );
                QVariant re = session->call( commd );
                if ( re.toULongLong() > 0 ) {
                    waitFor( re.toULongLong() );

                } else {
                    qWarning() << "Response: \n";
                    const QJsonDocument jdoc( QJsonDocument::fromJson( re.toString().toUtf8() ) );
                    if ( !jdoc.isEmpty() && !jdoc.isNull() ) {
                        std::cout << jdoc.toJson( QJsonDocument::Indented ).toStdString() << std::endl;
                        std::cout << "> " << std::flush;
                    } else {
                        std::cout << re.toString().toStdString() << std::endl;
                        std::cout << "> " << std::flush;
                    }
                }
            }
        }
        current_command = "";
    } else if ( QString( key ) == "\u007F" ) {
        std::cout << '\b';
        std::cout << " ";
        std::cout << '\b';
        if ( !current_command.isEmpty() ) {
            current_command.remove( current_command.length() - 1, 1 );
        }
    } else if ( QString( key ) == "\u001B" ) {
        if ( history_index > 0 ) {
            history_index--;
        } else {
            history_index = last_commands.count() -1;
        }
        current_command = last_commands.at( history_index );
        std::cout << "> " << current_command.toStdString() << std::flush;
    }
    else {
        current_command += QString( key );
        std::cout << key << std::flush;
    }
}
