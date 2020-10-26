#include "configeditdialog.h"
#include "ui_configeditdialog.h"

ConfigEditDialog::ConfigEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigEditDialog)
{
    ui->setupUi(this);
}

ConfigEditDialog::~ConfigEditDialog()
{
    delete ui;
}

void ConfigEditDialog::setConfigFile(const QString &filename) {
    _f = filename;
    QFile f(filename);
    if ( f.open( QFile::ReadWrite ) ) {
        const QString data( f.readAll().constData() );
        ui->textBrowser->setText( data );
    }
}

void ConfigEditDialog::on_buttonBox_accepted()
{
    QFile f(_f);
    if ( f.open( QFile::ReadWrite ) ) {
        const QString data( f.readAll().constData() );
        f.resize(0);
        f.write( ui->textBrowser->toPlainText().toUtf8() );
        f.close();
    }
}
