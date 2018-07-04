#include "widget.h"
#include "ui_widget.h"
#include "dlgsettings.h"
#include "generator.h"
#include <QClipboard>
#include <QStandardPaths>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget), settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/java-generator.ini", QSettings::IniFormat)
{
    ui->setupUi(this);
    refresh();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::refresh()
{
    hash.clear();
    int domains = settings.beginReadArray("domains");
    for(int i = 0;i<domains;i++) {
        settings.setArrayIndex(i);
        hash.insert(settings.value("domainName").toString(), settings.value("domainType").toString());
    }
    settings.endArray();
}

void Widget::on_btnSettings_clicked()
{
    DlgSettings dlgSettings;
    if(dlgSettings.exec()) {
        refresh();
    }
}

void Widget::on_btnDTO_clicked()
{
    QString dto = Generator::generateDTO(ui->textEdit->toPlainText(), hash);
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(dto);
}

void Widget::on_btnModel_clicked()
{
    QString model = Generator::generateModel(ui->textEdit->toPlainText(), hash);
    QGuiApplication::clipboard()->clear();
    QGuiApplication::clipboard()->setText(model);
}
