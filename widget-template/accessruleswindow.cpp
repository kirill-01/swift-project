#include "accessruleswindow.h"
#include "ui_accessruleswindow.h"

AccessRulesWindow::AccessRulesWindow(QWidget *parent) :  QMainWindow(parent)
  , ui(new Ui::AccessRulesWindow)
{
  ui->setupUi(this);
}
AccessRulesWindow::~AccessRulesWindow()
{
  delete ui;
}

void AccessRulesWindow::loadWindow(const QString &module_name) {

    setWindowTitle("Access rules : "+module_name );
    QFile f("/opt/swift-bot/modules/"+module_name+"/wamp.json");
    f.open( QFile::ReadWrite );
    j_wamp_config = QJsonDocument::fromJson( f.readAll().constData() ).object();
    f.close();




    QJsonObject items( collect_all_rules() );
    quint32 i = 0;
    QStringList ks( items.keys() );
    for( auto it = ks.begin(); it != ks.end(); it++ ) {
     ui->tableWidget->insertRow(i);
        ui->tableWidget->setColumnCount( 6 );
        const QString uri( *it );
        ui->tableWidget->setItem( i, 0, new QTableWidgetItem( uri ));
        ui->tableWidget->setItem( i, 1, new QTableWidgetItem( items.value( uri ).toObject().value("type").toString() ));

        if ( items.value(uri).toObject().value("type").toString() == "method" ) {
            QTableWidgetItem * i1 = new QTableWidgetItem();
            i1->setCheckState( j_wamp_config.value("module").toObject().value("call").toArray().contains( uri) ? Qt::Checked : Qt::Unchecked );
            QTableWidgetItem * i2 = new QTableWidgetItem();
            i2->setCheckState( j_wamp_config.value("module").toObject().value("register").toArray().contains( uri ) ? Qt::Checked : Qt::Unchecked );
            ui->tableWidget->setItem( i, 2, i1);
            ui->tableWidget->setItem( i, 3, i2);
        } else {
            QTableWidgetItem * i1 = new QTableWidgetItem();
            i1->setCheckState( j_wamp_config.value("module").toObject().value("publish").toArray().contains( uri) ? Qt::Checked : Qt::Unchecked );
            QTableWidgetItem * i2 = new QTableWidgetItem();
            i2->setCheckState( j_wamp_config.value("module").toObject().value("subscribe").toArray().contains( uri) ? Qt::Checked : Qt::Unchecked );
            ui->tableWidget->setItem( i, 4, i1);
            ui->tableWidget->setItem( i, 5, i2);
        }
        i++;
    }

    connect( ui->tableWidget, &QTableWidget::itemChanged, [=]( QTableWidgetItem * item ) {

        QJsonObject j_m( j_wamp_config.value("module").toObject() );
        QJsonArray j_call( j_m.value("call").toArray() );
        QJsonArray j_register( j_m.value("register").toArray() );
        QJsonArray j_subscribe( j_m.value("subscribe").toArray() );
        QJsonArray j_publish( j_m.value("publish").toArray() );

        const QString uri = ui->tableWidget->item( item->row(), 0)->text();
        const QString type = ui->tableWidget->item( item->row(), 1)->text();
        if ( type == "method" ) {
            if (  ui->tableWidget->item( item->row(), 2)->checkState() == Qt::Checked ) {
                if ( !j_call.contains( uri ) ) {
                   j_call.push_back( uri );
                }
            } else {
                if ( j_call.contains( uri ) ) {
                   for( auto it = j_call.begin(); it != j_call.end(); it++ ) {
                       if ( it->toString() == uri ) {
                           j_call.removeAt( it.i );
                       }
                   }
                }
            }
            if (  ui->tableWidget->item( item->row(), 3)->checkState() == Qt::Checked ) {
                if ( !j_register.contains( uri ) ) {
                   j_register.push_back( uri );
                }
            } else {
                if ( j_register.contains( uri ) ) {
                   for( auto it = j_register.begin(); it != j_register.end(); it++ ) {
                       if ( it->toString() == uri ) {
                           j_register.removeAt( it.i );
                       }
                   }
                }
            }
        } else {
            if (  ui->tableWidget->item( item->row(), 4)->checkState() == Qt::Checked ) {
                if ( !j_publish.contains( uri ) ) {
                   j_publish.push_back( uri );
                }
            } else {
                if ( j_subscribe.contains( uri ) ) {
                   for( auto it = j_subscribe.begin(); it != j_subscribe.end(); it++ ) {
                       if ( it->toString() == uri ) {
                           j_subscribe.removeAt( it.i );
                       }
                   }
                }
            }
            if (  ui->tableWidget->item( item->row(), 5)->checkState() == Qt::Checked ) {
                if ( !j_publish.contains( uri ) ) {
                   j_publish.push_back( uri );
                }
            } else {
                if ( j_subscribe.contains( uri ) ) {
                   for( auto it = j_subscribe.begin(); it != j_subscribe.end(); it++ ) {
                       if ( it->toString() == uri ) {
                           j_subscribe.removeAt( it.i );
                       }
                   }
                }
            }
        }
        j_m["call"] = j_call;
        j_m["register"] = j_register;
        j_m["publish"] = j_publish;
        j_m["subscribe"] = j_subscribe;
        j_wamp_config["module"] = j_m;
        QFile f("/opt/swift-bot/modules/"+module_name+"/wamp.json");
        if ( f.open( QFile::ReadWrite ) ) {
            f.resize(0);
            f.write( QJsonDocument( j_wamp_config ).toJson( QJsonDocument::Indented ));
        }
        f.close();
    });
    this->show();
}
