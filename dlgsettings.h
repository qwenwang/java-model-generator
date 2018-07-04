#ifndef DLGSETTINGS_H
#define DLGSETTINGS_H

#include <QDialog>
#include <QSettings>
#include "basetable.h"
#include "domain.h"

namespace Ui {
class DlgSettings;
}

class DlgSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSettings(QWidget *parent = 0);
    ~DlgSettings();

private slots:
    void on_btnAdd_clicked();
    void on_btnDelete_clicked();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();

private:
    Ui::DlgSettings *ui;
    QSettings settings;
    QList<Domain> domainRawList;
    QList<BaseEntity*> domainDataList;
};

#endif // DLGSETTINGS_H
