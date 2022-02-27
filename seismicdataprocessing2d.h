#ifndef SEISMICDATAPROCESSING2D_H
#define SEISMICDATAPROCESSING2D_H

#include <QMainWindow>
#include <QList>
#include <QChart>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class SeismicDataProcessing2D; }

class QListWidget;
class QListWidgetItem;
class QHBoxLayout;
class QLineEdit;
class QVBoxLayout;
class QComboBox;
class QToolBar;
class QSplitter;
class QDialog;
class QAction;
QT_END_NAMESPACE

class QCPRange;
class SeismicData2D;
class Sdp2dFileInformationTabs;
class Sdp2dProcessJobDockWidget;
class Sdp2dOpenedDataDockWidget;
class Sdp2dMapDiaplayDockWidget;
class Sdp2dFrequencyAnalysisDock;
class Sdp2dSegy;
class Sdp2dProcessModule;
class Sdp2dQDomDocument;
class Sdp2dDisplayParamTab;


using namespace QtCharts;

enum InteractiveFunctions{
     None=0,
     FrequencyAnalysis=1,
     BadTraceSelection=2,
     StackVelAnalysis=3,
     PickingMute=4,
     MeasureLinearVelcity=5
};


class SeismicDataProcessing2D : public QMainWindow
{
    Q_OBJECT

public:
    SeismicDataProcessing2D(QWidget *parent = nullptr);
    ~SeismicDataProcessing2D();

    void setCurrentDataPointer(SeismicData2D* sd2d);
    SeismicData2D* getCurrentDataPointer(void);
    QStatusBar* getStatusbarPointer();

    bool isGatherDisplayFitToScrollWindow(void);

    void setToolBarDisplayType(int val);
    void setVisibleOfJobDock(bool visible);

    void processCurrentGather(Sdp2dQDomDocument* paradom);
    void processCurrentGather(QString moduleName);
    void processCurrentGather(QString moduleName, float** indata, float** oudata, int ntr);
    void processWholeData(Sdp2dQDomDocument* paradom);
    void setupParameters(Sdp2dQDomDocument* paradom);

    void uncheckAllToolActions(void);

    int getInteractiveFunction(void);
    void setInteractiveFunction(InteractiveFunctions val);

    bool isProcessEnabled(void) const {return m_enableProcess;}

    void enableGatherProcess() {  m_enableProcess = true; }

    void addWidgetToTheCentralSplitView(QWidget* w);

    Sdp2dProcessJobDockWidget* getProcessJobPointer(void) const { return m_pjobdock; }

    void setCheckStatusOfOpenFilesMenuItem(bool checked);

public slots:
    void disableGatherProcess();
    //

private:
    Ui::SeismicDataProcessing2D *ui;

    void setupProcessingModules(void);

    QVBoxLayout* createLoadFileDataTypeDialog(QWidget* pParentWidget);
    void createStartupInfoLayout();
    bool isIndexFileExist(QString fileName);
    void setCurrendDisplayDataPointer(void);
    void insertFilenameToRecentFilesList(QString fileName);
    void saveRecentOpenedFileList(void);
    void setupMainWindowLayout(void);
    void cleanMainDisplayArea(bool all=false);

    void recreateDataObjectWithDataType(QString fileName);
    void addFile2RecentOpenedFileList(QString fileName);
    void createLoadingDataObject(QString fileName);
    bool isTheFileAlreadyOpened(QString fileName);

    void hideDisplaysOfSD2D(SeismicData2D* sd2d);
    void showDisplaysOfSD2D(SeismicData2D* sd2d);
    void loadSEGYDataUseFileName(QString fileName, bool checkSetting = false);

    void createToolBar();
    bool cannotFindTheFileSelectedFromList(QListWidgetItem *item);

    void generateIntermediateFileWithThread(void);
    void getTraceAmplitudeWithThread(void);

    void disableAllActions(bool disable);

private: signals:
    void closeDataTypeInputDlg();
    void getTraceAmplitude(Sdp2dSegy* sp);
    void generateIntermediateFile(Sdp2dSegy* sp);

private slots:
    void action_quit_all();
    void action_load_segy_file();
    void action_save_current_file();
    void action_close_current_file();
    void action_display_traceheader(bool checked);
    void action_display_wt_gather(bool checked);
    void action_display_vd_gather(bool checked);
    void action_display_next_gather();
    void action_display_previous_gather();
    void action_display_first_gather();
    void action_display_last_gather();
    void action_display_zoomin();
    void action_display_zoomout();
    void action_display_zoomfit(bool checked);
    void action_display_parameters();
    void action_processing_showdock(bool checked);

    void action_show_open_file_list(bool checked);
    void action_about_SDP2D();

    void action_tools_FrequencyAnalysis(bool checked);
    void action_tools_BadTracepicking(bool checked);
    void action_tools_StackVelocityAnalysis(bool checked);
    void action_tools_PickingMute(bool checked);
    void action_tools_buildModuleParameters();

    void on_startupSelectFileBtn_clicked();
    void on_startupOKBtn_clicked();
    void on_sartupCancelBtn_clicked();
    void on_startupOpenFileLineEdit_returnPressed();
    void on_startupRecentFIlesListWidget_itemDoubleClicked(QListWidgetItem *item);

    void setCDPSpacing();
    void setOffsetSpacing();
    void setMinOffset(const QString &text);
    void setMaxOffset(const QString &text);
    void setTraceDataSwapbytes(QString item);

    void on_startupRecentFIlesListWidget_itemClicked(QListWidgetItem *item);

    void loadFileOKBtnClicked(QString fileName);
    void loadFileCancelBtnClicked(void);

    void openRecent();   

private:
     QList<SeismicData2D*> m_lsd2d;
     QMap<QString, Sdp2dProcessModule*> m_pmodules;

     SeismicData2D* m_current_sd2d;
     SeismicData2D* m_loading_sd2d;

     Sdp2dOpenedDataDockWidget* m_datadock;     
     Sdp2dProcessJobDockWidget* m_pjobdock;     
     Sdp2dFileInformationTabs* m_finfotabs;
     Sdp2dDisplayParamTab* m_displayParaTab;
     QToolBar *m_displayToolBar;
     QToolBar *m_fileToolBar;
     QToolBar *m_toolsToolBar;

     QSplitter* m_displayView;

     QLineEdit *m_cdpSpaceLineEdit;
     QLineEdit *m_offSpaceLineEdit;
     QLineEdit *m_minOffLineEdit;
     QLineEdit *m_maxOffLineEdit;
     QComboBox *m_tracesbcbox;
     QComboBox *m_datatypebox;
     QDialog* m_fileTypeSetDlg;
     QList<QAction*> m_recentFileActionList;

     QHBoxLayout *m_fileInfoHLayout;

     QChart* m_chart;

     int m_maxRecentFiles;

     int m_startupParaFlag;
     bool m_analysisDataFlag;
     int m_datatype;

     QString m_fileName;     

     bool m_enableProcess;
     bool m_zoomfit;
     int m_screenHeight;
     int m_screenWidth;

     QThread* m_workerThread;
     QTimer*  m_timer;
     Sdp2dSegy* m_tempsegy;

};
#endif // SEISMICDATAPROCESSING2D_H
