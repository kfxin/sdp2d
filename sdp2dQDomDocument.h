#ifndef SDP2DQDOMDOCUMENT_H
#define SDP2DQDOMDOCUMENT_H

#include <QDomDocument>

class Sdp2dQDomDocument : public QDomDocument
{    
public:
    Sdp2dQDomDocument(const QString& name);
     ~Sdp2dQDomDocument();

    QString getModuleName();
    QString getParameterInGroup(QString paraName, QString groupName = QString());
    void setParameterInGroup(QString paraName, QString paraValue, QString groupName = QString());

    void getDomEmelemtOfParameter(QDomElement& parent, QDomElement& result, QString paraName, QString groupName);

    void setParameterInOption(QString paraName, QString paraValue, QString optionName);
    QString getParameterInOption(QString paraName, QString optionName);

    void setOptionValue(QString optionName, QString optionValue, QString parentName= QString());
    QString getOptionValue(QString optionName, QString parentName = QString());    
    void getDomEmelemtOfOperation(QDomElement& parent, QDomElement& result, QString optionName, QString parentName);

};

#endif // SDP2DQDOMDOCUMENT_H
