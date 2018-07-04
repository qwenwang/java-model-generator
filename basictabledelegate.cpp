#include "basictabledelegate.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QTimeEdit>
#include <QValidator>
#include <QRegExp>

BasicTableDelegate::BasicTableDelegate(QWidget *parent)
    :QStyledItemDelegate(parent)
{

}

void BasicTableDelegate::setColumns(QList<ColumnConfig> &columns)
{
    this->columns = columns;
}

QWidget* BasicTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    if(!index.isValid()){
        return 0;
    }
    switch((columns[index.column()]).getEditMode()){
    case EditMode::Null:
    {
        return 0;
    }
    case EditMode::CheckBox:
    {
        return new QCheckBox(parent);
    }
    case EditMode::DateEdit:
    {
        QDateEdit *dateEdit = new QDateEdit(parent);
        dateEdit->setCalendarPopup(true);
        return dateEdit;
    }
    case EditMode::TimeEdit:
    {
        return new QTimeEdit(parent);
    }
    case EditMode::StringEdit:
    {
        QLineEdit *lineEdit = new QLineEdit(parent);
        if(columns[index.column()].getAlignment() == HorizontalAlignment::AlignRight){
            lineEdit->setAlignment(Qt::AlignRight);
        }
        return lineEdit;
    }
    case EditMode::IntegerEdit:
    {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setInputMethodHints(Qt::ImhDigitsOnly);
        if(columns[index.column()].getAlignment() == HorizontalAlignment::AlignRight){
            lineEdit->setAlignment(Qt::AlignRight);
        }
        lineEdit->setValidator(new QIntValidator());
        return lineEdit;
    }
    case EditMode::FloatEdit:
    {
        QLineEdit *lineEdit = new QLineEdit(parent);
        lineEdit->setInputMethodHints(Qt::ImhDigitsOnly);
        if(columns[index.column()].getAlignment() == HorizontalAlignment::AlignRight){
            lineEdit->setAlignment(Qt::AlignRight);
        }
        lineEdit->setValidator((new QDoubleValidator()));
        return lineEdit;
    }
    case EditMode::CustomEdit:
        if(columns[index.column()].getCustomEdit() != 0){
            return columns[index.column()].getCustomEdit()->getNewInstance(parent);
        }
    }
    return 0;
}

void BasicTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if(editor == 0){
        return;
    }
    if(!index.isValid()){
        return;
    }
    switch(columns[index.column()].getEditMode()){
    case EditMode::Null:
    {
        return;
    }
    case EditMode::CheckBox:
    {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(editor);
        if(index.data(Qt::EditRole).toBool() == true){
            checkBox->setCheckState(Qt::Checked);
        } else {
            checkBox->setCheckState(Qt::Unchecked);
        }
        break;
    }
    case EditMode::DateEdit:
    {
        QDateEdit *dateEdit = qobject_cast<QDateEdit*>(editor);
        dateEdit->setDate(index.data(Qt::EditRole).toDate());
        break;
    }
    case EditMode::TimeEdit:
    {
        QTimeEdit *timeEdit = qobject_cast<QTimeEdit*>(editor);
        timeEdit->setTime(index.data(Qt::EditRole).toTime());
        break;
    }
    case EditMode::StringEdit:
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
        lineEdit->setText(index.data(Qt::EditRole).toString());
        break;
    }
    case EditMode::IntegerEdit:
    case EditMode::FloatEdit:
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
        QString dataFormat = const_cast<BasicTableDelegate*>(this)->columns[index.column()].getDataFormat();
        QRegExp regExp("[#*,?#*]*,?[0*,?0*]*\\.*[0]*");
        if(dataFormat != "" && regExp.exactMatch(dataFormat)){
            int i = dataFormat.indexOf(".");
            int presition = 0;
            int length = dataFormat.count("0");
            if(i != -1){
                presition = dataFormat.length()-1-i;
                length += 1;
            }
            lineEdit->setText(QString("%1").arg(index.data(Qt::EditRole).toFloat(), length, 'f', presition, QLatin1Char('0')));
        } else {
            lineEdit->setText(index.data(Qt::EditRole).toString());
        }
        break;
    }
    case EditMode::CustomEdit:
    {
        BasicEdit *basicEdit = columns[index.column()].getCustomEdit()->getBasicEdit(editor);
        basicEdit->setData(index.data(Qt::EditRole));
        break;
    }
    }
}

void BasicTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if(editor == 0){
        return;
    }
    if(!index.isValid()){
        return;
    }
    switch(columns[index.column()].getEditMode()){
    case EditMode::Null:
    {
        return;
    }
    case EditMode::CheckBox:
    {
        QCheckBox *checkBox = qobject_cast<QCheckBox*>(editor);
        model->setData(index, checkBox->checkState()==Qt::Checked?true:false);
        break;
    }
    case EditMode::DateEdit:
    {
        QDateEdit *dateEdit = qobject_cast<QDateEdit*>(editor);
        model->setData(index, dateEdit->date());
        break;
    }
    case EditMode::TimeEdit:
    {
        QTimeEdit *timeEdit = qobject_cast<QTimeEdit*>(editor);
        model->setData(index, timeEdit->time());
        break;
    }
    case EditMode::StringEdit:
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
        model->setData(index, lineEdit->text());
        break;
    }
    case EditMode::IntegerEdit:
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
        model->setData(index, lineEdit->text().toInt());
        break;
    }
    case EditMode::FloatEdit:
    {
        QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor);
        model->setData(index, lineEdit->text().toDouble());
        break;
    }
    case EditMode::CustomEdit:
    {
        BasicEdit *basicEdit = columns[index.column()].getCustomEdit()->getBasicEdit(editor);
        model->setData(index, basicEdit->getData());
        break;
    }
    }
}
