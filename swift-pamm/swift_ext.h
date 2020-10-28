#ifndef SWIFT_EXT_H
#define SWIFT_EXT_H
#include <QObject>
#include <QMap>
#include <QDateTime>
#include <swiftbot.h>
#include <QCache>

namespace SwiftBot {


class Investion;
typedef QMap<quint32,Investion> Investions;


    class ProfitTransaction {
    public:
        ProfitTransaction( const quint32& tid ) : id( tid ) {

        }

        quint32 id;
        quint32 coin_id;
        quint32 invest_id;
        double amount;
        QDateTime ts;

    };

    class Investion {
    public:
        static QCache<quint32, Investion> _invests_cache;

        static Investions active() {
            Investions r;
            QSqlQuery q;
            if ( q.exec("SELECT * FROM pamm_invests WHERE is_active=1") ) {
                while ( q.next() ) {
                    r.insert( q.value("id").toUInt(), Investion( q.record() ) );
                }
            }
            return r;
        }

        double realProfit() {
            return profit - feeAmount();
        }

        double feeAmount() {
            return profit * fee_size;
        }

        static double activePrice() {
            double r = 0;
            Investions all( Investion::active() );
            for( auto it = all.begin(); it != all.end(); it++ ) {
                r += it->price;
            }
            return r;
        }

        Investion( const QSqlRecord & q ) {
            id = q.value("id").toUInt();
            user_id = q.value("user_id").toUInt();
            coin_id = q.value("coin_id").toUInt();
            profit_coin_id = q.value("profit_coin_id").toUInt();
            amount = q.value("amount").toDouble();
            profit = q.value("profit").toDouble();
            fee_size = q.value("fee").toDouble();
            return_address = q.value("return_address").toString();
            profit_address = q.value("profit_address").toString();
            started_at = q.value("start_time").toDateTime();
            days = q.value("days").toUInt();

            days_left = QDateTime::currentDateTime().daysTo( started_at.addDays( days ) );
            fee = feeAmount();
        }

        Investion( const quint32 & invest_id ) {
            QSqlQuery q;
            if ( q.exec("SELECT id FROM pamm_invests WHERE id="+QString::number( invest_id ) ) ) {
                id = q.value("id").toUInt();
                user_id = q.value("user_id").toUInt();
                coin_id = q.value("coin_id").toUInt();
                profit_coin_id = q.value("profit_coin_id").toUInt();
                amount = q.value("amount").toDouble();
                profit = q.value("profit").toDouble();
                fee_size = q.value("fee").toDouble();
                return_address = q.value("return_address").toString();
                profit_address = q.value("profit_address").toString();
                started_at = q.value("start_time").toDateTime();
                days = q.value("days").toUInt();

                days_left = QDateTime::currentDateTime().daysTo( started_at.addDays( days ) );
                fee = feeAmount();
            }
        }
        quint32 id;
        quint32 user_id;
        quint32 coin_id;
        quint32 profit_coin_id;
        double amount;
        double profit;
        double fee_size;
        double fee;
        QString return_address;
        QString profit_address;
        QDateTime started_at;
        quint32 days;
        quint32 days_left;
        double price;
    };

    class User;
    typedef QMap<quint32, User> Users;

    class User {
    public:

        static User create(const QString& username, const QString& password ) {
            // Change to use used id param
            QString pswd( QCryptographicHash::hash( QString( password + "u_" + username).toUtf8(), QCryptographicHash::Sha1 ).toHex() );
            QSqlQuery q("INSERT INTO pamm_users (username, password) VALUES ('"+username+"','"+pswd+"')");
            if ( q.exec() ) {
                const quint32 newid = q.lastInsertId().toUInt();
                return User( newid );
            } else {
                return User(0);
            }
        }

        bool auth( const QString& password ) {
            return pass == QCryptographicHash::hash( QString( password + "u_"+name).toUtf8(), QCryptographicHash::Sha1 ).toHex();
        }

        Investions invests() {
            Investions r;
            QSqlQuery q;
            if ( q.exec("SELECT * FROM pamm_invests WHERE user_id="+QString::number( id ) ) ) {
                while ( q.next() ) {
                    r.insert( q.value("id").toUInt(), Investion( q.record() ) );
                }
            }
            return r;
        }

        Investions investsActive() {
            Investions r;
            QSqlQuery q;
            if ( q.exec("SELECT * FROM pamm_invests WHERE is_active=1 AND user_id="+QString::number( id ) ) ) {
                while ( q.next() ) {
                    r.insert( q.value("id").toUInt(), Investion( q.record() ) );
                }
            }
            return r;
        }

        User( const QSqlRecord & q ) {
            id = q.value("id").toUInt();
            name = q.value("username").toUInt();
            pass = q.value("password").toUInt();
        }

        static Users getAll() {
            Users r;
            QSqlQuery q;
            if ( q.exec("SELECT * FROM users" ) ) {
                while ( q.next() ) {
                    r.insert( q.value("id").toUInt(), q.record() );
                }
            }
            return r;
        }

        static Users getAllActive() {
            Users r;
            QSqlQuery q;
            if ( q.exec("SELECT * FROM pamm_users WHERE id IN ( SELECT DISTINCT id FROM insvests WHERE is_active=1 )" ) ) {
                while ( q.next() ) {
                    r.insert( q.value("id").toUInt(), q.record() );
                }
            }
            return r;
        }

        User( const QString & username ) {
            QSqlQuery q;
            if ( q.exec("SELECT * FROM pamm_users WHERE username='"+username+"'" ) && q.next() ) {
                id = q.value("id").toUInt();
                name = q.value("username").toString();
                pass = q.value("password").toString();
            } else {
                id = 0;
                name = "";
                pass = "";
            }
        }

        User( const quint32 & user_id ) {
            QSqlQuery q;
            if ( q.exec("SELECT * FROM pamm_users WHERE id="+QString::number( user_id ) ) && q.next() ) {
                id = user_id;
                name = q.value("username").toString();
                pass = q.value("password").toString();
            } else {
                id = 0;
                name = "";
                pass = "";
            }
        }
        double activeIvestsPrice() {
            Investions ai( investsActive() );
            double ret = 0;
            for( auto it = ai.begin(); it != ai.end(); it++ ) {
                ret += it->price;
            }
            return ret;
        }
        double profitPartSize() {
            // All user active invests price
            const double user_price = Investion::activePrice();
            // All system active invests price
            const double app_price = activeIvestsPrice();
            return user_price / app_price * 100;

        }

        bool hasActiveInvests() {
            return !investsActive().isEmpty();
        }

        quint32 id;
        QString name;
        QString email;
        QString pass;

    };


}

#endif // SWIFT_EXT_H
