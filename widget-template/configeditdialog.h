#ifndef CONFIGEDITDIALOG_H
#define CONFIGEDITDIALOG_H

#include <QDialog>
#include <QFile>

namespace Ui {
class ConfigEditDialog;
}

class ConfigEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigEditDialog(QWidget *parent = nullptr);
    ~ConfigEditDialog();
    void setConfigFile( const QString & filename );
private slots:
    void on_buttonBox_accepted();

private:
    Ui::ConfigEditDialog *ui;
    QString _f;
};

#endif // CONFIGEDITDIALOG_H
