#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QTableWidgetItem>
#include <QSettings>
#include <QVector>
#include "configeditdialog.h"
#include "accessruleswindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    process = new QProcess( this );
    process->setProgram("/usr/bin/swift-server");
    connect( process, &QProcess::stateChanged, [=](QProcess::ProcessState state){
        if ( state == QProcess::ProcessState::NotRunning ) {
            ui->pushButton->setText("Start server");
        }
    });
}

MainWindow::~MainWindow()
{
    process->kill();
    delete ui;
}

void MainWindow::setModules(const QJsonArray &modules) {
    int i =0;
    for( auto it = modules.begin(); it != modules.end(); it++ ) {
        _settings_files[i].reset( new QSettings( "/opt/swift-bot/modules/"+it->toObject().value("name").toString()+"/"+it->toObject().value("name").toString()+".ini", QSettings::IniFormat ));
        ui->tableWidget->insertRow( i );
        ui->tableWidget->setItem( i, 0, new QTableWidgetItem( it->toObject().value("name").toString() ) );

        QTableWidgetItem *item2 = new QTableWidgetItem("is_enabled");
        item2->setCheckState( it->toObject().value("is_enabled").toBool() ? Qt::Checked : Qt::Unchecked );

        ui->tableWidget->setItem( i, 1, item2 );

        ui->tableWidget->setItem( i, 2, new QTableWidgetItem( "Manage rights" ) );
        ui->tableWidget->setItem( i, 3, new QTableWidgetItem( "Edit config") );
        ui->tableWidget->setItem( i, 4, new QTableWidgetItem( it->toObject().value("binary").toString() ) );
        ui->tableWidget->setItem( i, 5, new QTableWidgetItem( it->toObject().value("description").toString() ) );
        i++;
    }
    connect( ui->tableWidget, &QTableWidget::itemClicked, [=](QTableWidgetItem *item){
        const quint32 module_index = item->row();
        const quint32 column = item->column();
        if ( column == 1 ) {
            _settings_files[ module_index ]->setValue("is_enabled", (item->checkState() == Qt::Checked) );
        } else if ( column == 4 ) {
            _settings_files[ module_index ]->setValue("binary", item->text() );
        } else if ( column == 5 ) {
            _settings_files[ module_index ]->setValue("description", item->text() );
        } else if ( column == 3 ) {
            ConfigEditDialog * dialog = new ConfigEditDialog( this );
            dialog->setWindowTitle( _settings_files[module_index]->fileName() );
            dialog->setConfigFile( _settings_files[module_index]->fileName() );
            dialog->showNormal();
        } else if ( column == 2 ) {
            AccessRulesWindow * w2 = new AccessRulesWindow( this );
            w2->loadWindow( _settings_files[module_index]->value("name").toString() );
        }

    });
}


void MainWindow::on_pushButton_clicked()
{
    if ( process->state() == QProcess::Running ) {
        process->kill();
        ui->pushButton->setText( "Start server" );
        qWarning() << "Killed";
    } else if ( process->state() == QProcess::NotRunning ) {
        process->start();
        qWarning() << "Started";
        ui->pushButton->setText( "Stop server" );
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if ( process->state() == QProcess::Running ) {
        if ( QMessageBox::question(
                 this,
                 "Server is still runing","If you close this window, server process will be terminated too. Sure?")
         ) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}
