#ifndef SDP2DPARASETMODEL_H
#define SDP2DPARASETMODEL_H

#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QDomDocument>
#include <QModelIndex>
#include <QVariant>

class Sdp2dParasSettingItem;

class Sdp2dParasSettingModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit Sdp2dParasSettingModel(QDomDocument* document, QObject *parent = nullptr);
    ~Sdp2dParasSettingModel();

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant &value, int role = Qt::EditRole) override;

    bool insertColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int position, int columns,
                       const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;

    int totalRowNumber(Sdp2dParasSettingItem *parentItem) const;

    Sdp2dParasSettingItem* getRootItem(void) const { return m_rootItem; }

    int fillDomWithParameters(QDomElement& parentNode, Sdp2dParasSettingItem *parentItem) ;
    void fillParametersWithDom(QDomElement& parentNode, Sdp2dParasSettingItem *parentItem) ;
    bool checkParametersValidate(Sdp2dParasSettingItem *parentItem);

    Sdp2dParasSettingItem *getItem(const QModelIndex &index) const;

private:    
    Sdp2dParasSettingItem* createItem(const QDomElement &element, Sdp2dParasSettingItem *parentItem);


private:
    QDomDocument* m_domDocument;
    Sdp2dParasSettingItem *m_rootItem;
};

#endif // SDP2DPARASETMODEL_H
