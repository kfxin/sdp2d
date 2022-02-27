#include "sdp2dParasSettingModel.h"
#include "sdp2dParasSettingItem.h"
#include <QMessageBox>
#include <QtXml>


#include <iostream>
#include <fstream>
#include <string>

using namespace std;

enum { DomElementRole = Qt::UserRole + 1 };

Q_DECLARE_METATYPE(QDomElement)

Sdp2dParasSettingModel::Sdp2dParasSettingModel(QDomDocument* document, QObject *parent)
    : QAbstractItemModel(parent),
      m_domDocument(document)
{
    const QStringList headers({tr("Name"), tr("Type"), tr("Range"), tr("Default Value"), tr("Repeatable"), tr("Required"), tr("Description")});
    QVector<QVariant> rootData;
    for (const QString &header : headers)
        rootData << header;

    m_rootItem = new Sdp2dParasSettingItem(rootData);
}


Sdp2dParasSettingModel::~Sdp2dParasSettingModel()
{
    delete m_rootItem;
}

int Sdp2dParasSettingModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 7;
}

int Sdp2dParasSettingModel::rowCount(const QModelIndex &parent) const
{
    const Sdp2dParasSettingItem *parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

int Sdp2dParasSettingModel::totalRowNumber(Sdp2dParasSettingItem *parentItem) const
{
    int count = 0;
    if(parentItem->childCount() > 0) {
        for(int i=0; i< parentItem->childCount(); i++){
            Sdp2dParasSettingItem* child = parentItem->child(i);
            count += totalRowNumber(child);
            count++;
        }
    }
    return count;
}


bool Sdp2dParasSettingModel::checkParametersValidate(Sdp2dParasSettingItem *parentItem)
{
    for(int i=0; i< parentItem->childCount(); i++){

        Sdp2dParasSettingItem* child = parentItem->child(i);

        QString type = child->data(1).toString();

        if(type.compare("ParamsGroup") == 0 || type.compare("ParaOptions") == 0){
            if(child->childCount() == 0){
                QMessageBox message(QMessageBox::NoIcon,
                     "Warn", QString("Parameter Group %1 must have at least ONE child!").arg(child->data(0).toString()), QMessageBox::Ok, NULL);
                message.exec();
                return false;
            }
        }
        if(type.compare("ParaOptions") == 0){
            for(int j=0; j < child->childCount(); j++){
                Sdp2dParasSettingItem* c = child->child(j);
                if(c->data(1).toString().compare("ParamsGroup")){
                    QMessageBox message(QMessageBox::NoIcon,
                         "Warn", QString("Children %1 of ParaOptions %2 must be Parameter Group!").arg(c->data(0).toString(), child->data(0).toString()), QMessageBox::Ok, NULL);
                    message.exec();
                    return false;
                }
            }
        }
        QStringList range = child->data(2).toString().split(",");
        if(type.compare("StringList") == 0 && range.count()<2){
            QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("StringList %1 must provide at least two strings in RANGE!").arg(child->data(0).toString()), QMessageBox::Ok, NULL);
            message.exec();
            return false;
        }

        if(!checkParametersValidate(child)) return false;
    }
    return true;
}

int Sdp2dParasSettingModel::fillDomWithParameters(QDomElement& parentNode, Sdp2dParasSettingItem *parentItem)
{

    int count = 0;
    if(parentItem->childCount() > 0) {
        for(int i=0; i< parentItem->childCount(); i++){
            Sdp2dParasSettingItem* child = parentItem->child(i);

            QString name = child->data(0).toString();
            QString type = child->data(1).toString();
            QString rang = child->data(2).toString();
            QString defa = child->data(3).toString();
            QString repe = child->data(4).toString();
            QString requ = child->data(5).toString();
            QString help = child->data(6).toString();

            QDomElement tag;
            if(type.compare("ParamsGroup") == 0){
                tag = m_domDocument->createElement("ParamsGroup");
                tag.setAttribute(QString("Type"), type);                
                if(repe.count()>0) tag.setAttribute(QString("Repeatable"), repe);
                if(requ.count()>0) tag.setAttribute(QString("Required"), requ);
                if(help.count()>0) tag.setAttribute(QString("Description"), help);
            } else if(type.compare("ParaOptions") == 0) {
                tag = m_domDocument->createElement("ParaOptions");
                tag.setAttribute(QString("Type"), type);
                if(child->childCount()>0){
                    QStringList pgns;
                    for(int j=0; j< child->childCount(); j++){
                        Sdp2dParasSettingItem* c = child->child(j);
                        pgns.append(c->data(0).toString());
                    }
                    if(pgns.count()>0) defa=pgns.at(0);
                    rang = pgns.join(", ");
                }
                if(rang.count()>0) tag.setAttribute(QString("Range"), rang);
                if(defa.count()>0) tag.setAttribute(QString("DefaultValue"), defa);
                if(requ.count()>0) tag.setAttribute(QString("Required"), requ);
                if(help.count()>0) tag.setAttribute(QString("Description"), help);
            } else {
                tag = m_domDocument->createElement("Parameters");
                QDomElement newPara = m_domDocument->createElement(name);
                if(type.count()>0) newPara.setAttribute(QString("Type"), type);
                if(rang.count()>0) newPara.setAttribute(QString("Range"), rang);

                if(type.compare("StringList") == 0) {
                    newPara.setAttribute(QString("DefaultValue"), rang.split(",")[0].trimmed());
                } else if(defa.count()>0) {
                    newPara.setAttribute(QString("DefaultValue"), defa);
                }

                if(requ.count()>0) newPara.setAttribute(QString("Required"), requ);
                if(help.count()>0) newPara.setAttribute(QString("Description"), help);
                tag.appendChild(newPara);
            }
            tag.setAttribute(QString("Name"), name);

            if(type.compare("ParamsGroup") == 0 || type.compare("ParaOptions") == 0){
                count += fillDomWithParameters(tag, child);
            }

            parentNode.appendChild(tag);
            count++;
        }
    }
    return count;
}

void Sdp2dParasSettingModel::fillParametersWithDom(QDomElement& parentNode, Sdp2dParasSettingItem *parentItem)
{
    QDomNode n = parentNode.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.

        if(e.tagName().compare("Parameters") == 0){
            QString name = e.attribute("Name");
            QDomElement child = e.firstChildElement(name);            
            createItem(child, parentItem);
        } else if(e.tagName().compare("ParamsGroup") == 0){
            Sdp2dParasSettingItem* p = createItem(e, parentItem);
            fillParametersWithDom(e, p);
        }else if(e.tagName().compare("ParaOptions") == 0){
            Sdp2dParasSettingItem* p = createItem(e, parentItem);
            fillParametersWithDom(e, p);
        }

        n = n.nextSibling();
    }
}


QModelIndex Sdp2dParasSettingModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Sdp2dParasSettingItem *childItem = getItem(index);
    Sdp2dParasSettingItem *parentItem = childItem ? childItem->parent() : nullptr;

    if (parentItem == m_rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

QVariant Sdp2dParasSettingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    Sdp2dParasSettingItem *item = getItem(index);

    return item->data(index.column());
}

Qt::ItemFlags Sdp2dParasSettingModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

Sdp2dParasSettingItem *Sdp2dParasSettingModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        Sdp2dParasSettingItem *item = static_cast<Sdp2dParasSettingItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_rootItem;
}

QVariant Sdp2dParasSettingModel::headerData(int section, Qt::Orientation orientation,
                              int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootItem->data(section);

    return QVariant();
}

QModelIndex Sdp2dParasSettingModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    Sdp2dParasSettingItem *parentItem = getItem(parent);
    if (!parentItem)
        return QModelIndex();

    Sdp2dParasSettingItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

bool Sdp2dParasSettingModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    beginInsertColumns(parent, position, position + columns - 1);
    const bool success = m_rootItem->insertColumns(position, columns);
    endInsertColumns();

    return success;
}

bool Sdp2dParasSettingModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    Sdp2dParasSettingItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChildren(position,
                                                    rows,
                                                    m_rootItem->columnCount());
    endInsertRows();    

    return success;
}

bool Sdp2dParasSettingModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    beginRemoveColumns(parent, position, position + columns - 1);
    const bool success = m_rootItem->removeColumns(position, columns);
    endRemoveColumns();

    if (m_rootItem->columnCount() == 0)
        removeRows(0, rowCount());

    return success;
}


bool Sdp2dParasSettingModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Sdp2dParasSettingItem *parentItem = getItem(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}



bool Sdp2dParasSettingModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    Sdp2dParasSettingItem *item = getItem(index);
    bool result = item->setData(index.column(), value);

    if(index.column() == 1){
        if(value.toString().compare("ParamsGroup") == 0) {
            item->setData(4, QVariant("No"));
        }else {
            item->setData(4, QVariant(""));
        }
    }

    if (result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return result;
}

bool Sdp2dParasSettingModel::setHeaderData(int section, Qt::Orientation orientation,
                              const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = m_rootItem->setData(section, value);

    if (result)
        emit headerDataChanged(orientation, section, section);

    return result;
}


Sdp2dParasSettingItem* Sdp2dParasSettingModel::createItem(const QDomElement &element, Sdp2dParasSettingItem *parentItem)
{
    int nchild = parentItem->childCount();

    QModelIndex idx = createIndex(nchild, 0, parentItem);
    insertRows(nchild, 1, idx);

    //cout << "nchild=" << nchild << " nchnew=" << parentItem->childCount() << " tag=" << element.tagName().toStdString().c_str() << " row=" << idx.row()  << " col=" << idx.column() << endl;

    Sdp2dParasSettingItem *item = parentItem->child(nchild);

    if(element.tagName().compare(QString("ParamsGroup")) == 0){
        item->setData(0, QVariant(element.attribute("Name")));
        item->setData(4, QVariant(element.attribute("Repeatable")));
    } else if(element.tagName().compare("ParaOptions") == 0){
        item->setData(0, QVariant(element.attribute("Name")));
        item->setData(4, QVariant(""));
    } else {
        item->setData(0, QVariant(element.tagName()));
        item->setData(4, QVariant(""));
    }
    item->setData(1, QVariant(element.attribute("Type")));
    item->setData(2, QVariant(element.attribute("Range")));
    item->setData(3, QVariant(element.attribute("DefaultValue")));
    item->setData(5, QVariant(element.attribute("Required")));
    item->setData(6, QVariant(element.attribute("Description")));

    return item;
}
