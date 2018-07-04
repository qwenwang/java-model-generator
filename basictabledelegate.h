#ifndef BASICTABLEDELEGATE_H
#define BASICTABLEDELEGATE_H

#include <QStyledItemDelegate>
#include "basetable.h"

class BasicTableDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    BasicTableDelegate(QWidget *parent);
    void setColumns(QList<ColumnConfig> &columns);
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const Q_DECL_OVERRIDE;

protected:
    QList<ColumnConfig> columns;
};

#endif // BASICTABLEDELEGATE_H
