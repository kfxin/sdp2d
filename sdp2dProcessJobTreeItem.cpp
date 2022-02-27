#include "sdp2dParasSettingItem.h"

#include <QtXml>

Sdp2dParasSettingItem::Sdp2dParasSettingItem(const QVector<QVariant> &data, Sdp2dParasSettingItem *parent)
    : m_itemData(data), m_parentItem(parent)
{

}

Sdp2dParasSettingItem::~Sdp2dParasSettingItem()
{
    qDeleteAll(m_childItems);
}


Sdp2dParasSettingItem *Sdp2dParasSettingItem::parent()
{
    return m_parentItem;
}

Sdp2dParasSettingItem *Sdp2dParasSettingItem::child(int number)
{
    if (number < 0 || number >= m_childItems.size())
        return nullptr;
    return m_childItems.at(number);
}


int Sdp2dParasSettingItem::childCount() const
{
    return m_childItems.count();
}

int Sdp2dParasSettingItem::childNumber() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<Sdp2dParasSettingItem*>(this));
    return 0;
}

int Sdp2dParasSettingItem::columnCount() const
{
    return m_itemData.count();
}

QVariant Sdp2dParasSettingItem::data(int column) const
{
    if (column < 0 || column >= m_itemData.size())
        return QVariant();
    return m_itemData.at(column);
}

bool Sdp2dParasSettingItem::insertChildren(int position, int count, int columns)
{
    if (position < 0 || position > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row) {
        QVector<QVariant> data(columns);
        Sdp2dParasSettingItem *item = new Sdp2dParasSettingItem(data, this);
        m_childItems.insert(position, item);
    }

    return true;
}

bool Sdp2dParasSettingItem::insertChild(int position, Sdp2dParasSettingItem* item)
{
    if (position < 0 || position > m_childItems.size())
        return false;

    m_childItems.insert(position, item);
    return true;
}

bool Sdp2dParasSettingItem::insertColumns(int position, int columns)
{
    if (position < 0 || position > m_itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        m_itemData.insert(position, QVariant());

    for (Sdp2dParasSettingItem *child : qAsConst(m_childItems))
        child->insertColumns(position, columns);

    return true;
}


bool Sdp2dParasSettingItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete m_childItems.takeAt(position);

    return true;
}

bool Sdp2dParasSettingItem::removeColumns(int position, int columns)
{
    if (position < 0 || position + columns > m_itemData.size())
        return false;

    for (int column = 0; column < columns; ++column)
        m_itemData.remove(position);

    for (Sdp2dParasSettingItem *child : qAsConst(m_childItems))
        child->removeColumns(position, columns);

    return true;
}

bool Sdp2dParasSettingItem::setData(int column, const QVariant &value)
{
    if (column < 0 || column >= m_itemData.size())
        return false;

    m_itemData[column] = value;

    return true;
}

