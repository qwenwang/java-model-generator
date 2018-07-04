#ifndef MYTABLEMODEL_H
#define MYTABLEMODEL_H

#include <QAbstractTableModel>
#include "basetable.h"
#include <QVariant>
#include <QItemSelection>

class BasicTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    BasicTableModel();
    virtual ~BasicTableModel();
    virtual void setDataList(QList<BaseEntity*> *dataList);
    virtual QList<BaseEntity*>* getDataList();
    virtual void setColumns(QList<ColumnConfig> &columns);
    virtual void setNewDataList(QList<BaseEntity*> &data);
    virtual void insertData(BaseEntity *data, int row);
    virtual void insertDataList(QList<BaseEntity*> &dataList, int row);
    virtual void update();
    virtual void updateData(BaseEntity *data, int row);
    virtual void updateDataList(QList<QPair<int, BaseEntity*> > &dataList);
    virtual void removeData(int row);
    virtual void removeDataList(QList<int> &rows);
    virtual void removeDataList(int row, int size);
    virtual Qt::ItemFlags flags(const QModelIndex & index) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

signals:
    void checkBoxDataChanged(const QModelIndex &selected, const QModelIndex &deselected);
    void sorted();
    void dataEditFinished(const QModelIndex &index);
    void aggreDataChanged();

public slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected, bool hasCheckBoxColumn);

protected:
    QList<BaseEntity*> *dataList;
    QList<ColumnConfig> columns;

    static QString sortColumn;
    static Qt::SortOrder sortOrder;
    static bool columnLessThan(BaseEntity *entity1, BaseEntity *entity2);
    static bool intMoreThan(int a, int b);
};

#endif // MYTABLEMODEL_H
