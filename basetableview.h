#ifndef BASETABLEVIEW_H
#define BASETABLEVIEW_H

#include <QTableView>
#include <QHeaderView>
#include <QColor>
#include <QList>
#include <QModelIndex>
#include "basictablemodel.h"
#include "basictabledelegate.h"

class BaseHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    BaseHeaderView(Qt::Orientation orientation, QWidget *parent = 0);
    void setCheckBoxColumn(int logicalIndex);
    void setUnsortableColumns(QList<int> logicalIndexes);
    bool getCheckBoxState();

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    void mousePressEvent(QMouseEvent *event);

signals:
    void checkBoxStateChanged(bool state);

public slots:
    void setCheckBoxState(bool state);

private:
    bool checkBoxState;
    int checkBoxColumn;
    QList<int> unsortableColumns;

};

class BaseTableView : public QTableView
{
    Q_OBJECT
public:
    enum TableRole {
        MainTable, FrozenTable, AggreTable
    };
    explicit BaseTableView(QWidget *parent = 0);
    ~BaseTableView();
    void setTableModel(BasicTableModel *basicTableModel);
    void setTableDelegate(BasicTableDelegate *basicTableDelegate);
    void init(QList<ColumnConfig> &columns, QList<BaseEntity*> *dataList = 0);
    void setNewDataList(QList<BaseEntity*> &dataList);
    void insertData(BaseEntity* data, int row = -1);
    void insertDataList(QList<BaseEntity*> &dataList, int row = -1);
    void update();
    void updateData(BaseEntity* data, int row = -1);
    void updateDataList(QList<QPair<int, BaseEntity*> > &dataList);
    void removeData(int row = -1);
    void removeDataList(QList<int> &rows);
    void removeDataList(int row, int size = 1);
    void setAlternatingRowColor(QColor color = qRgb(220, 220, 220));
    void setTableRowHeight(int height);
    QList<BaseEntity*>* getDataList();
    QList<BaseEntity*> getSelectedDatas();
    QList<BaseEntity*> getCheckBoxDatas();
    BasicTableModel *getModel();

    //以下接口仅用于固定列和汇总行
    void setTableRole(TableRole tableRole);
    void setAggreTableModel(BasicTableModel *basicTableModel);
    void initColumns(QList<ColumnConfig> &columns);
    void setNeedAggreRow();
    void setAggreData(QList<BaseEntity*> *aggreData);
    void setSortingEnabled(bool enable);

signals:
    void horizontalSectionResized();
    void dataEditFinished(const QModelIndex &index);

protected:
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    TableRole tableRole;
    BasicTableModel *basicTableModel;
    BasicTableDelegate *basicTableDelegate;
    QList<BaseEntity*> *dataList;
    BaseTableView *frozenTableView;
    BaseTableView *aggreTableView;
    QList<ColumnConfig> columns;
    QList<ColumnConfig> frozenColumns;
    bool needAggreRow;
    QList<BaseEntity*> *aggreData;
    BasicTableModel *aggreTableModel;
    BaseHeaderView *baseHeaderView;
    bool sortSelectionChanged;
    bool headerSelectionChanged;
    bool hasCheckBoxColumn;
    bool checkBoxRelativeToSelection;

    void initFrozenTableView();
    void initAggreTableView();

private slots:
    void updateSectionWidth(int logicalIndex, int /*oldSize*/, int newSize);
    void updateSectionHeight(int logicalIndex, int /*oldSize*/, int newSize);

    void updateFrozenTableViewGeometry();
    void updateAggreTableViewGeometry();

    void aggreDataChanged(const QModelIndex &index);
    void aggreDataChanged();

    void checkBoxDataChanged(const QModelIndex &selected, const QModelIndex &deselected);
    void headerCheckBoxDataChanged(bool state);
    void mySelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void sorted();
};

#endif // BASETABLEVIEW_H
