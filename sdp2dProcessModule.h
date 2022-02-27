#ifndef SDP2DPROCESSMODULE_H
#define SDP2DPROCESSMODULE_H

#include <QObject>

class SeismicDataProcessing2D;
class Sdp2dQDomDocument;

class Sdp2dProcessModule : public QObject
{
    Q_OBJECT

public:
    explicit Sdp2dProcessModule(SeismicDataProcessing2D *parent = nullptr);

    virtual bool setupParameters(Sdp2dQDomDocument* para) = 0;
    virtual void processCurrentGather(float** indata, float** outdata, int ntr)  = 0;
    virtual void processWholeData()  = 0;

protected:
    SeismicDataProcessing2D* m_mainWindow;
    QString m_moduleName;

signals:

};

#endif // SDP2DPROCESSMODULE_H
