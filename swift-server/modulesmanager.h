#ifndef MODULESMANAGER_H
#define MODULESMANAGER_H

#include <QObject>
#include <QFile>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

struct ModuleInfo {
    QString name;
    QString description;
    QString version;
    QString binary;
    bool is_enabled;
    bool is_exchange;
    public:
    QJsonObject toJson() const {
        QJsonObject j;
        j["name"] = name;
        j["description"] = description;
        j["version"] = version;
        j["binary"] = binary;
        j["is_enabled"] = is_enabled;
        j["is_exchange"] = is_exchange;
        return j;
    }
};

class ModulesManager : public QObject
{
    Q_OBJECT
public:
    explicit ModulesManager(QObject *parent = nullptr);

    /**
     * @brief getAvailableModules
     * @return
     */
    static QList<ModuleInfo> getAvailableModules( const QString& app_dir ) {
        QDir dir( app_dir+"/modules/" );
        const QStringList found_modules_dirs( dir.entryList( QDir::Filter::Dirs ) );
        QList<ModuleInfo> _ret;

        for( auto it = found_modules_dirs.begin(); it != found_modules_dirs.end(); it++ ) {
            QString settings_file_name( app_dir+"/modules/"+*it+"/"+*it+".ini");
            if ( !QFile::exists( settings_file_name ) && QFile::exists( settings_file_name.replace("ini","dist") ) ) {
                QFile dist( settings_file_name.replace("ini","dist")  );
                if ( dist.open( QFile::ReadWrite ) ) {
                    dist.rename( settings_file_name );
                    dist.close();
                }
            }
            // Inlude module in list only if config file exists
            if( QFile::exists( settings_file_name ) ) {
                QSettings read_settings( settings_file_name, QSettings::IniFormat );
                ModuleInfo info;
                info.name = read_settings.value("name","").toString();
                info.description = read_settings.value("description").toString();
                info.version = read_settings.value("version").toString();
                info.binary = read_settings.value("binary").toString();
                info.is_enabled = read_settings.value("is_enabled").toBool();
                info.is_exchange = read_settings.value("is_exchange").toBool();
                // Additional check of config
                if ( info.name != "" && QFile::exists( info.binary ) ) {
                    _ret.push_back( info );
                }
            }
        }
        return _ret;
    }

signals:


};

#endif // MODULESMANAGER_H
