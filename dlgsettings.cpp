#include "dlgsettings.h"
#include "ui_dlgsettings.h"
#include <QStandardPaths>
#include "basetable.h"

DlgSettings::DlgSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DlgSettings), settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/java-generator.ini", QSettings::IniFormat)
{
    ui->setupUi(this);
    int domains = settings.beginReadArray("domains");
    QList<ColumnConfig> columns;
    ColumnConfig column1("domainName",tr("domain名称"), 200);
    column1.setEditMode(StringEdit);
    columns.append(column1);
    ColumnConfig column2("domainType", tr("domain对应类型"), 200);
    column2.setEditMode(StringEdit);
    columns.append(column2);
    ui->tableView->init(columns, &domainDataList);
    QList<BaseEntity*> tempList;
    for(int i = 0;i<domains;i++) {
        settings.setArrayIndex(i);
        Domain domain;
        domain.setDomainName(settings.value("domainName").toString());
        domain.setDomainType(settings.value("domainType").toString());
        domainRawList.append(domain);
        tempList.append(&domainRawList.last());
    }
    settings.endArray();
    ui->tableView->setNewDataList(tempList);
}

DlgSettings::~DlgSettings()
{
    delete ui;
}

void DlgSettings::on_btnAdd_clicked()
{
    Domain domain;
    domainRawList.append(domain);
    ui->tableView->insertData(&domainRawList.last());
}

void DlgSettings::on_btnDelete_clicked()
{
    if(ui->tableView->currentIndex().isValid()) {
        ui->tableView->removeData(ui->tableView->currentIndex().row());
    }
}

void DlgSettings::on_btnOk_clicked()
{
    settings.beginWriteArray("domains");
    int index = 0;
    foreach(BaseEntity *baseEntity, domainDataList) {
        Domain *domain = static_cast<Domain*>(baseEntity);
        if(domain->getDomainType() == "" || domain->getDomainName() == "") {
            continue;
        }
        settings.setArrayIndex(index++);
        settings.setValue("domainName", domain->getDomainName());
        settings.setValue("domainType", domain->getDomainType());
    }
    settings.endArray();
    accept();
}

void DlgSettings::on_btnCancel_clicked()
{
    reject();
}
