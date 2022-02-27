#ifndef SDP2DPROCESSJOBDELEGATE_H
#define SDP2DPROCESSJOBDELEGATE_H

#include <QStyledItemDelegate>

class Sdp2dProcessJobDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit Sdp2dProcessJobDelegate(QObject *parent = nullptr);

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
