#ifndef SDP2DPARASSETTINGDELEGATE_H
#define SDP2DPARASSETTINGDELEGATE_H

#include <QStyledItemDelegate>

class Sdp2dParasSettingDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit Sdp2dParasSettingDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
};

#endif // SDP2DPARASSETTINGDELEGATE_H
