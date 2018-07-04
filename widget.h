#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSettings>
#include <QHash>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void refresh();

private slots:
    void on_btnSettings_clicked();
    void on_btnModel_clicked();
    void on_btnDTO_clicked();

private:
    Ui::Widget *ui;
    QSettings settings;
    QHash<QString, QString> hash;
};

#endif // WIDGET_H
