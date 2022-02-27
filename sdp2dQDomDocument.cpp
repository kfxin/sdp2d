#include "sdp2dQDomDocument.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QString>

#include <iostream>
#include <sys/stat.h>

#include <stdlib.h>

using namespace std;

Sdp2dQDomDocument::Sdp2dQDomDocument(const QString& name) : QDomDocument(name)
{

}

Sdp2dQDomDocument::~Sdp2dQDomDocument()
{

}

QString Sdp2dQDomDocument::getModuleName()
{
    QDomElement root = documentElement();
    return root.tagName();
}

void Sdp2dQDomDocument::setParameterInGroup(QString paraName, QString paraValue, QString groupName)
{
    QDomElement root = documentElement();
    QDomElement e;

    if(groupName.count() == 0){
        QDomNode n = root.firstChild();
        while(!n.isNull()) {
            e = n.toElement();
            QString tag  = e.tagName();
            QString name = e.attribute("Name");
            if(tag.compare("Parameters") == 0 && paraName.compare(name) == 0){
                QDomElement child = e.firstChildElement(paraName);
                e.setAttribute("Value", paraValue);
                return;
            }
            n = n.nextSibling();
        }
    }  else {
         getDomEmelemtOfParameter(root, e, paraName, groupName);
    }

    e.setAttribute("Value", paraValue);
}

QString Sdp2dQDomDocument::getParameterInGroup(QString paraName, QString groupName)
{
    QDomElement root = documentElement();
    QDomElement e;

    if(groupName.count() == 0){
        QDomNode n = root.firstChild();
        while(!n.isNull()) {
            e = n.toElement();
            QString tag  = e.tagName();
            QString name = e.attribute("Name");
            if(tag.compare("Parameters") == 0 && paraName.compare(name) == 0){
                return e.attribute("Value");
            }
            n = n.nextSibling();
        }
    }  else {
         getDomEmelemtOfParameter(root, e, paraName, groupName);
    }

    return e.attribute("Value");
}

void Sdp2dQDomDocument::getDomEmelemtOfParameter(QDomElement& parent, QDomElement& result, QString paraName, QString parentName)
{
    QDomNode n = parent.firstChild();
    QString parentTag = parent.tagName();
    QString parentComp;
    if(parentTag.compare("ParamsGroup") == 0) parentComp=parent.attribute("Name");
    if(parentTag.compare("ParaOptions") == 0) parentComp=parent.attribute("Value");

    while(!n.isNull()) {
        QDomElement e = n.toElement();
        QString tag  = e.tagName();
        QString name = e.attribute("Name");

        if(tag.compare("Parameters") == 0 && paraName.compare(name) == 0 && parentName.compare( parentComp) == 0){
            result = e;
            return;
        } else {
            getDomEmelemtOfParameter(e, result, paraName, parentName);
            if(result.isNull() == false ) return;
        }
        n = n.nextSibling();
    }

}

void Sdp2dQDomDocument::setParameterInOption(QString paraName, QString paraValue, QString optionName)
{
    QDomElement root = documentElement();
    QDomElement result;

    QDomNode n = root.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        QString tag  = e.tagName();
        //QString name = e.attribute("Name");
        if(tag.compare("Parameters") != 0 ){
            getDomEmelemtOfParameter(root, result, paraName, optionName);
            //cout << "within function:  " << paraName.toStdString().c_str() << " = " << result.attribute("Value").toStdString().c_str() << endl;
            if(result.isNull() == false) {
                QDomElement child = result.firstChildElement(paraName);
                child.setAttribute("Value", paraValue);
                return;
            }
        }
        n = n.nextSibling();
    }
}


QString Sdp2dQDomDocument::getParameterInOption(QString paraName, QString optionName)
{
    QDomElement root = documentElement();
    QDomElement result;

    QDomNode n = root.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        QString tag  = e.tagName();
        //QString name = e.attribute("Name");
        if(tag.compare("Parameters") != 0 ){
            getDomEmelemtOfParameter(root, result, paraName, optionName);
            //cout << "within function:  " << paraName.toStdString().c_str() << " = " << result.attribute("Value").toStdString().c_str() << endl;
            if(result.isNull() == false) break;
        }
        n = n.nextSibling();
    }
    //cout << "before exist getParameterInOption " << paraName.toStdString().c_str() << " = " << result.attribute("Value").toStdString().c_str() << endl;
    return result.attribute("Value");
}

void Sdp2dQDomDocument::setOptionValue(QString optionName, QString optionValue, QString parentName)
{
    QDomElement root = documentElement();
    QDomElement result;

    QDomNode n = root.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        QString tag  = e.tagName();
        //QString name = e.attribute("Name");
        if(tag.compare("Parameters") != 0 ){
            getDomEmelemtOfOperation(root, result, optionName, parentName);
            if(result.isNull() == false) break;
        }
        n = n.nextSibling();
    }

    result.setAttribute("Value", optionValue);
}


QString Sdp2dQDomDocument::getOptionValue(QString optionName, QString parentName)
{
    QDomElement root = documentElement();
    QDomElement result;

    QDomNode n = root.firstChild();
    while(!n.isNull()) {
        QDomElement e = n.toElement();
        QString tag  = e.tagName();
        //QString name = e.attribute("Name");
        if(tag.compare("Parameters") != 0 ){
            getDomEmelemtOfOperation(root, result, optionName, parentName);
            if(result.isNull() == false) break;
        }
        n = n.nextSibling();
    }

    return result.attribute("Value");
}

void Sdp2dQDomDocument::getDomEmelemtOfOperation(QDomElement& parent, QDomElement& result, QString optionName, QString parentName)
{
    QDomNode n = parent.firstChild();
    QString parentTag = parent.tagName();
    QString parentComp;
    if(parentTag.compare("ParamsGroup") == 0) parentComp=parent.attribute("Name");
    if(parentTag.compare("ParaOptions") == 0) parentComp=parent.attribute("Value");

    while(!n.isNull()) {
        QDomElement e = n.toElement();
        if(e.tagName().compare("ParaOptions") == 0 && optionName.compare(e.attribute("Name")) == 0 && parentName.compare(parentComp) == 0){
            result =  e;
            return;
        }
        getDomEmelemtOfOperation(e, result, optionName, parentName);
        if(result.isNull() == false) return;

        n = n.nextSibling();
    }
}
