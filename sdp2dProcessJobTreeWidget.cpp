#include "sdp2dProcessJobTreeWidget.h"
#include "sdp2dProcessJobDelegate.h"
#include "sdp2dQDomDocument.h"

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDomElement>
#include <QHeaderView>
#include <QCheckBox>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QStringList>

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Sdp2dProcessJobTreeWidget::Sdp2dProcessJobTreeWidget(Sdp2dQDomDocument* domdoc, Sdp2dQDomDocument* domval, QWidget *parent)
    : QTreeWidget(parent)
{
    m_domdoc = domdoc;
    m_domval = domval;

    setHeaderLabels({tr("Parameter"), tr("Type"), tr("Required"), tr("Value")});
    header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(3, QHeaderView::Stretch);
    setAlternatingRowColors(true);

    m_paraTreeInited = false;
    m_paraDelegate= new Sdp2dProcessJobDelegate(this);
    setItemDelegate(m_paraDelegate);

    connect(this, &QTreeWidget::itemChanged, this, &Sdp2dProcessJobTreeWidget::parameterOptionChanged);
}

Sdp2dProcessJobTreeWidget::~Sdp2dProcessJobTreeWidget(void)
{

}

void Sdp2dProcessJobTreeWidget::processModuleChanged()
{
    //cout << "processModuleChanged" << endl;
    clear();
    QDomElement root = m_domdoc->documentElement();
    QStringList topLevel;
    topLevel << root.tagName() +QString(": ") + root.attribute("Title");
    QTreeWidgetItem *item = new QTreeWidgetItem(topLevel);
    addTopLevelItem(item);
    item->setFirstColumnSpanned(true);

    item->setToolTip(0, root.attribute("Description"));

    m_paraTreeInited = false;
    fillParametersWithDom(item, root);
    expandAll();
    m_paraTreeInited = true;
}


void Sdp2dProcessJobTreeWidget::setParameterValuesToDom(void)
{
    if(topLevelItemCount() == 0 ) return;
    //cout << "setParameterValuesToDom" << endl;
    QTreeWidgetItem* topItem = topLevelItem(0);
    QDomElement root = m_domdoc->documentElement();    

    m_domval->clear();
    QDomElement valRoot = m_domval->createElement(root.tagName());
    m_domval->appendChild(valRoot);

    fillDomWithParameters(topItem, root, valRoot);

    const int IndentSize = 4;
    QFile file("currentjob.xml");
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream out(&file);
    m_domval->save(out, IndentSize);
    file.close();
}

void Sdp2dProcessJobTreeWidget::fillDomWithParameters(QTreeWidgetItem* parentItem, QDomElement& parentNode, QDomElement& valNode)
{
    int nchild = parentItem->childCount();
    //cout << "fillDomWithParameters" << endl;
    for(int idx=0; idx < nchild; idx++){
        QTreeWidgetItem* item = parentItem->child(idx);
        QString name = item->data(0, Qt::EditRole).toString();
        QString type = item->data(1, Qt::EditRole).toString();
        QString value = item->data(3, Qt::EditRole).toString();

        //cout << "Looking for " << name.toStdString().c_str() << " type=" << type.toStdString().c_str() << " with value " << value.toStdString().c_str() << endl;
        QCheckBox* cbox = dynamic_cast<QCheckBox*>(itemWidget(item, 2));
        bool checked = cbox->isChecked();

        if(type.compare("Group") == 0) {
            if(checked == false && item->isExpanded() == false) continue;
        } else{
            if(value.count()<1) {
                if(checked){
                    QMessageBox message(QMessageBox::NoIcon,
                            "Warn", QString("Parameter %1 is required!").arg(name), QMessageBox::Ok, NULL);
                    message.exec();
                    return;
                }
                continue;
            }
        }

        if(type.compare("Group") == 0){
            QDomElement e = findDomElementOfGroup(parentNode, name);
            //cout << "Group name = " << e.attribute(QString("Name")).toStdString().c_str() << endl;
            if(e.isNull()) continue;

            QDomElement parTag = m_domval->createElement("ParamsGroup");
            parTag.setAttribute("Name", name);
            valNode.appendChild(parTag);
            fillDomWithParameters(item, e, parTag);

        } else if(type.compare("Option") == 0){
            QDomElement child;
            QDomElement e = findDomElementOfOption(parentNode, name, value, child);
            //cout << "Option name = " << e.attribute(QString("Name")).toStdString().c_str() << endl;
            if(e.isNull() || child.isNull()) continue;

            QDomElement parTag = m_domval->createElement("ParaOptions");
            parTag.setAttribute("Name", name);
            parTag.setAttribute("Value", value);
            valNode.appendChild(parTag);

            fillDomWithParameters(item, child, parTag);

        } else {
            QDomElement e = findDomElementOfParameter(parentNode, name);
            //cout << "Para name = " << e.attribute(QString("Name")).toStdString().c_str() << endl;
            if(e.isNull()) continue;

            QDomElement parTag = m_domval->createElement("Parameters");
            parTag.setAttribute(QString("Name"), name);
            parTag.setAttribute(QString("Value"), value);
            valNode.appendChild(parTag);
        }
    }
}


QDomElement Sdp2dProcessJobTreeWidget::findDomElementOfParameter(QDomElement& parent, QString& name)
{
    QDomNode n = parent.firstChild();

    QDomElement result;
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(e.tagName().compare("Parameters") == 0){
            QString domname = e.attribute("Name");
            if(domname.compare(name) == 0){
                result = e.firstChildElement(name);
                break;
            }
        }
        n = n.nextSibling();
    }
    return result;
}


QDomElement Sdp2dProcessJobTreeWidget::findDomElementOfGroup(QDomElement& parent, QString& name)
{
    QDomNode n = parent.firstChild();

    QDomElement result;
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(e.tagName().compare("ParamsGroup") == 0){
            QString domname = e.attribute("Name");
            if(domname.compare(name) == 0) {
                result = e;
                break;
            }
        }
        n = n.nextSibling();
    }
    return result;
}


QDomElement Sdp2dProcessJobTreeWidget::findDomElementOfOption(QDomElement& parent, QString& name, QString& value, QDomElement& child)
{
    QDomNode n = parent.firstChild();

    QDomElement result;
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.

        if(e.tagName().compare("ParaOptions") == 0){
            QString domname = e.attribute("Name");
            if(domname.compare(name) == 0) {
                QDomNode nn = e.firstChild();
                while(!nn.isNull()) {
                    QDomElement ee = nn.toElement();
                    if(ee.tagName().compare("ParamsGroup") == 0){
                        QString grpname = ee.attribute("Name");
                        if(grpname.compare(value) ==0) {
                            child = ee;
                            return e;
                        }
                    }
                    nn = nn.nextSibling();
                }
            }
        }
        n = n.nextSibling();
    }
    return result;
}



void Sdp2dProcessJobTreeWidget::fillParametersWithDom(QTreeWidgetItem* parentItem, QDomElement& parentNode)
{
    //cout << "in fillParametersWithDom" << endl;
    QDomNode n = parentNode.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.

        if(e.tagName().compare("Parameters") == 0){
            QString name = e.attribute("Name");
            QDomElement child = e.firstChildElement(name);
            createTreeItem(parentItem, e, child);
        } else if(e.tagName().compare("ParamsGroup") == 0){
            QTreeWidgetItem* p = createTreeItem(parentItem, e, e);
            if(p != nullptr) fillParametersWithDom(p, e);
        }else if(e.tagName().compare("ParaOptions") == 0){
            QTreeWidgetItem* p = createTreeItem(parentItem, e, e);
            //QDomElement eFirstChild = e.firstChild().toElement();

            QString optName = e.attribute("Name");
            QString optValue = e.attribute("Value");
            if(optValue.count() < 1) optValue = e.attribute("DefaultValue");
            QDomElement eChild;
            //cout << "optName=" << optName.toStdString().c_str() << " optValue=" << optValue.toStdString().c_str() << endl;
            findDomElementForParaOption(optName, optValue, e, eChild);

            if(p != nullptr) fillParametersWithDom(p, eChild);
        }

        n = n.nextSibling();
    }
}


QTreeWidgetItem* Sdp2dProcessJobTreeWidget::createTreeItem(QTreeWidgetItem *parentItem, const QDomElement &parent, const QDomElement &child)
{

    QString name;

    int paraType = 0;

    if(child.tagName().compare(QString("ParamsGroup")) == 0){
        paraType = 1;
    } else if(child.tagName().compare("ParaOptions") == 0){
        paraType = 2;
    }


    if(paraType == 0){
        name = child.tagName();
    }else{
        name = child.attribute("Name");
    }

    int nchild = parentItem->childCount();
    for(int i=0; i<nchild; i++){
        if(parentItem->child(i)->data(0, Qt::EditRole).toString().compare(name) == 0) return nullptr;
    }

    QString type;

    if(paraType == 0){

        if(child.attribute("Type").compare(QString("String")) == 0){
            type = "String";
        } else if(child.attribute("Type").compare("Int") == 0){
            type = "Int";
        } else if(child.attribute("Type").compare("Float") == 0){
            type = "Float";
        } else if(child.attribute("Type").compare("Bool") == 0){
            type = "Bool";
        } else if(child.attribute("Type").compare("StringList") == 0){
            type = "List";
        }
    } else if(paraType == 1){
        type = "Group";
    } else {
        type = "Option";
    }


    QStringList itemValues;

    itemValues << name << type;
    //cout << "name=" << name.toStdString().c_str() << " type="<< type.toStdString().c_str() << " value=" << element.attribute("Value").toStdString().c_str() << endl;
    QTreeWidgetItem *item = new QTreeWidgetItem(parentItem, itemValues);

    QCheckBox* cbox = new QCheckBox();
    if(child.attribute("Required").count() > 0){
        if(child.attribute("Required").compare("Yes") == 0) cbox->setCheckState(Qt::Checked);
        else cbox->setCheckState(Qt::Unchecked);
    }
    cbox->setEnabled(false);
    setItemWidget(item, 2, cbox);

    if(child.attribute("Description").count() >0){
        item->setToolTip(0, child.attribute("Description"));
    }

    if(child.attribute("Range").count() > 0){
        item->setToolTip(1, child.attribute("Range"));
    }

    if(parent.attribute("Value").count()>0){
        item->setText(3, parent.attribute("Value"));
    }else if(child.attribute("DefaultValue").count()>0){
        item->setText(3, child.attribute("DefaultValue"));
    }

    if(paraType != 1){
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }

    return item;
}

void Sdp2dProcessJobTreeWidget::parameterOptionChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if(!m_paraTreeInited) return;
    if(column != 3) return;
    if(item->data(1, Qt::EditRole).toString().compare("Option") != 0) return;


    QString itemValue = item->data(3 , Qt::EditRole).toString();
    QString parentName = item->data(0 , Qt::EditRole).toString();

    //cout <<" Name=" << itemValue.toStdString().c_str() <<" Parent=" << parentName.toStdString().c_str() << endl;

    QDomElement root = m_domdoc->documentElement();
    QDomElement result;
    findDomElementForParaOption(parentName, itemValue, root, result);

    int nc = item->childCount();
    //cout << "name = " << result.attribute("Name").toStdString().c_str() << " child="<< nc << endl;
    for(int i=nc-1; i>=0; i--) item->removeChild(item->child(i));
    fillParametersWithDom(item, result);
    expandAll();
}

void Sdp2dProcessJobTreeWidget::findDomElementForParaOption(QString& parentName, QString& itemValue, QDomElement& parent, QDomElement& result)
{
    QDomNode n = parent.firstChild();

    while(!n.isNull()) {
        QDomElement e = n.toElement(); // try to convert the node to an element.

        if(e.tagName().compare("ParamsGroup") == 0 || e.tagName().compare("ParaOptions")==0){
            if(e.tagName().compare("ParamsGroup") == 0 && parent.tagName().compare("ParaOptions")==0){
                //cout << " node = "<< e.attribute("Name").toStdString().c_str() << " parent=" << parent.attribute("Name").toStdString().c_str() << endl;
                if(e.attribute("Name").compare(itemValue) ==0 && parent.attribute("Name").compare(parentName) ==0){
                    //cout << " node = "<< e.attribute("Name").toStdString().c_str()  << " item=" << itemValue.toStdString().c_str() << endl;
                    result = e;
                    break;
                }
            }
            findDomElementForParaOption(parentName, itemValue, e, result);
        }
        n = n.nextSibling();
    }
}
