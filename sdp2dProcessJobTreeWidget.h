#ifndef SDP2DPROCESSJOBTREEWIDGET_H
#define SDP2DPROCESSJOBTREEWIDGET_H

#include <QTreeWidget>


QT_BEGIN_NAMESPACE
class QTreeWidget;
class QTreeWidgetItem;
class QDomElement;
QT_END_NAMESPACE

class Sdp2dProcessJobDelegate;
class Sdp2dQDomDocument;

class Sdp2dProcessJobTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit Sdp2dProcessJobTreeWidget(Sdp2dQDomDocument* domdoc, Sdp2dQDomDocument* domval, QWidget *parent = nullptr);
    ~Sdp2dProcessJobTreeWidget();

    void processModuleChanged(void);
    void setParameterValuesToDom(void);

private:
    void fillParametersWithDom(QTreeWidgetItem* parentItem, QDomElement& parentNode);
    void fillDomWithParameters(QTreeWidgetItem* parentItem, QDomElement& parentNode, QDomElement& valNode);
    QDomElement findDomElementOfParameter(QDomElement& node, QString& name);    
    QDomElement findDomElementOfGroup(QDomElement& node, QString& name);
    QDomElement findDomElementOfOption(QDomElement& node, QString& name, QString& value, QDomElement& child);

    QTreeWidgetItem* createTreeItem(QTreeWidgetItem *parentItem, const QDomElement &parent, const QDomElement &child);

    void parameterOptionChanged(QTreeWidgetItem *item, int column);

    void findDomElementForParaOption(QString& parentName, QString& newValue, QDomElement &parent, QDomElement& result);


private:
    Sdp2dQDomDocument* m_domdoc;
    Sdp2dQDomDocument* m_domval;
    Sdp2dProcessJobDelegate* m_paraDelegate;

    bool m_paraTreeInited;
};

#endif // SDP2DPROCESSJOBTREEWIDGET_H
