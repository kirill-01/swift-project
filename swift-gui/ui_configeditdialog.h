/********************************************************************************
** Form generated from reading UI file 'configeditdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIGEDITDIALOG_H
#define UI_CONFIGEDITDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QTextBrowser>

QT_BEGIN_NAMESPACE

class Ui_ConfigEditDialog
{
public:
    QGridLayout *gridLayout;
    QTextBrowser *textBrowser;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ConfigEditDialog)
    {
        if (ConfigEditDialog->objectName().isEmpty())
            ConfigEditDialog->setObjectName(QString::fromUtf8("ConfigEditDialog"));
        ConfigEditDialog->resize(750, 673);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/swift/icon.png"), QSize(), QIcon::Normal, QIcon::Off);
        ConfigEditDialog->setWindowIcon(icon);
        gridLayout = new QGridLayout(ConfigEditDialog);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        textBrowser = new QTextBrowser(ConfigEditDialog);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setReadOnly(false);
        textBrowser->setOverwriteMode(true);

        gridLayout->addWidget(textBrowser, 0, 0, 1, 1);

        buttonBox = new QDialogButtonBox(ConfigEditDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 0, 1, 1, 1);


        retranslateUi(ConfigEditDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ConfigEditDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ConfigEditDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ConfigEditDialog);
    } // setupUi

    void retranslateUi(QDialog *ConfigEditDialog)
    {
        ConfigEditDialog->setWindowTitle(QCoreApplication::translate("ConfigEditDialog", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ConfigEditDialog: public Ui_ConfigEditDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIGEDITDIALOG_H
