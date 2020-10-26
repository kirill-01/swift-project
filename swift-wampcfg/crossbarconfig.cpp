#include "crossbarconfig.h"

CrossbarConfig::CrossbarConfig(QObject *parent) : QObject(parent)
{

}

QJsonObject CrossbarConfig::getRuleObj(const QString &uri) {
    QJsonObject j({
                      {"uri", uri},
                      {"match",(uri.contains("*") || uri == "") ? "prefix" : "exact"},
                      {"allow", QJsonObject({
                           {"call",false},
                           {"register",false},
                           {"publish",false},
                           {"subscribe",false},
                       })},
                      {"disclose", QJsonObject({
                           {"caller",false},
                           {"publisher",false}
                       })},
                      {"cache",true},
                  });
    return j;
}

QJsonArray CrossbarConfig::getPermissions(const QStringList &allow_call, const QStringList &allow_register, const QStringList &allow_publish, const QStringList &allow_subscribe) {
    QJsonArray j_rules;
    for( auto it = allow_call.begin(); it != allow_call.end(); it++ ) {
        QJsonObject j_it( getRuleObj( *it ) );
        QJsonObject j_allow( j_it.value("allow").toObject() );
        j_allow["call"] = true;
        j_it["allow"] = j_allow;
        j_rules.push_back( j_it );
    }
    for( auto it = allow_register.begin(); it != allow_register.end(); it++ ) {
        QJsonObject j_it( getRuleObj( *it ) );
        QJsonObject j_allow( j_it.value("allow").toObject() );
        j_allow["register"] = true;
        j_it["allow"] = j_allow;
        j_rules.push_back( j_it );
    }
    for( auto it = allow_publish.begin(); it != allow_publish.end(); it++ ) {
        QJsonObject j_it( getRuleObj( *it ) );
        QJsonObject j_allow( j_it.value("allow").toObject() );
        j_allow["publish"] = true;
        j_it["allow"] = j_allow;
        j_rules.push_back( j_it );
    }
    for( auto it = allow_subscribe.begin(); it != allow_subscribe.end(); it++ ) {
        QJsonObject j_it( getRuleObj( *it ) );
        QJsonObject j_allow( j_it.value("allow").toObject() );
        j_allow["subscribe"] = true;
        j_it["allow"] = j_allow;
        j_rules.push_back( j_it );
    }
    return j_rules;
}

void CrossbarConfig::loadConfig(const QString &config_filename) {
    QFile f( config_filename );
    if ( f.open( QFile::ReadWrite ) ) {
        _root = QJsonObject( QJsonDocument::fromJson( f.readAll().constData() ).object() );
        _controller = _root.value("controller").toObject();
        _workers = _root.value("workers").toArray();
        _worker = _workers.at(0).toObject();
        _worker_realms = _worker.value("realms").toArray();
        //_worker_realms_roles = _worker_realms.at(0).toObject().value("roles").toArray();

        _worker_transports = _worker.value("transports").toArray();
        _worker_transports_auth = _worker_transports.at(0).toObject().value("auth").toObject();


        _worker_transports_auth_wampcra = _worker_transports_auth.value("ticket").toObject();
        _worker_transports_auth_wampcra["type"] = "static";
        //_worker_transports_auth_wampcra_users = _worker_transports_auth_wampcra.value("principals").toObject();
        f.close();
    }

}

void CrossbarConfig::addUser(const QString &username, const QJsonObject &j_user) {
    _worker_transports_auth_wampcra_users[ username ] = j_user;
}

void CrossbarConfig::addRole(const QJsonObject &j_role) {
    for( auto it = _worker_realms_roles.begin(); it != _worker_realms_roles.end(); it++ ) {
        if ( it->toObject().value("name").toString() == j_role.value("name").toString() ) {
            _worker_realms_roles.removeAt( it.i );
        }
    }
    _worker_realms_roles.push_back( j_role );
}

QJsonObject CrossbarConfig::getConfig() {
    _worker_transports_auth_wampcra["principals"] = _worker_transports_auth_wampcra_users;
    _worker_transports_auth["ticket"] = _worker_transports_auth_wampcra;
    QJsonArray new_transports;
    for( auto it = _worker_transports.begin(); it != _worker_transports.end(); it ++ ) {
        QJsonObject j_transport( it->toObject() );
        if ( j_transport.value("type").toString() != "web" ) {
            j_transport["auth"] = _worker_transports_auth;
        }
        new_transports.push_back( j_transport );
    }
    _worker_transports = new_transports;
    QJsonObject j_realm( _worker_realms.at(0).toObject() );
    j_realm["roles"] = _worker_realms_roles;
    _worker_realms = QJsonArray({j_realm});
    _worker["realms"] = _worker_realms;
    _worker["transports"] = _worker_transports;
    _root["workers"] = QJsonArray({_worker});
    _root["controller"] = _controller;
    return _root;
}

void CrossbarConfig::saveConfig(const QString &config_filename) {
    QFile f( config_filename );
    if ( f.open( QFile::ReadWrite ) ) {
        f.resize(0);
        f.write( QJsonDocument( getConfig() ).toJson( QJsonDocument::Indented ) );
        f.close();
    }
}
