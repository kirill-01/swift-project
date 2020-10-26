#include "statsnotifier.h"

StatsNotifier::StatsNotifier(QObject *parent) : QObject(parent), session( nullptr )
{
    QTimer::singleShot( 15000, this, &StatsNotifier::locaDailyStats );
}

void StatsNotifier::locaDailyStats() {

  QSqlQuery q;
    if ( q.exec("SELECT (AVG(max_profit)-AVG(min_profit))  as profit_available, ( AVG( max_amount ) - AVG( min_amount )) *AVG(max_sell_rate) as funds_required, arbitrage_pair_id FROM arbitrage_events where ts >  date_sub(NOW(), interval 1 hour) GROUP BY arbitrage_pair_id")) {
        QString msg_str("<u><b>Arbitrage info</b>(1h)</u>\n\nOver the past hour, orders were available in the amount of <u>$ :volume:</u>, which could bring profit <u>$ :profit:</u>\n\n<b>By markets, detailed:</b>\n\n");
        QString details;
        double total = 0;
        double prf = 0;
        while (q.next()) {
            const quint32 arbpair( q.value("arbitrage_pair_id").toUInt() );
            const QString arbname( SwiftCore::getAssets()->getArbitragePairName( arbpair ) );
            const QString bcur( SwiftCore::getAssets()->getArbitragePairBaseCurrencyName( arbpair ) );
            const QString mcur( SwiftCore::getAssets()->getArbitragePairMarketCurrencyName( arbpair ) );
            total +=q.value("funds_required").toDouble();
            prf += q.value("profit_available").toDouble();
            double percent = 0;
            if ( q.value("funds_required").toDouble() > 0 ) {
                percent = q.value("profit_available").toDouble() / q.value("funds_required").toDouble() * 100;
            }
            details += "\n<b>"+arbname+"</b> : "+QString::number( q.value("funds_required").toDouble(), 'f', 2 )+"\n<u>Average profit</u>: "+QString::number( q.value("profit_available").toDouble(), 'f', 2 )+"\n\n<i><b>~"+QString::number(percent,'f',3)+"%</b></i>\n";
        }
        msg_str.replace(":volume:", QString::number( total, 'f', 2 ) );
        msg_str.replace(":profit:", QString::number( prf, 'f', 2 ) );
        msg_str += details;
        if ( session != nullptr && session->isJoined() ) {
            session->call(RCP_TELEGRAM_NOTIFY, QVariantList({msg_str}));
        }
    } else {
        qWarning() << "MYSQL ERROR" << q.lastError().text();
    }
    QTimer::singleShot( SwiftCore::getModuleSettings("arbitrage")->value("stats_show_interval", 600000).toUInt(), this, &StatsNotifier::locaDailyStats );
}

void StatsNotifier::onWampSession(Wamp::Session *sess) {
    session = sess;

    session->provide("swift.arbitrage.volume", [=]( const QVariantList&v, const QVariantMap&m ) {
        Q_UNUSED(v)
        Q_UNUSED(m)
        QTimer::singleShot( 600, this, &StatsNotifier::locaDailyStats );
        return "OK";
    });
}
