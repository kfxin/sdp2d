#ifndef DATAINFODOCKWIDGET_H
#define DATAINFODOCKWIDGET_H

#include <QTabWidget>

QT_BEGIN_NAMESPACE
class QTabWidget;
class QDialog;
class QStatusBar;
QT_END_NAMESPACE


class Sdp2dDisplayParamTab;
class SeismicDataProcessing2D;
class SeismicData2D;
class Sdp2dEbcdicHeaderTab;
class Sdp2dBinaryHeaderTab;
class Sdp2dFileInfoTab;

class Sdp2dDataInfoTabs : public QTabWidget
{
    Q_OBJECT

public:
    explicit Sdp2dDataInfoTabs(SeismicData2D* sd2d, SeismicDataProcessing2D* mainWindow, QWidget *parent = nullptr);
    ~Sdp2dDataInfoTabs();

    void updateInformatiom(SeismicData2D* sd2d, bool forceFInfoVisible=false);
    Sdp2dDisplayParamTab* getDisplayParamTabPointer(void);
    void showDisplayParameterTab(void);

private:

    Sdp2dEbcdicHeaderTab* m_ebcHeaderTab;
    Sdp2dBinaryHeaderTab* m_binHeaderTab;
    Sdp2dFileInfoTab* m_fileInfoTab;
    Sdp2dDisplayParamTab* m_displayParamTab;
    QStatusBar* m_statusbar;

    void setTabInvisible(int index);
};


#endif // DATAINFODOCKWIDGET_H
