#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QSettings>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setModules( const QJsonArray& modules );
private slots:
    void on_pushButton_clicked();
protected:
    void closeEvent(QCloseEvent *event);
private:
    QProcess * process;
    Ui::MainWindow *ui;
    QMap<quint32, QSharedPointer<QSettings>> _settings_files;
};
#endif // MAINWINDOW_H
