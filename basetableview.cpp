#include "basetableview.h"
#include <QHeaderView>
#include <QScrollBar>
#include <QScrollArea>
#include <QPainter>
#include <QMouseEvent>
#include <QItemSelectionModel>

BaseHeaderView::BaseHeaderView(Qt::Orientation orientation, QWidget *parent)
    :QHeaderView(orientation, parent)
{
    checkBoxState = false;
    checkBoxColumn = -1;
}

void BaseHeaderView::setCheckBoxColumn(int logicalIndex)
{
    checkBoxColumn = logicalIndex;
}

void BaseHeaderView::setUnsortableColumns(QList<int> logicalIndexes)
{
    unsortableColumns = logicalIndexes;
}

bool BaseHeaderView::getCheckBoxState()
{
    return checkBoxState;
}

void BaseHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (checkBoxColumn != -1 && logicalIndex == checkBoxColumn)
    {
      QStyleOptionButton option;
      option.rect = QRect(0, rect.height()/2-10, 20, 20);
      if (checkBoxState){
        option.state = QStyle::State_Enabled|QStyle::State_On;
      }
      else {
        option.state = QStyle::State_Enabled|QStyle::State_Off;
      }
      this->style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter);
    }
}

void BaseHeaderView::mousePressEvent(QMouseEvent *event)
{
    QRect rect(0, height()/2-10, 20, 20);
    int logicalIndex = logicalIndexAt(event->pos());
    if(checkBoxColumn != -1 && logicalIndex == checkBoxColumn){
        if(rect.contains(event->pos())){
            checkBoxState = checkBoxState==true?false:true;
            emit checkBoxStateChanged(checkBoxState);
            updateSection(logicalIndex);
        }
    } else if(!unsortableColumns.contains(logicalIndex) || cursor().shape() == Qt::SplitHCursor){
        QHeaderView::mousePressEvent(event);
    }
}

void BaseHeaderView::setCheckBoxState(bool state)
{
    checkBoxState = state;
    updateSection(checkBoxColumn);
}

BaseTableView::BaseTableView(QWidget *parent)
    : QTableView(parent)
{
    baseHeaderView = new BaseHeaderView(Qt::Horizontal, this);
    baseHeaderView->setSectionResizeMode(QHeaderView::Interactive);
    baseHeaderView->setSectionsClickable(true);
    setHorizontalHeader(baseHeaderView);
    verticalHeader()->setMinimumWidth(25);
    basicTableModel = new BasicTableModel();
    connect(basicTableModel, SIGNAL(dataEditFinished(QModelIndex)), this, SIGNAL(dataEditFinished(QModelIndex)));
    basicTableDelegate = new BasicTableDelegate(parent);
    setSelectionBehavior(SelectionBehavior::SelectRows);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    frozenTableView = 0;
    needAggreRow = false;
    aggreTableView = 0;
    aggreTableModel = 0;
    aggreData = 0;
    dataList = 0;
    tableRole = TableRole::MainTable;
    sortSelectionChanged = false;
    headerSelectionChanged = false;
    hasCheckBoxColumn = false;
    checkBoxRelativeToSelection = false;
}

BaseTableView::~BaseTableView()
{
    delete baseHeaderView;
    if(dataList != 0){
        delete dataList;
    }
    if(tableRole == TableRole::MainTable){
        delete basicTableModel;
        if(aggreData != 0){
            if(aggreData->size() != 0){
                for(int i = 0;i<aggreData->size();i++){
                    delete aggreData->at(i);
                }
            }
            delete aggreData;
        }
    }
}

void BaseTableView::setTableRole(TableRole tableRole)
{
    this->tableRole = tableRole;
}

void BaseTableView::setTableModel(BasicTableModel *basicTableModel)
{
    delete this->basicTableModel;
    this->basicTableModel = basicTableModel;
    connect(this->basicTableModel, SIGNAL(dataEditFinished(QModelIndex)), this, SIGNAL(dataEditFinished(QModelIndex)));
}

void BaseTableView::setAggreTableModel(BasicTableModel *basicTableModel)
{
    delete this->aggreTableModel;
    this->aggreTableModel = basicTableModel;
}

void BaseTableView::setTableDelegate(BasicTableDelegate *basicTableDelegate)
{
    delete this->basicTableDelegate;
    this->basicTableDelegate = basicTableDelegate;
    setItemDelegate(basicTableDelegate);
}

void BaseTableView::init(QList<ColumnConfig> &columns, QList<BaseEntity *> *dataList)
{
    if(dataList == 0){
        dataList = new QList<BaseEntity*>();
        this->dataList = dataList;
    }
    for(int i = 0;i<columns.size();i++){
        if(columns[i].getCheckBoxColumn()){
            if(columns[i].getRelativeToSelection()){
                columns[i].setId("checkBox");
            } else {
                columns[i].setId("checkBox2");
            }
            columns[i].setName("");
            columns[i].setWidth(20);
            columns[i].setEditMode(EditMode::Null);
            columns[i].setResizable(false);
            columns[i].setSortable(false);
            baseHeaderView->setCheckBoxColumn(i);
        }
    }
    basicTableModel->setDataList(dataList);
    basicTableModel->setColumns(columns);
    basicTableDelegate->setColumns(columns);
    if(tableRole != TableRole::AggreTable){
        connect(basicTableModel, SIGNAL(aggreDataChanged()), this, SLOT(aggreDataChanged()));
        connect(basicTableModel, SIGNAL(dataEditFinished(QModelIndex)), this, SLOT(aggreDataChanged(QModelIndex)));
    }
    setModel(basicTableModel);
    setItemDelegate(basicTableDelegate);
    initColumns(columns);
}

void BaseTableView::initColumns(QList<ColumnConfig> &columns)
{
    this->columns = columns;
    bool frozenContinued = true;
    bool sortable = false;
    frozenColumns.clear();
    QList<int> unsortableColumns;
    for(int i = 0;i<columns.size();i++){
        if(columns[i].getCheckBoxColumn()){
            baseHeaderView->setCheckBoxColumn(i);
            hasCheckBoxColumn = true;
            if(columns[i].getRelativeToSelection()){
                checkBoxRelativeToSelection = true;
            }
        }
        setColumnHidden(i, columns[i].getHidden());
        setColumnWidth(i, columns[i].getWidth());
        if(!columns[i].getResizable()){
            horizontalHeader()->setSectionResizeMode(i, QHeaderView::Fixed);
        }
        if(columns[i].getSortable()){
            sortable = true;
        } else {
            unsortableColumns.append(i);
        }
        if(frozenContinued && columns[i].getColumnFrozen()){
            ColumnConfig frozenColumn = columns[i];
            frozenColumn.setColumnFrozen(false);
            frozenColumns.append(frozenColumn);
        } else {
            frozenContinued = false;
            if(tableRole != TableRole::AggreTable && columns[i].getColumnAggre()){
                needAggreRow = true;
            }
        }
    }
    baseHeaderView->setUnsortableColumns(unsortableColumns);
    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(mySelectionChanged(QItemSelection,QItemSelection)));
    if(hasCheckBoxColumn == true){
        connect(basicTableModel, SIGNAL(checkBoxDataChanged(QModelIndex,QModelIndex)), this, SLOT(checkBoxDataChanged(QModelIndex,QModelIndex)));
        connect(baseHeaderView, SIGNAL(checkBoxStateChanged(bool)), this, SLOT(headerCheckBoxDataChanged(bool)));

    }
    if(needAggreRow != 0){
        aggreTableView = new BaseTableView(this);
        aggreTableView->setTableRole(TableRole::AggreTable);
        aggreTableView->setStyleSheet("QTableView { border: none; }");

        aggreTableModel = new BasicTableModel();

        connect(horizontalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(updateSectionWidth(int,int,int)));
        connect(verticalHeader(), SIGNAL(sectionResized(int,int,int)), this, SLOT(updateSectionHeight(int,int,int)));

        if(tableRole == TableRole::MainTable){
            aggreData = new QList<BaseEntity*>();
            BaseEntity *baseEntity = new BaseEntity();
            for(int i = 0;i<basicTableModel->getDataList()->size();i++){
                BaseEntity *entity = basicTableModel->getDataList()->at(i);
                for(int j = 0;j<columns.size();j++){
                    if(columns[j].getColumnAggre()){
                        baseEntity->set(columns[j].getId(), baseEntity->get(columns[j].getId()).toDouble()+entity->get(columns[j].getId()).toDouble());
                    }
                }
            }
            for(int i = 0;i<columns.size();i++){
                if(columns[i].getColumnAggre() == false && columns[i].getColumnAggreName().length() > 0){
                    baseEntity->set(columns[i].getId(), columns[i].getColumnAggreName());
                }
            }
            aggreData->append(baseEntity);
        }

        initAggreTableView();

        connect(aggreTableView->horizontalScrollBar(), SIGNAL(valueChanged(int)), horizontalScrollBar(), SLOT(setValue(int)));
        connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), aggreTableView->horizontalScrollBar(), SLOT(setValue(int)));
    }
    if(frozenColumns.size() != 0){
        QPalette newPalette = palette();
        newPalette.setColor(QPalette::Inactive, QPalette::Highlight, newPalette.color(QPalette::Highlight));
        newPalette.setColor(QPalette::Inactive, QPalette::HighlightedText, newPalette.color(QPalette::HighlightedText));
        setPalette(newPalette);
        frozenTableView = new BaseTableView(this);
        frozenTableView->setTableRole(TableRole::FrozenTable);
        frozenTableView->setStyleSheet("QTableView { border: none; }");
        frozenTableView->setPalette(palette());
        if(needAggreRow){
            frozenTableView->setNeedAggreRow();
            frozenTableView->setAggreData(aggreData);
        }

        initFrozenTableView();

        connect(frozenTableView->verticalScrollBar(), SIGNAL(valueChanged(int)), verticalScrollBar(), SLOT(setValue(int)));
        connect(verticalScrollBar(), SIGNAL(valueChanged(int)), frozenTableView->verticalScrollBar(), SLOT(setValue(int)));
        if(needAggreRow){
            connect(frozenTableView, SIGNAL(horizontalSectionResized()), this, SLOT(updateAggreTableViewGeometry()));
        }
    }
    if(sortable){
        setSortingEnabled(true);
        connect(basicTableModel, SIGNAL(sorted()), this, SLOT(sorted()));
    }
}

void BaseTableView::setNewDataList(QList<BaseEntity *> &dataList)
{
    disconnect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(mySelectionChanged(QItemSelection,QItemSelection)));
    selectionModel()->clear();
    basicTableModel->setNewDataList(dataList);
    connect(selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(mySelectionChanged(QItemSelection,QItemSelection)));
    repaint();
    if(frozenTableView != 0){
        updateFrozenTableViewGeometry();
    }
    if(aggreTableView != 0){
        updateAggreTableViewGeometry();
    }
}

void BaseTableView::insertData(BaseEntity *data, int row)
{
    basicTableModel->insertData(data, row);
    repaint();
    if(frozenTableView != 0){
        updateFrozenTableViewGeometry();
    }
    if(aggreTableView != 0){
        updateAggreTableViewGeometry();
    }
}

void BaseTableView::insertDataList(QList<BaseEntity *> &dataList, int row)
{
    basicTableModel->insertDataList(dataList, row);
    repaint();
    if(frozenTableView != 0){
        updateFrozenTableViewGeometry();
    }
    if(aggreTableView != 0){
        updateAggreTableViewGeometry();
    }
}

void BaseTableView::update()
{
    basicTableModel->update();
}

void BaseTableView::updateData(BaseEntity *data, int row)
{
    basicTableModel->updateData(data, row);
}

void BaseTableView::updateDataList(QList<QPair<int, BaseEntity *> > &dataList)
{
    basicTableModel->updateDataList(dataList);
}

void BaseTableView::removeData(int row)
{
    basicTableModel->removeData(row);
    repaint();
    if(frozenTableView != 0){
        updateFrozenTableViewGeometry();
    }
    if(aggreTableView != 0){
        updateAggreTableViewGeometry();
    }
}

void BaseTableView::removeDataList(QList<int> &rows)
{
    basicTableModel->removeDataList(rows);
}

void BaseTableView::removeDataList(int row, int size)
{
    basicTableModel->removeDataList(row, size);
}

void BaseTableView::setAlternatingRowColor(QColor color)
{
    QPalette newPalette = palette();
    newPalette.setColor(QPalette::AlternateBase, color);
    setPalette(newPalette);
    setAlternatingRowColors(true);
    if(frozenTableView != 0){
        frozenTableView->setPalette(palette());
        frozenTableView->setAlternatingRowColors(true);
    }
}

void BaseTableView::checkBoxDataChanged(const QModelIndex &selected, const QModelIndex &deselected)
{
    if(checkBoxRelativeToSelection){
        if(selected.isValid()){
            selectionModel()->select(selected, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        if(deselected.isValid()){
            selectionModel()->select(deselected, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
        }
    } else {
        if(baseHeaderView->getCheckBoxState()){
            if(deselected.isValid()){
                baseHeaderView->setCheckBoxState(false);
            }
        } else {
            if(selected.isValid()){
                for(int i = 0;i<getDataList()->size();i++){
                    if(getDataList()->at(i)->get("checkBox2").toInt() == Qt::Unchecked){
                        return;
                    }
                }
                baseHeaderView->setCheckBoxState(true);
            }
        }
    }
}

void BaseTableView::mySelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(sortSelectionChanged){
        sortSelectionChanged = false;
    } else {
        basicTableModel->selectionChanged(selected, deselected, checkBoxRelativeToSelection);
    }
    if(headerSelectionChanged){
        headerSelectionChanged = false;
    } else if(checkBoxRelativeToSelection){
        if(selectionModel()->selectedRows().size() == basicTableModel->getDataList()->size()){
            baseHeaderView->setCheckBoxState(true);
        } else {
            baseHeaderView->setCheckBoxState(false);
        }
    }

}

void BaseTableView::headerCheckBoxDataChanged(bool state)
{
    if(checkBoxRelativeToSelection){
        if(state){
            selectAll();
        } else {
            selectionModel()->clear();
        }
        headerSelectionChanged = true;
    } else {
        for(int i = 0;i<getDataList()->size();i++){
            if(state){
                getDataList()->at(i)->set("checkBox2", Qt::Checked);
            } else {
                getDataList()->at(i)->set("checkBox2", Qt::Unchecked);
            }
        }
        update();
    }
}

void BaseTableView::sorted()
{
    QList<BaseEntity*> *list = basicTableModel->getDataList();
    for(int i = 0;i<list->size();i++){
        if(list->at(i)->get("checkBox").toInt() == Qt::Checked){
            selectionModel()->select(basicTableModel->index(i, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
        } else {
            selectionModel()->select(basicTableModel->index(i, 0), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
        }
    }
}

void BaseTableView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    if(frozenTableView != 0){
        updateFrozenTableViewGeometry();
    }
    if(aggreTableView != 0){
        updateAggreTableViewGeometry();
        setViewportMargins(verticalHeader()->width(), horizontalHeader()->height(), 0, aggreTableView->rowHeight(0));
    }
}

void BaseTableView::updateSectionWidth(int logicalIndex, int /*oldSize*/, int newSize)
{
    if(frozenTableView != 0){
        if(logicalIndex < frozenColumns.size()){
            frozenTableView->setColumnWidth(logicalIndex, newSize);
            updateFrozenTableViewGeometry();
        }
    }
    if(aggreTableView != 0){
        if(logicalIndex >= frozenColumns.size()){
            aggreTableView->setColumnWidth(logicalIndex, newSize);
            updateAggreTableViewGeometry();
            emit horizontalSectionResized();
        }
    }
}

void BaseTableView::updateSectionHeight(int logicalIndex, int /*oldSize*/, int newSize)
{
    if(frozenTableView != 0){
        frozenTableView->setRowHeight(logicalIndex, newSize);
    }
}

void BaseTableView::initFrozenTableView()
{
    frozenTableView->setTableModel(basicTableModel);
    frozenTableView->setTableDelegate(basicTableDelegate);
    frozenTableView->initColumns(frozenColumns);
    frozenTableView->horizontalHeader()->setFocusPolicy(Qt::NoFocus);
    frozenTableView->verticalHeader()->hide();
    frozenTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    frozenTableView->raise();
    frozenTableView->setSelectionModel(selectionModel());
    frozenTableView->setSelectionBehavior(SelectionBehavior::SelectRows);
    for(int i = 0;i<columns.size();i++){
        if(i < frozenColumns.size()){
            frozenTableView->setColumnWidth(i, columnWidth(i));
        } else {
            frozenTableView->setColumnHidden(i, true);
        }
    }

    frozenTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    frozenTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    frozenTableView->show();

    updateFrozenTableViewGeometry();

    frozenTableView->setVerticalScrollMode(verticalScrollMode());

}

void BaseTableView::initAggreTableView()
{
    QList<ColumnConfig> tempColumns;
    for(int i = 0;i<columns.size();i++){
        ColumnConfig column = columns.at(i);
        if(!column.getColumnAggre()){
            column.setDataFormat("");
        }
        column.setColumnFrozen(false);
        column.setEditMode(EditMode::Null);
        column.setCheckBoxColumn(false);
        tempColumns.append(column);
    }
    aggreTableView->setModel(aggreTableModel);
    aggreTableView->init(tempColumns, aggreData);
    aggreTableView->setFocusPolicy(Qt::NoFocus);
    aggreTableView->verticalHeader()->hide();
    aggreTableView->horizontalHeader()->hide();
    QPalette palette = aggreTableView->palette();
    palette.setColor(QPalette::Base, palette.color(QPalette::Window));
    aggreTableView->setPalette(palette);

    aggreTableView->raise();
    aggreTableView->setSelectionMode(SelectionMode::NoSelection);
    for(int i = 0;i<columns.size();i++){
        if(i < frozenColumns.size()){
            aggreTableView->setColumnHidden(i, true);
        } else {
            aggreTableView->setColumnWidth(i, columnWidth(i));
        }
    }

    aggreTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    aggreTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    updateAggreTableViewGeometry();

    aggreTableView->setHorizontalScrollMode(horizontalScrollMode());
}

void BaseTableView::updateFrozenTableViewGeometry()
{
    int width = 0;
    for(int i = 0;i<frozenColumns.size();i++){
        width += columnWidth(i);
    }
    int aggreRowHeight = 0;
    if(aggreTableModel != 0){
        aggreRowHeight = aggreTableView->rowHeight(0);
    }
    frozenTableView->setGeometry(verticalHeader()->width() + frameWidth(),
                                 frameWidth(), width, viewport()->height()+horizontalHeader()->height()+aggreRowHeight);


}

void BaseTableView::updateAggreTableViewGeometry()
{
    int width = 0;
    for(int i = 0;i<frozenColumns.size();i++){
        width += columnWidth(i);
    }
    aggreTableView->setGeometry(verticalHeader()->width() + frameWidth() + width, viewport()->height()+frameWidth()+horizontalHeader()->height(), viewport()->width()-width-frameWidth(), aggreTableView->rowHeight(0));
}

void BaseTableView::aggreDataChanged(const QModelIndex &index)
{
    if(columns[index.column()].getColumnAggre()){
        double sum = 0;
        for(int i = 0;i<basicTableModel->getDataList()->size();i++){
            BaseEntity *entity = basicTableModel->getDataList()->at(i);
            sum += entity->get(columns[index.column()].getId()).toDouble();
        }
        aggreData->at(0)->set(columns[index.column()].getId(), sum);
    }
}

void BaseTableView::aggreDataChanged()
{
    QList<QString> aggreColumns;
    QMap<QString, double> sums;
    for(int i = 0;i<columns.size();i++){
        if(columns[i].getColumnAggre()){
            aggreColumns.append(columns[i].getId());
            sums.insert(columns[i].getId(), 0);
        }
    }
    for(int i = 0;i<basicTableModel->getDataList()->size();i++){
        BaseEntity *entity = basicTableModel->getDataList()->at(i);
        for(int j = 0;j<aggreColumns.size();j++){
            sums[aggreColumns[j]] += entity->get(aggreColumns[j]).toDouble();
        }
    }
    for(int i = 0;i<aggreColumns.size();i++){
        aggreData->at(0)->set(aggreColumns[i], sums[aggreColumns[i]]);
    }
}

void BaseTableView::setNeedAggreRow()
{
    this->needAggreRow = true;
}

void BaseTableView::setAggreData(QList<BaseEntity *> *aggreData)
{
    this->aggreData = aggreData;
}

void BaseTableView::setSortingEnabled(bool enable)
{
    QTableView::setSortingEnabled(enable);
    if(tableRole == TableRole::MainTable){
        if(frozenTableView != 0){
            frozenTableView->setSortingEnabled(enable);
        }
        for(int i = 0;i<columns.size();i++){
            if(columns[i].getSortable()){
                if(i < frozenColumns.size()){
                    frozenTableView->horizontalHeader()->setSortIndicator(i, Qt::AscendingOrder);
                    frozenTableView->sortByColumn(frozenTableView->horizontalHeader()->sortIndicatorSection(),
                                                  frozenTableView->horizontalHeader()->sortIndicatorOrder());
                } else {
                    horizontalHeader()->setSortIndicator(i, Qt::AscendingOrder);
                    sortByColumn(horizontalHeader()->sortIndicatorSection(),
                                                  horizontalHeader()->sortIndicatorOrder());
                }
                return;
            }
        }
    }
}

void BaseTableView::setTableRowHeight(int height)
{
    QHeaderView *vertical = verticalHeader();
    vertical->setDefaultSectionSize(height);
    if(frozenTableView != 0){
        frozenTableView->setTableRowHeight(height);
    }
}

QList<BaseEntity*> BaseTableView::getSelectedDatas()
{
    QModelIndexList list = selectionModel()->selectedRows();
    QList<BaseEntity*> entityList;
    foreach(QModelIndex index, list){
        entityList.append(basicTableModel->getDataList()->at(index.row()));
    }
    return entityList;
}

QList<BaseEntity *> BaseTableView::getCheckBoxDatas()
{
    QList<BaseEntity*> entityList;
    if(!hasCheckBoxColumn){
        return entityList;
    }
    QString checkBoxTag = "checkBox";
    if(!checkBoxRelativeToSelection){
        checkBoxTag = "checkBox2";
    }
    for(int i = 0;i<getDataList()->size();i++){
        if(getDataList()->at(i)->get(checkBoxTag).toInt() == Qt::Checked){
            entityList.append(getDataList()->at(i));
        }
    }
    return entityList;
}

BasicTableModel *BaseTableView::getModel()
{
    return basicTableModel;
}

QList<BaseEntity*>* BaseTableView::getDataList()
{
    return basicTableModel->getDataList();
}
