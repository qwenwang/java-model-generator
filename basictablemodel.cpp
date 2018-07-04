#include "basictablemodel.h"
#include <QRegExp>
#include <QLatin1Char>
#include <QDateTime>
#include <QDebug>

QString BasicTableModel::sortColumn = "";
Qt::SortOrder BasicTableModel::sortOrder = Qt::AscendingOrder;

BasicTableModel::BasicTableModel()
{
    this->dataList = 0;
}

BasicTableModel::~BasicTableModel()
{

}

void BasicTableModel::setDataList(QList<BaseEntity *> *dataList)
{
    this->dataList = dataList;
}

void BasicTableModel::setColumns(QList<ColumnConfig> &columns)
{
    this->columns = columns;
}

Qt::ItemFlags BasicTableModel::flags(const QModelIndex & index) const
{
    if(!index.isValid()){
        return 0;
    }
    ColumnConfig column = columns.at(index.column());
    if(column.getCheckBoxColumn()){
        return Qt::ItemIsUserCheckable | QAbstractTableModel::flags(index);
    }
    else if(column.getEditMode() == EditMode::Null){
        return QAbstractTableModel::flags(index);
    }
    else {
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    }
}

QVariant BasicTableModel::data(const QModelIndex &index, int role) const
{
    if(dataList == 0){
        return QVariant();
    }
    if(!index.isValid()){
        return QVariant();
    }
    if((*dataList)[index.row()]->isValid() == false){
        return QVariant();
    }
    if(role == Qt::DisplayRole){
        QString columnText = const_cast<BasicTableModel*>(this)->columns[index.column()].getColumnText();
        if(columnText != ""){
            return columnText;
        }
        else if(const_cast<BasicTableModel*>(this)->columns[index.column()].getCheckBoxColumn()){
            return "";
        }
        QString dataFormat = const_cast<BasicTableModel*>(this)->columns[index.column()].getDataFormat();
        QVariant data = (*dataList)[index.row()]->get(columns[index.column()].getId());
        if(dataFormat == ""){
            return data.toString();
        }
        QRegExp regExp("[#*,?#*]*,?[0*,?0*]*\\.*[0]*");
        if(regExp.exactMatch(dataFormat)){
            int index = dataFormat.indexOf(".");
            int precision = 0;
            int length = dataFormat.count("0");
            if(index != -1){
                precision = dataFormat.length()-1-index;
                length += 1;
            }
            if(dataFormat.contains(",")){
                return QString("%L1").arg(data.toDouble(), length, 'f', precision, QLatin1Char('0'));
            } else {
                return QString("%1").arg(data.toDouble(), length, 'f', precision, QLatin1Char('0'));
            }
        }
        else {
            if(dataFormat.contains(QRegExp("y|M|d"))){
                return data.toDateTime().toString(dataFormat);
            } else {
                return data.toTime().toString(dataFormat);
            }
        }
    }
    else if(role == Qt::EditRole){
        return (*dataList)[index.row()]->get(columns[index.column()].getId());
    } else if(role == Qt::CheckStateRole){
        if(columns[index.column()].getCheckBoxColumn()){
            return (*dataList)[index.row()]->get(columns[index.column()].getId()).toInt();
        } else {
            return QVariant();
        }
    }
    else if(role == Qt::TextAlignmentRole){
        if(columns[index.column()].getAlignment() == HorizontalAlignment::AlignLeft){
            return QVariant(Qt::AlignLeft|Qt::AlignVCenter);
        }
        else if(columns[index.column()].getAlignment() == HorizontalAlignment::AlignRight){
            return QVariant(Qt::AlignRight|Qt::AlignVCenter);
        }
        else if(columns[index.column()].getAlignment() == HorizontalAlignment::AlignCenter){
            return QVariant(Qt::AlignHCenter|Qt::AlignVCenter);
        }
    }
    return QVariant();
}

bool BasicTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(dataList == 0){
        return 0;
    }
    if(!index.isValid()){
        return true;
    }
    if(role == Qt::EditRole){
        (*dataList)[index.row()]->set(columns[index.column()].getId(), value);
        emit dataEditFinished(index);
    }
    else if(role == Qt::CheckStateRole){
        if(columns[index.column()].getCheckBoxColumn()){
            (*dataList)[index.row()]->set(columns[index.column()].getId(), value.toInt()==Qt::Checked?Qt::Checked:Qt::Unchecked);
            if(value.toInt() == Qt::Checked){
                emit checkBoxDataChanged(index, QModelIndex());
            } else {
                emit checkBoxDataChanged(QModelIndex(), index);
            }
        }
    }
    return true;
}

void BasicTableModel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected, bool hasCheckBoxColumn)
{
    if(hasCheckBoxColumn){
        emit layoutAboutToBeChanged();
    }
    QModelIndexList list = selected.indexes();
    QSet<int> rows;
    QList<int> tRows;
    for(int i = 0;i<list.size();i++){
        rows.insert(list.at(i).row());
    }
    tRows = rows.toList();
    for(int i = 0;i<tRows.size();i++){
        if(tRows.at(i) < dataList->size()){
            (*dataList)[tRows.at(i)]->set("checkBox", Qt::Checked);
        }
    }
    list = deselected.indexes();
    rows.clear();
    for(int i = 0;i<list.size();i++){
        rows.insert(list.at(i).row());
    }
    tRows = rows.toList();
    for(int i = 0;i<tRows.size();i++){
        if(tRows.at(i) < dataList->size()){
            (*dataList)[tRows.at(i)]->set("checkBox", Qt::Unchecked);
        }
    }
    if(hasCheckBoxColumn){
        emit layoutChanged();
    }
}

QVariant BasicTableModel::headerData(int section, Qt::Orientation orientation,
                            int role) const
{
    if(orientation == Qt::Horizontal){
        switch(role){
        case Qt::DisplayRole:
            return columns[section].getName();
        case Qt::SizeHintRole:
            return QSize(columns[section].getWidth(), 28);
        case Qt::TextAlignmentRole:
            if(columns[section].getAlignment() == HorizontalAlignment::AlignLeft){
                return Qt::AlignLeft;
            }
            else if(columns[section].getAlignment() == HorizontalAlignment::AlignRight){
                return Qt::AlignRight;
            }
            else if(columns[section].getAlignment() == HorizontalAlignment::AlignCenter){
                return Qt::AlignCenter;
            }
        case Qt::CheckStateRole:
            if(columns[section].getCheckBoxColumn()){
                return Qt::Unchecked;
            } else {
                return QVariant();
            }
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

int BasicTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if(dataList == 0){
        return 0;
    }
    return dataList->size();
}

int BasicTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return columns.size();
}

void BasicTableModel::setNewDataList(QList<BaseEntity *> &dataList)
{
    if(this->dataList->size() == dataList.size()){
        emit layoutAboutToBeChanged();
        this->dataList->clear();
        this->dataList->append(dataList);
        emit layoutChanged();
    }
    else if(this->dataList->size() > dataList.size()){
        beginRemoveRows(QModelIndex(), dataList.size(), this->dataList->size()-1);
        this->dataList->clear();
        this->dataList->append(dataList);
        endRemoveRows();
    }
    else {
        beginInsertRows(QModelIndex(), this->dataList->size(), dataList.size()-1);
        this->dataList->clear();
        this->dataList->append(dataList);
        endInsertRows();
    }
    emit aggreDataChanged();
}

void BasicTableModel::insertData(BaseEntity *data, int row)
{
    if(row < 0){
        row = dataList->size()+row;
    }
    row++;
    if(row < 0 || row > dataList->size()){
        return;
    }
    beginInsertRows(QModelIndex(), row, row);
    this->dataList->insert(row, data);
    endInsertRows();
    emit aggreDataChanged();
}

void BasicTableModel::insertDataList(QList<BaseEntity*> &tempList, int row)
{
    if(row < 0){
        row = dataList->size()+row;
    }
    row++;
    if(row < 0 || row > dataList->size()){
        return;
    }
    beginInsertRows(QModelIndex(), row, row+tempList.size()-1);
    for(int i = 0;i<tempList.size();i++){
        this->dataList->insert(row+i, tempList.at(i));
    }
    endInsertRows();
    emit aggreDataChanged();
}

void BasicTableModel::update()
{
    emit layoutChanged();
    emit aggreDataChanged();
}

void BasicTableModel::updateData(BaseEntity *data, int row)
{
    if(row < 0){
        row = dataList->size()+row;
    }
    if(row < 0 || row >= dataList->size()){
        return;
    }
    emit layoutAboutToBeChanged();
    (*dataList)[row] = data;
    emit layoutChanged();
    emit aggreDataChanged();
}

void BasicTableModel::updateDataList(QList<QPair<int, BaseEntity *> > &tempList)
{
    emit layoutAboutToBeChanged();
    for(int i = 0;i<tempList.size();i++){
        int row = tempList[i].first;
        BaseEntity *data = tempList[i].second;
        if(row < 0){
            row = dataList->size()+row;
        }
        if(row < 0 || row >= dataList->size()){
            continue;
        }
        (*dataList)[row] = data;
    }
    emit layoutChanged();
    emit aggreDataChanged();
}

void BasicTableModel::removeData(int row)
{
    if(row < 0){
        row = dataList->size()+row;
    }
    if(row < 0 || row >= dataList->size()){
        return;
    }
    beginRemoveRows(QModelIndex(), row, row);
    this->dataList->removeAt(row);
    endRemoveRows();
    emit aggreDataChanged();
}

void BasicTableModel::removeDataList(QList<int> &rows)
{
    beginRemoveRows(QModelIndex(), dataList->size()-rows.size(), dataList->size()-1);
    qSort(rows.begin(), rows.end(), intMoreThan);
    for(int i = 0;i<rows.size();i++){
        int row = rows.at(i);
        if(row < 0){
            row = dataList->size()+row;
        }
        if(row < 0 || row >= dataList->size()){
            continue;
        }
        this->dataList->removeAt(row);
    }
    endRemoveRows();
}

void BasicTableModel::removeDataList(int row, int size)
{
    beginRemoveRows(QModelIndex(), dataList->size()-size, dataList->size()-1);
    if(row < 0){
        row = dataList->size()+row;
    }
    if(row >= 0){
        for(int i = 0;i<size;i++){
            this->dataList->removeAt(row);
        }
    }
    endRemoveRows();
}

QList<BaseEntity*>* BasicTableModel::getDataList()
{
    return dataList;
}

void BasicTableModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();
    if(column >= 0 && column < columns.size()){
        sortColumn = columns[column].getId();
        sortOrder = order;
        qSort(dataList->begin(), dataList->end(), columnLessThan);
    }
    emit layoutChanged();
    emit sorted();
}

bool BasicTableModel::columnLessThan(BaseEntity *entity1, BaseEntity *entity2)
{
    if(entity1->get(sortColumn) < entity2->get(sortColumn)){
        return sortOrder == Qt::AscendingOrder?true:false;
    } else {
        return sortOrder == Qt::AscendingOrder?false:true;
    }
}

bool BasicTableModel::intMoreThan(int a, int b)
{
    if(a > b){
        return true;
    } else {
        return false;
    }
}
