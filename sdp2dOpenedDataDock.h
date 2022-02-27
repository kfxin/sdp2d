#ifndef OPENEDDATADOCKWIDGET_H
#define OPENEDDATADOCKWIDGET_H

#include <QDockWidget>

QT_FORWARD_DECLARE_CLASS(QListWidget)
QT_FORWARD_DECLARE_CLASS(QString)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QSplitter)
QT_FORWARD_DECLARE_CLASS(SeismicDataProcessing2D)

QT_BEGIN_NAMESPACE
class QStatusBar;
class QMenu;
QT_END_NAMESPACE

class SeismicData2D;
class Sdp2dDisplayParamTab;
class SeismicDataProcessing2D;
class SeismicData2DPreStack;
class Sdp2dDataInfoTabs;

class Sdp2dOpenedDataDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit Sdp2dOpenedDataDockWidget(QList<SeismicData2D*>& sd2dlist, SeismicData2D* current_sd2d, SeismicDataProcessing2D *parent = nullptr);
    ~Sdp2dOpenedDataDockWidget();

    void appendNewFileToList(SeismicData2D* current_sd2d);
    void setFileAsCurrentItem(QString fileName);
    void showFileInfoTabs(void);
    SeismicData2D* removeCurrentItemFromDataList(void);
    void removeFilenameFromDataList(SeismicData2D* sd2d);
    void createFileInfoDock(SeismicData2D* sd2d, bool forceFInfoVisible = false);
    void hideFileInfoTabs(void);
    void showDisplayParameterTab(void);
    Sdp2dDisplayParamTab* getDisplayParamTabPointer(void);

public slots:


private:

    SeismicDataProcessing2D* m_mainWindow;
    QMenu* m_myMenu;
    QStatusBar* m_statusbar;
    QListWidget* m_datalist;
    QList<SeismicData2D*>& m_sd2dlist;

    Sdp2dDataInfoTabs* m_infotabs;
    QHBoxLayout *m_layout;
    QSplitter* m_page;

    QAction* m_displayAmpDlg;

    SeismicData2DPreStack* find2DSeismicDataPoint(QListWidgetItem *item);
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void on_ListWidget_customContextMenuRequested(const QPoint &pos);
    void displayScattersDlg();
    void displayCDPFoldDlg();
    void displayAmplitudeDlg();

    void on_mainDataListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_mainDataListWidget_itemDoubleClicked(QListWidgetItem *item);

};

#endif // OPENEDDATADOCKWIDGET_H
