#ifndef BASETABLE_H
#define BASETABLE_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include <QMap>
#include <QString>
#include <QVariant>
#include <QWidget>

enum HorizontalAlignment {
    AlignLeft, AlignCenter, AlignRight
};

enum EditMode {
    Null, CheckBox, DateEdit, TimeEdit, StringEdit, IntegerEdit, FloatEdit, CustomEdit
};

class BasicEdit {
public:
    BasicEdit(){}
    virtual void setData(QVariant data) = 0;
    virtual QVariant getData() = 0;
    virtual QWidget* getNewInstance(QWidget *parent) = 0;
    virtual BasicEdit* getBasicEdit(QWidget *widget) = 0;
};

class BaseEntity
{
public:
    BaseEntity(){}
    void set(const QString &key, const QVariant &value){
        attributes.insert(key, value);
    }
    QVariant get(const QString &key) const {return attributes.value(key);}
    bool isValid() const
    {
        if(attributes.size() == 0) return false;
        QStringList keys = attributes.keys();
        foreach(const QString &key, keys){
            if(!key.startsWith("checkBox")) return true;
        }
        return false;
    }
    void setCheckState(bool state){state?set("checkBox2", Qt::Checked):set("checkBox2", Qt::Unchecked);}
    bool getCheckState() const {return get("checkBox2").toInt()==Qt::Checked?true:false;}

protected:
    QHash<QString, QVariant> attributes;

    friend class AppUtil;
};

class ColumnConfig {
public:
    ColumnConfig() : alignment(HorizontalAlignment::AlignLeft), editMode(EditMode::Null)
    {this->width = 20; hidden = false; resizable = true; sortable = true; columnFrozen = false; columnAggre = false; checkBoxColumn = false;
     customEdit = 0; relativeToSelection = true;}
    ColumnConfig(QString id, QString name, int width) : alignment(HorizontalAlignment::AlignLeft), editMode(EditMode::Null)
    {this->id = id; this->name = name; this->width = width; hidden = false; resizable = true; sortable = true; columnFrozen = false;
     columnAggre = false; checkBoxColumn = false; customEdit = 0; relativeToSelection = true;}
    void setId(QString id){this->id = id;}
    void setName(QString name){this->name = name;}
    void setWidth(int width){this->width = width;}
    void setAlignment(HorizontalAlignment align){this->alignment = align;}
    void setDataFormat(QString format){this->format = format;} //NumberFormat #,##0.00, DateTimeFormat yyyy-MM-dd hh:mm:ss
    void setHidden(bool hidden){this->hidden = hidden;}
    void setResizable(bool resizable){this->resizable = resizable;}
    void setSortable(bool sortable){this->sortable = sortable;}
    void setEditMode(EditMode editMode){this->editMode = editMode;}
    void setCustomEdit(BasicEdit *customEdit){this->customEdit = customEdit;}
    void setColumnFrozen(bool state){this->columnFrozen = state;}
    void setColumnAggre(bool state){this->columnAggre = state;}
    void setColumnAggreName(QString name){this->aggreName = name;}
    void setCheckBoxColumn(bool checkBoxColumn, bool relativeToSelection = true){this->checkBoxColumn = checkBoxColumn; this->relativeToSelection = relativeToSelection;}
    void setColumnText(QString columnText){this->columnText = columnText;}
    QString getId() const {return id;}
    QString getName() const {return name;}
    int getWidth() const {return width;}
    HorizontalAlignment getAlignment() const {return alignment;}
    QString getDataFormat() const {return format;}
    bool getHidden() const {return hidden;}
    bool getResizable() const {return resizable;}
    bool getSortable() const {return sortable;}
    EditMode getEditMode() const {return editMode;}
    BasicEdit *getCustomEdit() const {return customEdit;}
    bool getColumnFrozen() const {return columnFrozen;}
    bool getColumnAggre() const {return columnAggre;}
    QString getColumnAggreName() const {return aggreName;}
    bool getCheckBoxColumn() const {return checkBoxColumn;}
    bool getRelativeToSelection() const {return relativeToSelection;}
    QString getColumnText() const {return columnText;}

private:
    QString id;
    QString name;
    int width;
    HorizontalAlignment alignment;
    QString format;
    bool hidden;
    bool resizable;
    bool sortable;
    EditMode editMode;
    BasicEdit *customEdit;
    bool columnFrozen;
    bool columnAggre;
    QString aggreName;
    bool checkBoxColumn;
    bool relativeToSelection;
    QString columnText;
};

#endif // BASETABLE_H

