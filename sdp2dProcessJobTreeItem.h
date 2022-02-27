#ifndef SDP2DPARASETITEM_H
#define SDP2DPARASETITEM_H

#include <QVariant>
#include <QVector>
//#include <QStandardItem>

QT_BEGIN_NAMESPACE
class QDomElement;
QT_END_NAMESPACE


class Sdp2dParasSettingItem
{
public:
    Sdp2dParasSettingItem(const QVector<QVariant> &data, Sdp2dParasSettingItem *parent = nullptr);
    ~Sdp2dParasSettingItem();

    Sdp2dParasSettingItem *child(int number);
    Sdp2dParasSettingItem *parent();

    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    bool insertChild(int position, Sdp2dParasSettingItem* item);
    bool insertChildren(int position, int count, int columns);
    bool insertColumns(int position, int columns);

    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(int column, const QVariant &value);    

private:

    QVector<Sdp2dParasSettingItem *> m_childItems;
    QVector<QVariant> m_itemData;
    Sdp2dParasSettingItem *m_parentItem;

};

#endif // SDP2DPARASETITEM_H
