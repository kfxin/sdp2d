#include "seismicdataprocessing2d.h"
#include "ui_seismicdataprocessing2d.h"
#include "sdp2dFileInformationTabs.h"
#include "sdp2dMainGatherDisplayArea.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "sdp2dOpenedDataDock.h"
#include "sdp2dMapDiaplayDock.h"
#include "sdp2dProcessJobDock.h"
#include "sdp2dDisplayParamTab.h"
#include "sdp2dFrequencyAnalysisDock.h"
#include "sdp2dModuleParametersSetupDlg.h"
#include "sdp2dQDomDocument.h"
#include "qcustomplot.h"
#include "sdp2dUtils.h"
#include "sdp2dSegy.h"
#include "sdp2dProcessModule.h"
#include "sdp2dProcessedGatherDisplayArea.h"
#include "sdp2dPMElevationStaticCorrection.h"
#include "sdp2dPMFilter.h"
#include "sdp2dPMSuGain.h"


#include <QMainWindow>
#include <QFileInfo>
#include <QDockWidget>
#include <QListWidget>
#include <QStringList>
#include <QLabel>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSpacerItem>
#include <QFileDialog>
#include <QGroupBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidgetItem>
#include <QTextStream>
#include <QErrorMessage>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPixmap>
#include <QColorSpace>
#include <QImageReader>
#include <QLineSeries>
#include <QValueAxis>
#include <QPainter>
#include <QPen>
#include <QFile>

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace std;

SeismicDataProcessing2D::SeismicDataProcessing2D(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SeismicDataProcessing2D)
{
    ui->setupUi(this);

    setupProcessingModules();

    ui->action_display_zoomin->setVisible(false);
    ui->action_display_zoomout->setVisible(false);
    ui->action_display_zoomfit->setCheckable(true);
    ui->action_display_zoomfit->setChecked(true);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    m_screenHeight = screenGeometry.height();
    m_screenWidth  = screenGeometry.width();

    //cout << "m_screenHeight = " << m_screenHeight << endl;
    //cout << "m_screenWidth = " << m_screenWidth << endl;

    m_startupParaFlag = 0;
    m_analysisDataFlag = true;

    m_datadock = nullptr;
    m_pjobdock = nullptr;

    m_tempsegy = nullptr;

    m_displayToolBar = nullptr;
    m_fileToolBar = nullptr;
    m_toolsToolBar = nullptr;
    m_displayView = nullptr;

    m_finfotabs = nullptr;
    m_fileInfoHLayout =  nullptr;

    m_datatypebox =  nullptr;

    m_loading_sd2d = nullptr;
    m_current_sd2d = nullptr;

    m_datatype = SeismicDataType::PreStack;    

    QFileInfo sd2ddir(QDir::homePath() + QString("/.sdp2d"));
    ui->statusbar->showMessage(sd2ddir.absoluteFilePath());
    if(!sd2ddir.isDir()){
        char command[20];
        sprintf(command,"mkdir ~/.sdp2d");
        system(command);
    }

    QString fileName = QDir::homePath() + QString("/.sdp2d/recent");
    //QFileInfo sd2dfinfo(fileName);
    //ui->statusbar->showMessage(sd2dfinfo.absoluteFilePath());
    QFile sd2dfile(fileName);
    if(sd2dfile.exists()){
        if(sd2dfile.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&sd2dfile);
            while (!in.atEnd()) {
                   QString qline = in.readLine();
                   ui->startupRecentFIlesListWidget->addItem(qline);
            }
        }else{
            cout <<"the recent file cannot open" << endl;
        }
        sd2dfile.close();
    }

    m_maxRecentFiles = 20;
    QAction* recentFileAction = 0;
    for(auto i = 0; i < m_maxRecentFiles; ++i){
        recentFileAction = new QAction(this);
        recentFileAction->setVisible(false);
        QObject::connect(recentFileAction, &QAction::triggered, this, &SeismicDataProcessing2D::openRecent);
        m_recentFileActionList.append(recentFileAction);
    }
    //QMenu* recentFilesMenu = ui->menusFile->addMenu(tr("Open Recent"));
    QMenu* recentFilesMenu = new QMenu(tr("Recent Files..."));
    ui->menusFile->insertMenu(ui->action_quit_all, recentFilesMenu);
    ui->menusFile->insertSeparator(ui->action_quit_all);
    for(auto i = 0; i < m_maxRecentFiles; ++i)
            recentFilesMenu->addAction(m_recentFileActionList.at(i));

    ui->statusbar->setSizeGripEnabled(true);

    connect(ui->action_quit_all, &QAction::triggered, this, &SeismicDataProcessing2D::action_quit_all);
    connect(ui->action_load_segy_file, &QAction::triggered, this, &SeismicDataProcessing2D::action_load_segy_file);
    connect(ui->action_saveas_segy_file, &QAction::triggered, this, &SeismicDataProcessing2D::action_save_current_file);
    connect(ui->action_close_current_file, &QAction::triggered, this, &SeismicDataProcessing2D::action_close_current_file);
    connect(ui->action_display_offsetelev, &QAction::toggled, this, &SeismicDataProcessing2D::action_display_traceheader);
    connect(ui->action_display_wt_gather, &QAction::triggered, this, &SeismicDataProcessing2D::action_display_wt_gather);
    connect(ui->action_display_vd_gather, &QAction::triggered, this, &SeismicDataProcessing2D::action_display_vd_gather);
    connect(ui->action_display_next_gather, &QAction::triggered, this, &SeismicDataProcessing2D::action_display_next_gather);
    connect(ui->action_display_previous_gather, &QAction::triggered, this, &SeismicDataProcessing2D::action_display_previous_gather);
    connect(ui->action_display_first_gather, &QAction::triggered, this, &SeismicDataProcessing2D::action_display_first_gather);
    connect(ui->action_display_last_gather, &QAction::triggered, this, &SeismicDataProcessing2D::action_display_last_gather);
    connect(ui->action_display_zoomin, &QAction::triggered, this, &SeismicDataProcessing2D::action_display_zoomin);
    connect(ui->action_display_zoomout, &QAction::triggered, this, &SeismicDataProcessing2D::action_display_zoomout);
    connect(ui->action_display_zoomfit, &QAction::toggled, this, &SeismicDataProcessing2D::action_display_zoomfit);
    connect(ui->action_display_parameters, &QAction::triggered, this, &SeismicDataProcessing2D::action_display_parameters);

    connect(ui->action_tools_FrequencyAnalysis, &QAction::toggled, this, &SeismicDataProcessing2D::action_tools_FrequencyAnalysis);
    connect(ui->action_tools_BadTraceSelection, &QAction::toggled, this, &SeismicDataProcessing2D::action_tools_BadTracepicking);
    connect(ui->action_tools_StackVelocityAnalysis, &QAction::triggered, this, &SeismicDataProcessing2D::action_tools_StackVelocityAnalysis);
    connect(ui->action_tools_PickingMute, &QAction::toggled, this, &SeismicDataProcessing2D::action_tools_PickingMute);

    connect(ui->action_tools_buildModuleParameters, &QAction::triggered, this, &SeismicDataProcessing2D::action_tools_buildModuleParameters);

    connect(ui->action_show_open_file_list, &QAction::triggered, this, &SeismicDataProcessing2D::action_show_open_file_list);
    connect(ui->action_about_SDP2D, &QAction::triggered, this, &SeismicDataProcessing2D::action_about_SDP2D);

    connect(ui->action_processing_showdock, &QAction::toggled, this, &SeismicDataProcessing2D::action_processing_showdock);

    m_workerThread = new QThread();
    m_timer = new QTimer();
    m_timer->setSingleShot(true);

}

SeismicDataProcessing2D::~SeismicDataProcessing2D()
{
    delete ui;

    while (!m_lsd2d.isEmpty())
        delete m_lsd2d.takeFirst();
    m_lsd2d.clear();

    m_workerThread->quit();
    m_workerThread->wait();

    delete m_pjobdock;
}

void SeismicDataProcessing2D::setupProcessingModules(void)
{
    m_pmodules["Filter"] = new Sdp2dPMFilter(this);
    m_pmodules["ElevStatic"] = new Sdp2dPMElevationStaticCorrection(this);    
    m_pmodules["SuGain"] = new Sdp2dPMSuGain(this);
}

void SeismicDataProcessing2D::action_quit_all()
{
    QApplication::exit();
}

void SeismicDataProcessing2D::action_load_segy_file()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "Open File", "/", "SEGY (*.segy *.sgy *.SEGY *.SGY)");

    bool checkSetting = true;
    loadSEGYDataUseFileName(fileName, checkSetting);
}

void SeismicDataProcessing2D::action_save_current_file()
{
    QString fileName = m_current_sd2d->getOutputSegyFileName();
    cout << "fileName=" << fileName.toStdString().c_str() << endl;
    if(fileName.count() < 1){
        m_pjobdock->showOutputFileView();
    } else {
        m_current_sd2d->outputSegyWithLocalFormat();
    }
}

void SeismicDataProcessing2D::action_close_current_file()
{    
    if(m_lsd2d.count() <= 0){
        QMessageBox message(QMessageBox::NoIcon,
                            "Warn", "Do you want to exit SD2D program?", QMessageBox::Yes | QMessageBox::No, NULL);
        if(message.exec() == QMessageBox::Yes)
        {
                action_quit_all();
                return;
        }
        return;
    }

    int itemIdx = 0;
    for(int i=0; i< m_lsd2d.count(); i++){
        if(m_current_sd2d == m_lsd2d.at(i)){
            itemIdx = i;
            break;
        }
    }

    m_datadock->removeFilenameFromDataList(m_current_sd2d);
    //m_lsd2d.removeOne(m_current_sd2d);
    m_lsd2d.takeAt(m_lsd2d.indexOf(m_current_sd2d));
    delete m_current_sd2d;

    //cout << "m_lsd2d.count="<< m_lsd2d.count()<<endl;

    if(m_lsd2d.count()>0){
        itemIdx = itemIdx-1;
        if(itemIdx<0) itemIdx=0;
        m_current_sd2d = m_lsd2d[itemIdx];
        string sname = m_current_sd2d->getSEGYFileName();
        const QString qname = QString::fromUtf8( sname.data(), sname.size() );
        m_datadock->setFileAsCurrentItem(qname);
        showDisplaysOfSD2D(m_current_sd2d);
    }else{
        m_current_sd2d = nullptr;
        m_datadock->hideFileInfoTabs();
        m_pjobdock->cleanTreeView();
        m_pjobdock->hide();
        disableAllActions(true);
    }
}

void SeismicDataProcessing2D::action_display_traceheader(bool checked)
{
    m_current_sd2d->setTraceHeaderDisplay(checked);
}

void SeismicDataProcessing2D::action_display_wt_gather(bool checked)
{    
    if(!checked && ui->action_display_vd_gather->isChecked() == false){
        ui->action_display_vd_gather->setChecked(true);
        m_current_sd2d->getDisplayParamTabPointer()->setDisplayType(1);
    }else{
        ui->action_display_vd_gather->setChecked(false);
        m_current_sd2d->getDisplayParamTabPointer()->setDisplayType(2);
    }
}

void SeismicDataProcessing2D::action_display_vd_gather(bool checked)
{
    if(!checked && ui->action_display_wt_gather->isChecked() == false){
        ui->action_display_wt_gather->setChecked(true);
        m_current_sd2d->getDisplayParamTabPointer()->setDisplayType(2);
    }else{
        ui->action_display_wt_gather->setChecked(false);
        m_current_sd2d->getDisplayParamTabPointer()->setDisplayType(1);
    }
}

void SeismicDataProcessing2D::action_display_next_gather()
{
    Sdp2dMainGatherDisplayArea* display = m_current_sd2d->getfocusedDisplayPointer();
    if(display ==  nullptr) return;
    int step = m_current_sd2d->getGroupStep();
    int gidx = m_current_sd2d->getPlotGroupIndex() + step;
    int interactiveFunction = m_current_sd2d->getInteractiveFunction();

    if(interactiveFunction ==  InteractiveFunctions::StackVelAnalysis){
        m_displayParaTab->checkCDPGatherIndexForVelAna(gidx);
        SeismicData2DPreStack* sd2dp = dynamic_cast<SeismicData2DPreStack*>(m_current_sd2d);
        sd2dp->prepareStackVelocityAnalysis();
    } else {
        m_current_sd2d->setPlotGroupIndex(gidx);
    }
}

void SeismicDataProcessing2D::action_display_previous_gather()
{
    Sdp2dMainGatherDisplayArea* display = m_current_sd2d->getfocusedDisplayPointer();
    if(display ==  nullptr) return;
    int step = m_current_sd2d->getGroupStep();
    int gidx = m_current_sd2d->getPlotGroupIndex() - step;

    int interactiveFunction = m_current_sd2d->getInteractiveFunction();
    if(interactiveFunction ==  InteractiveFunctions::StackVelAnalysis){
        m_displayParaTab->checkCDPGatherIndexForVelAna(gidx);
        SeismicData2DPreStack* sd2dp = dynamic_cast<SeismicData2DPreStack*>(m_current_sd2d);
        sd2dp->prepareStackVelocityAnalysis();
    } else {
        m_current_sd2d->setPlotGroupIndex(gidx);
    }
}

void SeismicDataProcessing2D::action_display_first_gather()
{
    Sdp2dMainGatherDisplayArea* display = m_current_sd2d->getfocusedDisplayPointer();
    if(display ==  nullptr) return;

    int interactiveFunction = m_current_sd2d->getInteractiveFunction();
    int minNTraces = 1;
    if(interactiveFunction ==  InteractiveFunctions::StackVelAnalysis){
        minNTraces = 10;
    }

    m_current_sd2d->setPlotGroupToTheFirst(minNTraces);

    if(interactiveFunction ==  InteractiveFunctions::StackVelAnalysis){
        SeismicData2DPreStack* sd2dp = dynamic_cast<SeismicData2DPreStack*>(m_current_sd2d);
        sd2dp->prepareStackVelocityAnalysis();
    }
}

void SeismicDataProcessing2D::action_display_last_gather()
{
    Sdp2dMainGatherDisplayArea* display = dynamic_cast<Sdp2dMainGatherDisplayArea*>(m_current_sd2d->getfocusedDisplayPointer());
    if(display ==  nullptr) return;

    int interactiveFunction = m_current_sd2d->getInteractiveFunction();
    int minNTraces = 1;
    if(interactiveFunction ==  InteractiveFunctions::StackVelAnalysis){
        minNTraces = 10;
    }

    m_current_sd2d->setPlotGroupToTheLast(minNTraces);

    if(interactiveFunction ==  InteractiveFunctions::StackVelAnalysis){
        SeismicData2DPreStack* sd2dp = dynamic_cast<SeismicData2DPreStack*>(m_current_sd2d);
        sd2dp->prepareStackVelocityAnalysis();
    }
}

void SeismicDataProcessing2D::action_display_zoomin()
{

}

void SeismicDataProcessing2D::action_display_zoomout()
{

}

void SeismicDataProcessing2D::action_display_zoomfit(bool checked)
{    
    if(m_current_sd2d->getInputGatherDisplayPointer() == nullptr) return;
    m_current_sd2d->setPlotFitZoom(checked);
    if(checked) {
        m_current_sd2d->getInputGatherDisplayPointer()->resizeToFitTheDisplay();
        Sdp2dProcessedGatherDisplayArea* odp = m_current_sd2d->getProcessedGatherDisplayPointer();
        if(odp != nullptr) {
            odp->resizeToFitTheDisplay();
        }
    }
}

void SeismicDataProcessing2D::action_display_parameters()
{
    m_datadock->showDisplayParameterTab();
}

void SeismicDataProcessing2D::action_processing_showdock(bool checked)
{
    if(checked){
        m_pjobdock->show();        
    } else {
        m_pjobdock->hide();
    }    
}

void SeismicDataProcessing2D::action_tools_FrequencyAnalysis(bool checked)
{
    int interactiveFunction = m_current_sd2d->getInteractiveFunction();

    if(checked && interactiveFunction == InteractiveFunctions::StackVelAnalysis){
        m_current_sd2d->removeAdjunctiveDisplays();
    }

    m_current_sd2d->cleanMainDisplay(true);
    if(checked){
        m_current_sd2d->setInteractiveFunction(InteractiveFunctions::FrequencyAnalysis);
        m_pjobdock->interactiveFunctionEnabled(InteractiveFunctions::FrequencyAnalysis);
    }else{        
        if(interactiveFunction == InteractiveFunctions::None) return;
        m_current_sd2d->setInteractiveFunction(InteractiveFunctions::None);
        m_pjobdock->interactiveFunctionDisAbled(InteractiveFunctions::FrequencyAnalysis);
    }

    m_current_sd2d->getInputGatherDisplayPointer()->disableHideGather(checked);
}

void SeismicDataProcessing2D::action_tools_BadTracepicking(bool checked)
{
    m_current_sd2d->cleanMainDisplay(true);
    if(checked){
        m_current_sd2d->setInteractiveFunction(InteractiveFunctions::BadTraceSelection);
        m_pjobdock->interactiveFunctionEnabled(InteractiveFunctions::BadTraceSelection);
        m_current_sd2d->removeAdjunctiveDisplays();
    } else {
        int interactiveFunction = m_current_sd2d->getInteractiveFunction();
        if(interactiveFunction == InteractiveFunctions::None) return;
        m_current_sd2d->setInteractiveFunction(InteractiveFunctions::None);
        m_pjobdock->interactiveFunctionDisAbled(InteractiveFunctions::BadTraceSelection);
    }

    m_current_sd2d->getInputGatherDisplayPointer()->setBadTracesWithinOneGather(checked);
}

void SeismicDataProcessing2D::action_tools_StackVelocityAnalysis(bool checked)
{    
    cout << "action_tools_StackVelocityAnalysis checked=" << checked << endl;
    SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack *>(m_current_sd2d);
    sd2d->removeAdjunctiveDisplays();
    sd2d->cleanMainDisplay(true);
    if(checked){
        sd2d->setInteractiveFunction(InteractiveFunctions::StackVelAnalysis);
        m_pjobdock->interactiveFunctionEnabled(InteractiveFunctions::StackVelAnalysis);
        if(m_displayParaTab->getGatherType() != DisplayGatherType::CommonDepthPoint){
            m_displayParaTab->setGatherType(DisplayGatherType::CommonDepthPoint);
        }else{
            m_displayParaTab->checkCDPGatherIndexForVelAna();
        }
        ui->action_display_offsetelev->setChecked(false);
        ui->action_display_offsetelev->setEnabled(false);

        sd2d->loadPickedNMOVelocities();
        m_pjobdock->runJob();
    } else {
        int interactiveFunction = sd2d->getInteractiveFunction();
        if(interactiveFunction == InteractiveFunctions::None) return;
        sd2d->setInteractiveFunction(InteractiveFunctions::None);
        m_pjobdock->interactiveFunctionDisAbled(InteractiveFunctions::StackVelAnalysis);
        ui->action_display_offsetelev->setEnabled(true);
        sd2d->unloadPickedNMOVelocities();
    }    

}

void SeismicDataProcessing2D::action_tools_PickingMute(bool checked)
{   
    m_current_sd2d->cleanMainDisplay(true);

    if(checked){         
        m_current_sd2d->setInteractiveFunction(InteractiveFunctions::PickingMute);
        m_pjobdock->interactiveFunctionEnabled(InteractiveFunctions::PickingMute);
        m_current_sd2d->removeAdjunctiveDisplays();
    } else {
        int interactiveFunction = m_current_sd2d->getInteractiveFunction();
        if(interactiveFunction == InteractiveFunctions::None) return;
        m_current_sd2d->setInteractiveFunction(InteractiveFunctions::None);
        m_pjobdock->interactiveFunctionDisAbled(InteractiveFunctions::PickingMute);
    }

    SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack*>(m_current_sd2d);
    sd2d->setMuteValuesOfCurrentGather(checked);    
}

void SeismicDataProcessing2D::action_tools_buildModuleParameters()
{
    Sdp2dModuleParametersSetupDlg* paraSetupDlg = new Sdp2dModuleParametersSetupDlg(this);
    paraSetupDlg->show();
}

void SeismicDataProcessing2D::action_show_open_file_list(bool checked)
{
    if(checked) m_datadock->show();
    else m_datadock->hide();
}

void SeismicDataProcessing2D::action_about_SDP2D()
{
    QMessageBox message(QMessageBox::NoIcon,
        "SDP2D", "This 2D Seismic Data Processing software is developed by Xin Kefeng using Qt. This software is under the LGPL v3 and GPL v3 open source license.", QMessageBox::Yes, NULL);
    message.exec();
    return;
}

void SeismicDataProcessing2D::setCheckStatusOfOpenFilesMenuItem(bool checked)
{
    ui->action_show_open_file_list->setChecked(checked);
}

void SeismicDataProcessing2D::disableAllActions(bool disable)
{
    ui->action_saveas_segy_file->setDisabled(disable);
    ui->action_display_offsetelev->setDisabled(disable);
    ui->action_display_wt_gather->setDisabled(disable);
    ui->action_display_vd_gather->setDisabled(disable);
    ui->action_display_next_gather->setDisabled(disable);
    ui->action_display_previous_gather->setDisabled(disable);
    ui->action_display_first_gather->setDisabled(disable);
    ui->action_display_last_gather->setDisabled(disable);
    ui->action_display_zoomfit->setDisabled(disable);
    ui->action_display_parameters->setDisabled(disable);
    ui->action_processing_showdock->setDisabled(disable);
    ui->action_tools_FrequencyAnalysis->setDisabled(disable);
    ui->action_tools_StackVelocityAnalysis->setDisabled(disable);
    ui->action_tools_BadTraceSelection->setDisabled(disable);
    ui->action_tools_PickingMute->setDisabled(disable);
    ui->action_close_current_file->setDisabled(disable);
    if(disable){
        ui->action_processing_showdock->setChecked(false);
    }
}

void SeismicDataProcessing2D::on_startupSelectFileBtn_clicked()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "Open File", "/", "SEGY (*.segy *.sgy *.SEGY *.SGY)");

    if (QFile::exists(fileName)){
        ui->startupOpenFileLineEdit->setText(fileName);
        cleanMainDisplayArea();
        createLoadingDataObject(fileName);
        createLoadFileDataTypeDialog(ui->startupFileInfoFrame);
    }else{
        ui->statusbar->showMessage("No file is selected !"+fileName);
    }
}

void SeismicDataProcessing2D::on_startupOKBtn_clicked()
{
    const QString fileName = ui->startupOpenFileLineEdit->text();

    if(fileName.isEmpty()) return;
    cout << "try to set file info for " << fileName.toStdString().c_str()<< endl;
    if(ui->startupRecentFIlesListWidget == nullptr ) { //clean central area
        cout << "create docks. m_startupParaFlag=" << m_startupParaFlag <<  endl;
        if(m_startupParaFlag != 15) return;
        recreateDataObjectWithDataType(fileName);
        SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack *>(m_loading_sd2d);

        sd2d->setupDataIndex();

        setCurrendDisplayDataPointer();
        cleanMainDisplayArea(true);
        setupMainWindowLayout();
        addFile2RecentOpenedFileList(fileName);
        this->showMaximized();
    }else{ //after file selectf, or data type setting
        cout << "set data type window" << endl;
        cleanMainDisplayArea();
        createLoadingDataObject(fileName);
        createLoadFileDataTypeDialog(ui->startupFileInfoFrame);
    }

}

void SeismicDataProcessing2D::on_sartupCancelBtn_clicked()
{
    QApplication::exit();
}

void SeismicDataProcessing2D::on_startupOpenFileLineEdit_returnPressed()
{
    const QString fileName = ui->startupOpenFileLineEdit->text();
    if(!QFile(fileName).exists()){
        ui->statusbar->showMessage("Cannot find the file:"+fileName);
        return;
    }

    on_startupOKBtn_clicked();
}

void SeismicDataProcessing2D::on_startupRecentFIlesListWidget_itemDoubleClicked(QListWidgetItem *item)
{

    if(cannotFindTheFileSelectedFromList(item)){
        return;
    }
    const QString fileName = item->text();

    cout << "before check idx file exist" << endl;
    if(isIndexFileExist(fileName)){ // if no index file, need input cdp&offset spacing

        if(m_datatype == SeismicDataType::PreStack){
            SeismicData2DPreStack* sd2d = new SeismicData2DPreStack(fileName.toStdString(), this);
            m_loading_sd2d = sd2d;
            sd2d->setupDataIndex();
        }else{
            QErrorMessage err(this);
            err.showMessage("The index file seems not correct. Please remove it and reload the data");
            return;
        }

        ui->statusbar->showMessage("Find the index file");

        setCurrendDisplayDataPointer();
        cleanMainDisplayArea(true);
        setupMainWindowLayout();
        addFile2RecentOpenedFileList(fileName);

        this->showMaximized();
    } else{
        cout << " index file doesn't exist" << endl;
        ui->startupOpenFileLineEdit->setText(fileName);
        cleanMainDisplayArea();
        createLoadingDataObject(fileName);
        createLoadFileDataTypeDialog(ui->startupFileInfoFrame);
    }
}

bool SeismicDataProcessing2D::cannotFindTheFileSelectedFromList(QListWidgetItem *item)
{
    const QString fileName = item->text();
    QFile sgyfile(fileName);
    if(!sgyfile.exists()){
        QMessageBox message(QMessageBox::NoIcon,
                            "Warn", "Cannot find the selected file. Take away it from the list?", QMessageBox::Yes | QMessageBox::No, NULL);
        if(message.exec() == QMessageBox::Yes)
        {
            ui->startupRecentFIlesListWidget->takeItem(ui->startupRecentFIlesListWidget->row(item));
            saveRecentOpenedFileList();
        }
        return true;
    }    
    return false;
}

bool SeismicDataProcessing2D::isGatherDisplayFitToScrollWindow(void)
{
    return m_current_sd2d->getPlotFitZoom();
}

void SeismicDataProcessing2D::setTraceDataSwapbytes(QString item)
{
    cout << "Swapbytes = "<<item.toStdString().c_str() << endl;
    int swapbytes = 0;
    if(item.compare("IBM Swapbytes")   == 0)  swapbytes = 1;
    if(item.compare("IBM NoSwapbytes") == 0)  swapbytes = 2;
    if(item.compare("IEEE Swapbytes")   == 0) swapbytes = 3;
    if(item.compare("IEEE NoSwapbytes") == 0) swapbytes = 4;
    m_loading_sd2d->getSegyhandle()->setTraceDataSwapbytes(swapbytes);
    m_finfotabs->setTraceDataTableValues();

}

void SeismicDataProcessing2D::setCDPSpacing()
{    
    float value = m_cdpSpaceLineEdit->text().toFloat();
    float dcdp =  m_loading_sd2d->getCDPSpacing();

    if(qAbs(value - dcdp) < 0.0001 ) {
        if(value > 0.01) m_startupParaFlag = m_startupParaFlag | 1;
        return;
    }

    m_loading_sd2d->setCDPSpacing(value);
    m_startupParaFlag = m_startupParaFlag | 1;
    m_loading_sd2d->setRegenerateIndexFile(true);
}

void SeismicDataProcessing2D::setOffsetSpacing()
{
    float value = m_offSpaceLineEdit->text().toFloat();
    float doff =  m_loading_sd2d->getOffsetSpacing();

    if(qAbs(value - doff) < 0.0001 ) {
        if(value > 0.01) m_startupParaFlag = m_startupParaFlag | 2;
        return;
    }
    m_loading_sd2d->setOffsetSpacing(value);
    m_startupParaFlag = m_startupParaFlag | 2;
    m_loading_sd2d->setRegenerateIndexFile(true);
}

void SeismicDataProcessing2D::setMinOffset(const QString &text)
{
    //float value = m_minOffLineEdit->text().toFloat();
    float value = text.toFloat();
    float moff =  m_loading_sd2d->getMinOffset();

    if(qAbs(value - moff) < 0.0001 ) {
        if(value > 0.01) m_startupParaFlag = m_startupParaFlag | 4;
        return;
    }

    m_loading_sd2d->setMinOffset(value);
    m_startupParaFlag = m_startupParaFlag | 4;
    m_loading_sd2d->setRegenerateIndexFile(true);
}

void SeismicDataProcessing2D::setMaxOffset(const QString &text)
{
    //float value = m_maxOffLineEdit->text().toFloat();
    float value = text.toFloat();

    float moff =  m_loading_sd2d->getMaxOffset();

    if(qAbs(value - moff) < 0.0001 ) {
        if(value > 0.01) m_startupParaFlag = m_startupParaFlag | 8;
        return;
    }

    m_loading_sd2d->setMaxOffset(value);
    m_startupParaFlag = m_startupParaFlag | 8;
    m_loading_sd2d->setRegenerateIndexFile(true);
}

void SeismicDataProcessing2D::on_startupRecentFIlesListWidget_itemClicked(QListWidgetItem *item)
{    
    if(cannotFindTheFileSelectedFromList(item)){
        return;
    }
    const QString fileName = item->text();
    ui->startupOpenFileLineEdit->setText(fileName);
}

QStatusBar* SeismicDataProcessing2D::getStatusbarPointer()
{
    return ui->statusbar;
}

QVBoxLayout* SeismicDataProcessing2D::createLoadFileDataTypeDialog(QWidget* pParentWidget)
{
    //ui->statusbar->showMessage("Cannot find the index file, need to set values about CDP & offset");

    QLabel *topLabel = new QLabel(tr("File Information:"));

    m_finfotabs = new Sdp2dFileInformationTabs(m_loading_sd2d, 10);

    createStartupInfoLayout();

    QVBoxLayout *layout = new QVBoxLayout(pParentWidget);
    layout->addWidget(topLabel);
    layout->addSpacing(4);
    layout->addWidget(m_finfotabs);
    layout->setStretchFactor(m_finfotabs, 3);
    layout->addSpacing(20);
    layout->addLayout(m_fileInfoHLayout);
    return layout;
}

void SeismicDataProcessing2D::createStartupInfoLayout()
{
    m_fileInfoHLayout = new QHBoxLayout();

    QLabel *label5 = new QLabel(tr("Data type:"));

    m_datatypebox = new QComboBox;
    m_datatypebox->addItem("Pre-stack");
    m_datatypebox->addItem("Stack");
    m_datatypebox->addItem("Attribute");
    m_datatypebox->setCurrentIndex(0);

    QLabel *label6 = new QLabel(tr("Data swapbytes:"));
    m_tracesbcbox = new QComboBox;
    m_tracesbcbox->addItem("AutoDetection");
    m_tracesbcbox->addItem("IBM Swapbytes");
    m_tracesbcbox->addItem("IBM NoSwapbytes");
    m_tracesbcbox->addItem("IEEE Swapbytes");
    m_tracesbcbox->addItem("IEEE NoSwapbytes");
    m_tracesbcbox->setCurrentIndex(0);

    QLabel *label1 = new QLabel(tr("CDP spacing:"));
    QLabel *label2 = new QLabel(tr("Offset spacing:"));
    QLabel *label3 = new QLabel(tr("Minimum Offset:"));
    QLabel *label4 = new QLabel(tr("Maximum Offset:"));
    m_cdpSpaceLineEdit = new QLineEdit();
    m_offSpaceLineEdit = new QLineEdit();
    m_minOffLineEdit = new QLineEdit();
    m_maxOffLineEdit = new QLineEdit();

    m_cdpSpaceLineEdit->setValidator( new QDoubleValidator(this));
    m_offSpaceLineEdit->setValidator( new QDoubleValidator(this));
    m_minOffLineEdit->setValidator( new QDoubleValidator(this));
    m_maxOffLineEdit->setValidator( new QDoubleValidator(this));
    connect(m_cdpSpaceLineEdit, &QLineEdit::editingFinished, this, &SeismicDataProcessing2D::setCDPSpacing);
    connect(m_offSpaceLineEdit, &QLineEdit::editingFinished, this, &SeismicDataProcessing2D::setOffsetSpacing);
    connect(m_minOffLineEdit, &QLineEdit::textChanged, this, &SeismicDataProcessing2D::setMinOffset);
    connect(m_maxOffLineEdit, &QLineEdit::textChanged, this, &SeismicDataProcessing2D::setMaxOffset);
    connect(m_tracesbcbox, &QComboBox::currentTextChanged, this, &SeismicDataProcessing2D::setTraceDataSwapbytes);

    QGridLayout *grid1 = new QGridLayout;
    grid1->addWidget(label1, 0, 0);
    grid1->addWidget(label2, 1, 0);
    grid1->addWidget(m_cdpSpaceLineEdit, 0, 1);
    grid1->addWidget(m_offSpaceLineEdit, 1, 1);

    QGridLayout *grid2 = new QGridLayout;
    grid2->addWidget(label3, 0, 0);
    grid2->addWidget(label4, 1, 0);
    grid2->addWidget(m_minOffLineEdit, 0, 1);
    grid2->addWidget(m_maxOffLineEdit, 1, 1);

    QGridLayout *grid3 = new QGridLayout;
    grid3->addWidget(label5, 0, 0);
    grid3->addWidget(label6, 1, 0);
    grid3->addWidget(m_datatypebox, 0, 1);
    grid3->addWidget(m_tracesbcbox, 1, 1);

    m_fileInfoHLayout->addSpacing(100);
    m_fileInfoHLayout->addLayout(grid1);
    m_fileInfoHLayout->addSpacing(20);
    m_fileInfoHLayout->addLayout(grid2);
    m_fileInfoHLayout->addSpacing(20);
    m_fileInfoHLayout->addLayout(grid3);
    m_fileInfoHLayout->addSpacing(100);

    cout << "Analysis Data Flag: " << m_analysisDataFlag << endl;
    m_startupParaFlag = 0;
    double value = 0;
    if(!m_analysisDataFlag){
        value = m_loading_sd2d->getCDPSpacing();
        if(value > 0.5 ){
            m_cdpSpaceLineEdit->setText(QString::number(value, 'f', 2));
            m_startupParaFlag = m_startupParaFlag | 1;
        }

        value = m_loading_sd2d->getOffsetSpacing();
        if(value > 0.5 ){
            m_offSpaceLineEdit->setText(QString::number(value, 'f', 2));
            m_startupParaFlag = m_startupParaFlag | 2;
        }

        value = m_loading_sd2d->getMinOffset();
        //cout << "getMinOffset=" << value << endl;
        m_minOffLineEdit->setText(QString::number(value, 'f', 2));
        m_startupParaFlag = m_startupParaFlag | 4;


        value = m_loading_sd2d->getMaxOffset();
        if(value > 0.5 ){
            m_maxOffLineEdit->setText(QString::number(value, 'f', 2));
            m_startupParaFlag = m_startupParaFlag | 8;
        }

        int swapbytes = m_loading_sd2d->getSegyhandle()->getTraceDataSwapbytes();
        m_tracesbcbox->setCurrentIndex(swapbytes);
    }

    m_datatype = SeismicDataType::PreStack;
}

bool SeismicDataProcessing2D::isIndexFileExist(QString fileName)
{
    string sgy_file = fileName.toStdString();
    size_t i = sgy_file.rfind('.', sgy_file.length());
    size_t j = sgy_file.rfind('/', sgy_file.length());
    string m_idxfile = sgy_file.substr(0, j+1)+'.'+sgy_file.substr(j+1, i-j-1)+".idx";
    const QString idxFileName = QString::fromUtf8( m_idxfile.data(), m_idxfile.size() );
    QFile idxfile(idxFileName);
    if(idxfile.exists()){
        ifstream infile(m_idxfile, ifstream::binary);
        infile.read((char*)&m_datatype, sizeof(SeismicDataType));
        infile.close();
        m_analysisDataFlag = false;
        return true;
    }else{
        m_analysisDataFlag = true;
        return false;
    }
}

void SeismicDataProcessing2D::setCurrendDisplayDataPointer()
{
    setCurrentDataPointer(m_loading_sd2d);    
    m_loading_sd2d = nullptr;
    for(int i=0; i< m_lsd2d.size(); i++) {
        if(m_current_sd2d == m_lsd2d.at(i)) return;
    }
    m_lsd2d.append(m_current_sd2d);

    getTraceAmplitudeWithThread();
    generateIntermediateFileWithThread();
    //cout << "size of m_lsd2d=" << m_lsd2d.size() << endl;

}

void SeismicDataProcessing2D::insertFilenameToRecentFilesList(QString fileName)
{
    int nrecents = ui->startupRecentFIlesListWidget->count();
    int i;
    for(i=0; i<nrecents; i++){
        if(ui->startupRecentFIlesListWidget->item(i)->text().compare(fileName) == 0) {
            ui->startupRecentFIlesListWidget->takeItem(i);
        }
    }
    //cout << "nrecents=" << nrecents << " i="<< i << endl;
    ui->startupRecentFIlesListWidget->insertItem(0, fileName);

}

void SeismicDataProcessing2D::saveRecentOpenedFileList()
{
    int nfiles = ui->startupRecentFIlesListWidget->count();

    QString fileName = QDir::homePath() + QString("/.sdp2d/recent");
    QFile sd2dfile(fileName);    

    if(sd2dfile.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&sd2dfile);
        for(int i=0; i < nfiles; i++){
            QString fn = ui->startupRecentFIlesListWidget->item(i)->text();
            out << fn << "\n";            
        }
        sd2dfile.close();
    }else{
        std::cout <<"the recent file cannot open" << endl;
    }
}

void SeismicDataProcessing2D::setupMainWindowLayout(void)
{   
    // only create once
    createToolBar();

    if(m_datadock == nullptr){ //only one kept from the beginning to end
        m_datadock = new Sdp2dOpenedDataDockWidget(m_lsd2d, m_current_sd2d, this);        
        addDockWidget(Qt::LeftDockWidgetArea, m_datadock);
        m_displayParaTab = m_datadock->getDisplayParamTabPointer();
    }    else{
        m_datadock->appendNewFileToList(m_current_sd2d);
    }

    m_current_sd2d->setDisplayParamTabPointer(m_displayParaTab);

    if(m_displayView == nullptr){
        m_displayView = new QSplitter(Qt::Horizontal);
        setCentralWidget(m_displayView);
        m_displayView->setOpaqueResize(false);
    }

    if(m_current_sd2d->getDataType() == SeismicDataType::PreStack){
        ui->action_tools_StackVelocityAnalysis->setVisible(true);

        // every new dataset will generate a new map
        Sdp2dMapDiaplayDockWidget* mapdock = new Sdp2dMapDiaplayDockWidget(m_current_sd2d, this);
        addDockWidget(Qt::LeftDockWidgetArea, mapdock);

        // the central area. each SD2D keep its own.
        // Sdp2dMainGatherDisplayArea is for prestack gather display. The stack and attribute need implement new ones.
        Sdp2dMainGatherDisplayArea* newshot = new Sdp2dMainGatherDisplayArea(m_current_sd2d, this);
        m_displayView->addWidget(newshot);
        m_current_sd2d->updateDisplayParameters();
    } else if(m_current_sd2d->getDataType() == SeismicDataType::Stack){
        ui->action_tools_StackVelocityAnalysis->setVisible(false);
    } else if(m_current_sd2d->getDataType() == SeismicDataType::Attribure){
        ui->action_tools_StackVelocityAnalysis->setVisible(false);
    }

    action_display_zoomfit(true);

    // every new dataset will generate a new frequency analysis dock
    m_current_sd2d->createFrequencyAnaDock();

    if(m_pjobdock == nullptr){ // only one kept, but every time change dataset, change the values
        m_pjobdock = new Sdp2dProcessJobDockWidget(m_current_sd2d, this);
        m_pjobdock->setVisible(false);
        addDockWidget(Qt::RightDockWidgetArea, m_pjobdock);        
    }

}

void SeismicDataProcessing2D::createToolBar()
{
    if(m_fileToolBar == nullptr){
        m_fileToolBar = addToolBar(tr("File"));
        m_fileToolBar->addAction(ui->action_load_segy_file);
        m_fileToolBar->addAction(ui->action_saveas_segy_file);
        m_fileToolBar->addAction(ui->action_close_current_file);
        m_fileToolBar->addAction(ui->action_quit_all);
    }

    if(m_displayToolBar == nullptr){
        m_displayToolBar = addToolBar(tr("Display"));
        m_displayToolBar->addAction(ui->action_display_offsetelev);
        m_displayToolBar->addSeparator();
        m_displayToolBar->addAction(ui->action_display_wt_gather);
        m_displayToolBar->addAction(ui->action_display_vd_gather);
        m_displayToolBar->addSeparator();
        m_displayToolBar->addAction(ui->action_display_first_gather);
        m_displayToolBar->addAction(ui->action_display_previous_gather);
        m_displayToolBar->addAction(ui->action_display_next_gather);
        m_displayToolBar->addAction(ui->action_display_last_gather);
        m_displayToolBar->addSeparator();
        m_displayToolBar->addAction(ui->action_display_zoomin);
        m_displayToolBar->addAction(ui->action_display_zoomout);
        m_displayToolBar->addAction(ui->action_display_zoomfit);
    }

    if(m_toolsToolBar == nullptr){
        m_toolsToolBar = addToolBar(tr("Tools"));
        m_toolsToolBar->addAction(ui->action_tools_FrequencyAnalysis);
        m_toolsToolBar->addAction(ui->action_tools_BadTraceSelection);
        m_toolsToolBar->addAction(ui->action_tools_StackVelocityAnalysis);
        m_toolsToolBar->addAction(ui->action_tools_PickingMute);
        QActionGroup* toolsGroup = new QActionGroup(this);
        toolsGroup->addAction(ui->action_tools_FrequencyAnalysis);
        toolsGroup->addAction(ui->action_tools_BadTraceSelection);
        toolsGroup->addAction(ui->action_tools_StackVelocityAnalysis);
        toolsGroup->addAction(ui->action_tools_PickingMute);
        toolsGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
    }
}

void SeismicDataProcessing2D::cleanMainDisplayArea(bool all)
{
    if(ui->label_2 != nullptr){
        ui->label_2->setAttribute(Qt::WA_DeleteOnClose);
        ui->label_2->close();
        ui->label_2 = nullptr;
    }
    if(ui->startupRecentFIlesListWidget != nullptr){
        ui->startupRecentFIlesListWidget->setAttribute(Qt::WA_DeleteOnClose);
        ui->startupRecentFIlesListWidget->close();
        ui->startupRecentFIlesListWidget=nullptr;
    }

    if(!all) return;

    if(ui->startupFrame != nullptr){
        ui->startupFrame->setAttribute(Qt::WA_DeleteOnClose);
        ui->startupFrame->close();
        ui->startupFrame = nullptr;
    }

    if(ui->startupOKBtn != nullptr){
        ui->startupOKBtn->setAttribute(Qt::WA_DeleteOnClose);
        ui->startupOKBtn->close();
        ui->startupOKBtn = nullptr;
    }

    if(ui->sartupCancelBtn != nullptr){
        ui->sartupCancelBtn->close();
        ui->sartupCancelBtn->setAttribute(Qt::WA_DeleteOnClose);
        ui->sartupCancelBtn = nullptr;
    }

}

void SeismicDataProcessing2D::createLoadingDataObject(QString fileName)
{
    if(isIndexFileExist(fileName)){ // If index file exists
        cout << "index file exists" << endl;
        if(m_datatype == SeismicDataType::PreStack){
            SeismicData2DPreStack* sd2d = new SeismicData2DPreStack(fileName.toStdString(), this);
            if(m_loading_sd2d != nullptr) delete m_loading_sd2d;
            m_loading_sd2d = dynamic_cast<SeismicData2D*>(sd2d);            
        }else{
            QErrorMessage err(this);
            err.showMessage("The index file seems not correct. Please remove it and reload the data");
            return;
        }
        ui->statusbar->showMessage("Find the index file");
    }else{
        cout << "index file does not exists" << endl;
        if(m_loading_sd2d != nullptr) delete m_loading_sd2d;
        m_loading_sd2d = new SeismicData2D(fileName.toStdString(), this);
        m_loading_sd2d->createDataSummaryInfo();
    }
}

void SeismicDataProcessing2D::recreateDataObjectWithDataType(QString fileName)
{
    // for the time being. need to get from data type setting.
    if(m_datatype == SeismicDataType::UnKnow){
        m_datatype = SeismicDataType::PreStack;
    }
    if(m_datatype == SeismicDataType::PreStack && isIndexFileExist(fileName) == false){        
        Sdp2dSegy* oldsgy = m_loading_sd2d->getSegyhandle();

        SeismicData2DPreStack* sd2d = new SeismicData2DPreStack(oldsgy->getSEGYFileName(), this, false);
        Sdp2dSegy* newsgy = sd2d->getSegyhandle();
        //TraceHeaders** newtrh = newsgy->getAllTracesHeader();
        newsgy->setTraceDataSwapbytes(oldsgy->getTraceDataSwapbytes());

        int ntr = sd2d->getNumberOfTraces();
        for(int i=0; i< ntr; i++){
            TraceHeaders* tmp_thdr = new TraceHeaders;
            TraceHeaders* oldtrh = oldsgy->getTraceHeader(i);
            std::memcpy((void*)tmp_thdr, (void*)oldtrh, TRACE_HEADER_SIZE);
            newsgy->keepSEGYTraceHeader(i, tmp_thdr);            
        }
        sd2d->setCDPSpacing(m_loading_sd2d->getCDPSpacing());
        sd2d->setOffsetSpacing(m_loading_sd2d->getOffsetSpacing());
        sd2d->setMinOffset(m_loading_sd2d->getMinOffset());
        sd2d->setMaxOffset(m_loading_sd2d->getMaxOffset());

        delete m_loading_sd2d;
        m_loading_sd2d = sd2d;
    }else{
        cout << "reCreateDataObjectWithDataType do nothing" << endl;
    }
}

void SeismicDataProcessing2D::addFile2RecentOpenedFileList(QString fileName)
{    
    QList<QString> recFileList;
    QString recFileName = QDir::homePath() + QString("/.sdp2d/recent");
    //QFileInfo sd2dfinfo(fileName);
    //ui->statusbar->showMessage(sd2dfinfo.absoluteFilePath());

    QFile sd2dfile(recFileName);
    if(sd2dfile.exists()){
        if(sd2dfile.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream in(&sd2dfile);
            while (!in.atEnd()) {
                   QString qline = in.readLine();
                   if(qline.compare(fileName) == 0) continue;
                   recFileList.append(qline);
            }
        }else{
            std::cout <<"the recent file cannot open" << endl;
        }
        sd2dfile.close();
    }

    recFileList.insert(0,fileName);

    if(sd2dfile.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream out(&sd2dfile);
        for(int i=0; i < recFileList.size(); i++){
            out << recFileList[i] << "\n";
            if(i< m_maxRecentFiles){
                QString strippedName = QFileInfo(recFileList[i]).fileName();
                m_recentFileActionList.at(i)->setText(strippedName);
                m_recentFileActionList.at(i)->setData(recFileList[i] );
                m_recentFileActionList.at(i)->setVisible(true);
            }
        }
        sd2dfile.close();
    }
    if(recFileList.size() < m_maxRecentFiles){
        for(int i=recFileList.size(); i < m_maxRecentFiles; i++){
            m_recentFileActionList.at(i)->setVisible(false);
        }
    }

    disableAllActions(false);
}

void SeismicDataProcessing2D::loadFileOKBtnClicked(QString fileName)
{    
    if (m_startupParaFlag != 15) return;
    emit closeDataTypeInputDlg();
    recreateDataObjectWithDataType(fileName);
    SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack *>(m_loading_sd2d);
    sd2d->setupDataIndex();
    setCurrendDisplayDataPointer();
    setupMainWindowLayout();
    addFile2RecentOpenedFileList(fileName);
}

void SeismicDataProcessing2D::loadFileCancelBtnClicked(void)
{    
    if(m_loading_sd2d != nullptr){
        delete m_loading_sd2d;
        m_loading_sd2d = nullptr;
    }
}

bool SeismicDataProcessing2D::isTheFileAlreadyOpened(QString fileName)
{
    if(m_datadock == nullptr) return false;  //no file opened yet
    if(m_lsd2d.size() == 0) return false;

    string sname = m_current_sd2d->getSEGYFileName();
    const QString qname = QString::fromUtf8( sname.data(), sname.size() );
    if(qname.compare(fileName) == 0) return true;

    for (int i = 0; i < m_lsd2d.size(); ++i) {
        string sname = m_lsd2d.at(i)->getSEGYFileName();

        const QString qname = QString::fromUtf8( sname.data(), sname.size() );
        if(qname.compare(fileName) == 0) {        
            setCurrentDataPointer(m_lsd2d.at(i));
            m_datadock->setFileAsCurrentItem(fileName);
            return true;
        }
    }
    return false;
}

void SeismicDataProcessing2D::setCurrentDataPointer(SeismicData2D* sd2d)
{
    if(m_current_sd2d != nullptr) {
        hideDisplaysOfSD2D(m_current_sd2d);
    }

    m_current_sd2d = sd2d;    
    int interactiveFunction = m_current_sd2d->getInteractiveFunction();
    //cout << " interactiveFunction = " << interactiveFunction << endl;

    if(m_current_sd2d->getDataType() == SeismicDataType::PreStack){
        ui->action_tools_StackVelocityAnalysis->setVisible(true);
    }else {
        ui->action_tools_StackVelocityAnalysis->setVisible(false);
    }

    showDisplaysOfSD2D(m_current_sd2d);

    ui->action_display_zoomfit->setChecked(m_current_sd2d->getPlotFitZoom());
    if(m_current_sd2d->getDisplayType() == SeismicDisplayTpye::Color){
        ui->action_display_vd_gather->setChecked(true);        
        ui->action_display_wt_gather->setChecked(false);
    }else if(m_current_sd2d->getDisplayType() == SeismicDisplayTpye::Wiggle){
        ui->action_display_vd_gather->setChecked(false);
        ui->action_display_wt_gather->setChecked(true);
    } else {
        ui->action_display_vd_gather->setChecked(true);
        ui->action_display_wt_gather->setChecked(true);
    }

    switch (interactiveFunction) {
    case InteractiveFunctions::FrequencyAnalysis:
        ui->action_tools_FrequencyAnalysis->setChecked(true);
        break;
    case InteractiveFunctions::BadTraceSelection:
        ui->action_tools_BadTraceSelection->setChecked(true);
        break;
    case InteractiveFunctions::StackVelAnalysis:
        m_current_sd2d->getFreqAnalysisPointer()->hide();
        ui->action_tools_StackVelocityAnalysis->setChecked(true);
        m_current_sd2d->setInteractiveFunction(InteractiveFunctions::StackVelAnalysis);
        break;
    case InteractiveFunctions::PickingMute:
        ui->action_tools_PickingMute->setChecked(true);
        break;
    case InteractiveFunctions::MeasureLinearVelcity:
    case InteractiveFunctions::None:
        uncheckAllToolActions();
        break;
    }
    if(m_current_sd2d->getTraceHeaderDisplay()){
        ui->action_display_offsetelev->setChecked(true);
    } else {
        ui->action_display_offsetelev->setChecked(false);
    }

    if(m_pjobdock != nullptr){
        m_pjobdock->setCurrentDataPointer(m_current_sd2d);
    }

}

void SeismicDataProcessing2D::uncheckAllToolActions(void)
{
    ui->action_tools_PickingMute->setChecked(false);
    ui->action_tools_StackVelocityAnalysis->setChecked(false);
    ui->action_tools_BadTraceSelection->setChecked(false);
    ui->action_tools_FrequencyAnalysis->setChecked(false);
}

SeismicData2D* SeismicDataProcessing2D::getCurrentDataPointer(void)
{
    return m_current_sd2d;
}

void SeismicDataProcessing2D::hideDisplaysOfSD2D(SeismicData2D* sd2d)
{
    //cout << " hide current displays" << endl;
    sd2d->hideAllDisplayWidgets();
}

void SeismicDataProcessing2D::showDisplaysOfSD2D(SeismicData2D* sd2d)
{
    if(sd2d == nullptr) return;
    sd2d->showAllDisplayWidgets();
    sd2d->updateDisplayParameters();
}

void SeismicDataProcessing2D::loadSEGYDataUseFileName(QString fileName, bool checkSetting)
{
    if(!QFile::exists(fileName)) {
        ui->statusbar->showMessage(QString("The selected file doesn't exist: ")+fileName);
        return;
    }else{
        ui->statusbar->showMessage(QString("Opening the file: ")+fileName);
    }

    if(isTheFileAlreadyOpened(fileName)) return;

    createLoadingDataObject(fileName);

    if(isIndexFileExist(fileName) && !checkSetting){
        if(m_datatype == SeismicDataType::PreStack){
            SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack*>(m_loading_sd2d) ;
            sd2d->setupDataIndex();
        } else {
            //TODO: This part for other seismic data types.
        }
        setCurrendDisplayDataPointer();
        cleanMainDisplayArea(true);
        setupMainWindowLayout();
        addFile2RecentOpenedFileList(fileName);
    }else{
        if(checkSetting) m_analysisDataFlag = false;
        if(ui->startupRecentFIlesListWidget != nullptr ){
            ui->startupOpenFileLineEdit->setText(fileName);
            cleanMainDisplayArea();
            createLoadFileDataTypeDialog(ui->startupFileInfoFrame);
        }else {
            QDialog* m_fileTypeSetDlg = new QDialog(this);
            m_fileTypeSetDlg->setAttribute(Qt::WA_DeleteOnClose);

            QDialogButtonBox* buttonBox= new QDialogButtonBox(m_fileTypeSetDlg);
            buttonBox->setOrientation(Qt::Horizontal);
            buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
            //QVBoxLayout* vlBox = createLoadFileDataTypeDialog(m_fileTypeSetDlg);
            QVBoxLayout* vlBox = createLoadFileDataTypeDialog(m_fileTypeSetDlg);
            vlBox->addWidget(buttonBox);

            connect(buttonBox, &QDialogButtonBox::rejected, m_fileTypeSetDlg, &QDialog::close);
            connect(this, &SeismicDataProcessing2D::closeDataTypeInputDlg, m_fileTypeSetDlg, &QDialog::close);
            connect(buttonBox, &QDialogButtonBox::accepted, this, [ = ] {loadFileOKBtnClicked(fileName);});
            connect(buttonBox, &QDialogButtonBox::rejected, this, [ = ] {loadFileCancelBtnClicked();});

            m_fileTypeSetDlg->show();
            m_fileTypeSetDlg->resize(1200, 600);
        }
    }
}

void SeismicDataProcessing2D::openRecent(void){
    QAction *action = qobject_cast<QAction *>(sender());
    cout << "Load SEGY file: " << action->data().toString().toStdString().c_str() << endl;
    if (action) loadSEGYDataUseFileName(action->data().toString());
}

void SeismicDataProcessing2D::setToolBarDisplayType(int val)
{
    if(val == SeismicDisplayTpye::Color){
        ui->action_display_vd_gather->setChecked(true);
        ui->action_display_wt_gather->setChecked(false);
    }else if(val == SeismicDisplayTpye::Wiggle){
        ui->action_display_vd_gather->setChecked(false);
        ui->action_display_wt_gather->setChecked(true);
    } else {
        ui->action_display_vd_gather->setChecked(true);
        ui->action_display_wt_gather->setChecked(true);
    }
}

void SeismicDataProcessing2D::setVisibleOfJobDock(bool visible)
{
    if(m_current_sd2d == nullptr) return;
    ui->action_processing_showdock->setChecked(visible);
    m_current_sd2d->setJobdockVisible(visible);
}

void SeismicDataProcessing2D::generateIntermediateFileWithThread()
{
    Sdp2dSegy* segy = m_current_sd2d->getSegyhandle();
    while(m_workerThread->isFinished()){
        if(m_tempsegy != nullptr) delete m_tempsegy;
        m_tempsegy = nullptr;
    }

    m_tempsegy = new Sdp2dSegy(segy, ui->statusbar);

    m_tempsegy->moveToThread(m_workerThread);
    connect(m_workerThread, SIGNAL(finished()), m_tempsegy, SLOT(deleteLater()));
    //connect(m_workerThread, &QThread::finished, m_tempsegy, &QObject::deleteLater);
    connect(this, &SeismicDataProcessing2D::generateIntermediateFile, m_tempsegy, &Sdp2dSegy::generateIntermediateFile);
    m_workerThread->start();
    emit generateIntermediateFile(segy);
}

void SeismicDataProcessing2D::getTraceAmplitudeWithThread()
{
    Sdp2dSegy* segy = m_current_sd2d->getSegyhandle();
    if(m_workerThread->isFinished()){
        if(m_tempsegy != nullptr) delete m_tempsegy;
    }
    m_tempsegy = new Sdp2dSegy(segy, ui->statusbar);

    m_tempsegy->moveToThread(m_workerThread);
    connect(m_workerThread, SIGNAL(finished()), m_tempsegy, SLOT(deleteLater()));
    connect(this, &SeismicDataProcessing2D::getTraceAmplitude, m_tempsegy, &Sdp2dSegy::calculateAmplitudePutToHeader);

    m_workerThread->start();
    emit getTraceAmplitude(segy);

}

void SeismicDataProcessing2D::disableGatherProcess()
{
    m_enableProcess = false;    
    m_current_sd2d->removeAdjunctiveDisplays();
}

void SeismicDataProcessing2D::processCurrentGather(Sdp2dQDomDocument* paradom)
{    
    QString moduleName = paradom->getModuleName();
    //cout << "in processCurrentGather1 moduleName=" << moduleName.toStdString().c_str() << endl;
    if(!m_pmodules.contains(moduleName)) return;

    if(!m_pmodules.value(moduleName)->setupParameters(paradom)) return;

    //cout << "in processCurrentGather 1" << endl;

    m_current_sd2d->removeAdjunctiveDisplays();

    Sdp2dProcessedGatherDisplayArea* odp = new Sdp2dProcessedGatherDisplayArea(m_current_sd2d, this);
    m_displayView->addWidget(odp);
    odp->setVisible(true);

    Sdp2dMainGatherDisplayArea* idp = m_current_sd2d->getInputGatherDisplayPointer();
    int ntr = idp->getNumTracesOfCurrentGather();
    int ns = m_current_sd2d->getSamplesPerTraces();

    float** oudata = Sdp2dUtils::alloc2float(ns, ntr);
    float** indata = Sdp2dUtils::alloc2float(ns, ntr);
    idp->getDataOfInputSeismicGather(indata);

    m_pmodules.value(moduleName)->processCurrentGather(indata, oudata, ntr);
    odp->displayOneGather(oudata);

    m_enableProcess = true;

    Sdp2dUtils::free2float(indata);
    Sdp2dUtils::free2float(oudata);
}

void SeismicDataProcessing2D::processCurrentGather(QString moduleName)
{
    //cout << "in processCurrentGather2 moduleName=" << moduleName.toStdString().c_str() << endl;
    if(!m_enableProcess) return;
    if(!m_pmodules.contains(moduleName)) return;
    //cout << "in processCurrentGather 2" << endl;

    Sdp2dMainGatherDisplayArea* idp = m_current_sd2d->getInputGatherDisplayPointer();
    if(idp == nullptr) return;
    Sdp2dProcessedGatherDisplayArea* odp = m_current_sd2d->getProcessedGatherDisplayPointer();
    if(odp == nullptr) return;

    int ntr = idp->getNumTracesOfCurrentGather();
    int ns = m_current_sd2d->getSamplesPerTraces();

    float** oudata = Sdp2dUtils::alloc2float(ns, ntr);
    float** indata = Sdp2dUtils::alloc2float(ns, ntr);

    idp->getDataOfInputSeismicGather(indata);    
    m_pmodules.value(moduleName)->processCurrentGather(indata, oudata, ntr);
    odp->displayOneGather(oudata);

    Sdp2dUtils::free2float(indata);
    Sdp2dUtils::free2float(oudata);
}

void SeismicDataProcessing2D::processCurrentGather(QString moduleName, float** indata, float** oudata, int ntr)
{
    //cout << "in processCurrentGather3 moduleName=" << moduleName.toStdString().c_str() << endl;

    if(!m_pmodules.contains(moduleName)) return;
    //cout << "in processCurrentGather 3" << endl;

    m_pmodules.value(moduleName)->processCurrentGather(indata, oudata, ntr);
}

void SeismicDataProcessing2D::processWholeData(Sdp2dQDomDocument* paradom)
{
    QString moduleName = paradom->getModuleName();
    cout << "in processWholeData moduleName=" << moduleName.toStdString().c_str() << endl;
    if(!m_pmodules.contains(moduleName)) return;
    if(!m_pmodules.value(moduleName)->setupParameters(paradom)) return;
    cout << "in processWholeData" << endl;

    if(m_current_sd2d->getDataType() == SeismicDataType::PreStack){
        SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack*>(m_current_sd2d);
        sd2d->processAndOutputSegyWithLocalFormat(moduleName);
    }
}

void SeismicDataProcessing2D::setupParameters(Sdp2dQDomDocument* paradom)
{
    QString moduleName = paradom->getModuleName();
    if(!m_pmodules.contains(moduleName)) return;
    m_pmodules.value(moduleName)->setupParameters(paradom);
}

void SeismicDataProcessing2D::addWidgetToTheCentralSplitView(QWidget* w)
{
    m_displayView->addWidget(w);
    w->setVisible(true);
}

int SeismicDataProcessing2D::getInteractiveFunction(void)
{
    return m_current_sd2d->getInteractiveFunction();
}

void SeismicDataProcessing2D::setInteractiveFunction(InteractiveFunctions val)
{
    m_current_sd2d->setInteractiveFunction(val);
}

