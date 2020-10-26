#ifndef SWIFTCORE_H
#define SWIFTCORE_H

#include "swiftbot.h"

#include "swift-corelib_global.h"
#include <QSettings>
#include <QSharedPointer>
#include <QFile>
#include "assetsstorage.h"

class SWIFTCORELIB_EXPORT SwiftCore
{
public:
    SwiftCore();

    /**
     * @brief createQuery
     * Create query in called thread
     * @param dbname
     * @return
     */
    static QSqlQuery createQuery( const QString& dbname = "default" );

    /**
     * @brief getExchangeFee
     * Shorthand to assets
     * @param exchange_id
     * @return
     */
    static double getExchangeFee( const quint32& exchange_id ) {
        return getAssets()->getExchangeTradeFee( exchange_id );
    }

    /**
     * @brief getAssets
     * Get assets object
     * @return
     */
    static AssetsStorage * getAssets( const bool& force_reload = false ) {
        static AssetsStorage* _i = nullptr;
        if ( _i == nullptr ) {
            _i = new AssetsStorage();
            _i->loadAssets();
        } else {
            if ( force_reload ) {
                _i->loadAssets();
            }
        }

        return _i;
    }

    /**
     * @brief getSettings
     * Get system settings
     * @param home_dir
     * @return
     */
    static QSettings * getSettings( const QString& s = "") {
        Q_UNUSED(s)
        return SwiftBot::appSettings();
    }

    /**
     * @brief getModuleSettings
     *
     * get module params file
     * @param module_name
     * @return
     */
    static QSettings * getModuleSettings( const QString& s = "") {
         Q_UNUSED(s)
        if ( s == "" ) {
            return SwiftBot::moduleSettings();
        } else {
            return SwiftBot::moduleSettings( s );
        }
    }
};

#endif // SWIFTCORE_H
