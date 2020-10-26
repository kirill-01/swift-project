#ifndef ACCESSRULESWINDOW_H
#define ACCESSRULESWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QFile>
#include <QDirIterator>

QT_BEGIN_NAMESPACE
namespace Ui { class AccessRulesWindow; }
QT_END_NAMESPACE


class AccessRulesWindow : public QMainWindow
{
    Q_OBJECT
public:
    QJsonObject collect_all_rules() {
        QStringList _modules_files;
        QDirIterator it("/opt/swift-bot/modules", QStringList() << "wamp.json", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            _modules_files << it.next();
        }
        QJsonObject _all_rules;
        for( auto it = _modules_files.begin(); it != _modules_files.end(); it++ ) {
            QFile f( *it );
            if ( f.open( QFile::ReadOnly ) ) {
                QJsonArray j_items( QJsonDocument::fromJson( f.readAll().constData() ).object().value("provide").toObject().value("methods").toArray() );
                for( auto iii = j_items.begin(); iii != j_items.end(); iii++ ) {
                    _all_rules[ iii->toObject().value("uri").toString() ] = QJsonObject(
                    {{"uri",iii->toObject().value("uri").toString()}, {"description",iii->toObject().value("description").toString()},{"type","method"}});
                }
                QJsonArray j_items2( QJsonDocument::fromJson( f.readAll().constData() ).object().value("provide").toObject().value("feeds").toArray() );
                for( auto iii = j_items2.begin(); iii != j_items2.end(); iii++ ) {
                    _all_rules[ iii->toObject().value("uri").toString() ] = QJsonObject(
                    {{"uri",iii->toObject().value("uri").toString()}, {"description",iii->toObject().value("description").toString()},{"type","feed"}});
                }
                f.close();
            }
        }
        return _all_rules;
    }
    explicit AccessRulesWindow(QWidget *parent = nullptr);
    ~AccessRulesWindow();
    void loadWindow( const QString& module_name );
signals:
private:
    QJsonObject j_wamp_config;
    Ui::AccessRulesWindow *ui;
};

#endif // ACCESSRULESWINDOW_H
