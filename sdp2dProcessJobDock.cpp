#include "sdp2dProcessJobDock.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "seismicdataprocessing2d.h"
#include "sdp2dMainGatherDisplayArea.h"
#include "sdp2dProcessJobTreeWidget.h"
#include "sdp2dQDomDocument.h"
#include "sdp2dFrequencyAnalysisDock.h"
#include "sdp2dPreStackMutePicks.h"
#include "sdp2dVelSemblanceDisplayArea.h"
#include "sdp2dUtils.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QDir>
#include <QTextStream>
#include <QHeaderView>
#include <QTextEdit>
#include <QMessageBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QTreeWidgetItem>

Sdp2dFilenameLineEdit::Sdp2dFilenameLineEdit(QTreeWidgetItem* item, QWidget *parent) : QLineEdit(parent)
{
    m_item = item;
    setAttribute(Qt::WA_MacShowFocusRect);
    setBackgroundRole(QPalette::NoRole);
    setPlaceholderText(QDir::homePath() + QString("/Test.segy"));
}

void Sdp2dFilenameLineEdit::mouseDoubleClickEvent(QMouseEvent *e)
{
    cout << "edit line double clicked" << endl;
    const QString fileName = QFileDialog::getSaveFileName(this, "Save file", QDir::currentPath(), "SEGY (*.segy *.sgy *.SEGY *.SGY)");
    if(fileName.count() >0){
        this->setText(fileName);
        m_item->setText(3, fileName);
    }
    setFocus();
    QLineEdit::mouseDoubleClickEvent(e);
}

Sdp2dProcessJobDockWidget::Sdp2dProcessJobDockWidget(SeismicData2D* sd2d,  SeismicDataProcessing2D *parent) :
    QDockWidget(parent)
{
    m_mainWindow = parent;
    m_sd2d = sd2d;

    setWindowTitle("Processing Parameter Setup");
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    resize(400, 300);

    m_moduleList = nullptr;
    m_runjobbtn = nullptr;
    m_validatebtn = nullptr;
    m_savebtn = nullptr;
    m_loadbtn = nullptr;
    m_paraTree = nullptr;
    m_outputitem = nullptr;

    m_saveFunctionToSD2D = true;
    m_processWholeData = false;
    m_interactFunction = 0;

    m_domdoc = new Sdp2dQDomDocument("sdp2dProcessingModule");
    m_domval = new Sdp2dQDomDocument("sdp2dProcessingModule");

    setuplayout();
}

Sdp2dProcessJobDockWidget::~Sdp2dProcessJobDockWidget()
{
    delete m_domdoc;
    delete m_domval;
}

void Sdp2dProcessJobDockWidget::hideEvent(QHideEvent *event)
{
    m_mainWindow->setVisibleOfJobDock(false);
    QDockWidget::hideEvent(event);
}

void Sdp2dProcessJobDockWidget::showEvent(QShowEvent *event)
{
    m_mainWindow->setVisibleOfJobDock(true);
    QDockWidget::showEvent(event);
}

void Sdp2dProcessJobDockWidget::setuplayout(void)
{
    QWidget *dockWidgetContents = new QWidget();
    setWidget(dockWidgetContents);

    QVBoxLayout* vlayout = new QVBoxLayout(dockWidgetContents);
    vlayout->setContentsMargins(5, 5, 5, 5);

    QHBoxLayout* hlayout1 = new QHBoxLayout();
    QLabel* label = new QLabel("Select a processing module:", this);
    m_moduleList = new QComboBox(this);
    setAvailableModules();

    hlayout1->addWidget(label);
    hlayout1->addWidget(m_moduleList);

    QHBoxLayout* hlayout2 = new QHBoxLayout();
    m_apply2all = new QCheckBox("Apply to whole data", this);
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/images/runJob.png"), QSize(), QIcon::Normal, QIcon::Off);
    m_validatebtn = new QPushButton(style()->standardIcon((enum QStyle::StandardPixmap)45), "Check", this);

    m_runjobbtn = new QPushButton(icon, "Run", this);
    m_runjobbtn->setEnabled(false);

    hlayout2->addWidget(m_apply2all);
    hlayout2->addWidget(m_validatebtn);
    hlayout2->addWidget(m_runjobbtn);

    vlayout->addLayout(hlayout1);
    vlayout->addLayout(hlayout2);

    m_paraTree = new Sdp2dProcessJobTreeWidget(m_domdoc, m_domval);

    QSplitter* splitter = new QSplitter(Qt::Vertical);
    m_infoView = new QTextEdit(this);
    m_infoView->setReadOnly(true);

    splitter->addWidget(m_paraTree);
    splitter->addWidget(m_infoView);
    vlayout->addWidget(splitter);

    connect(m_apply2all,SIGNAL(stateChanged(int)), this, SLOT(setProcessWholeData(int)));
    connect(m_runjobbtn, SIGNAL(clicked()), this, SLOT(runJob()));
    connect(m_validatebtn, SIGNAL(clicked(bool)), this, SLOT(checkRequiredParameters(bool)));
    connect(m_moduleList,SIGNAL(currentTextChanged(const QString)), this, SLOT(processModuleChanged(const QString)));
    connect(m_paraTree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(parameterItemClicked(QTreeWidgetItem*, int)));
}

void Sdp2dProcessJobDockWidget::createOutputTreeitem(void)
{
    if(m_outputitem != nullptr) delete  m_outputitem;

    m_outputitem = new QTreeWidgetItem();
    m_outputitem->setFirstColumnSpanned(true);
    m_outputitem->setText(0, QString("Output to SEGY file"));
    QString helpInfo = "Output seismic data/header to SEGY file. If filename is not provided, picked bad trace and mute time will be stored to the trace header of the input segy file. ";
    helpInfo =  helpInfo + QString("The output file name is need when processing the whole data.");
    m_outputitem->setToolTip(0, helpInfo);

    QTreeWidgetItem* item = new QTreeWidgetItem(m_outputitem);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(0, QString("FileName"));
    item->setText(1, QString("String"));
    item->setToolTip(0, QString("Filename with full path of the output SEGY file"));

    m_ofname = new Sdp2dFilenameLineEdit(item);
    m_ofname->setFrame(false);
    m_ofname->setAutoFillBackground(true);
    m_ofname->setAttribute(Qt::WA_OpaquePaintEvent);
    m_paraTree->setItemWidget(item, 3, m_ofname);
    QString fname = m_sd2d->getOutputSegyFileName();
    cout << "Init Output file name = " << m_sd2d->getOutputSegyFileName().toStdString().c_str() << endl;
    if(fname.count() > 1) m_ofname->setText(fname);
    item->setText(3, fname);
    connect(m_ofname, SIGNAL(editingFinished()), this, SLOT(outputSEGYFilenameChanged()));

    QCheckBox* cbox = new QCheckBox();    
    cbox->setEnabled(false);
    m_paraTree->setItemWidget(item, 2, cbox);
    cbox->setCheckState(Qt::Checked);

    item = new QTreeWidgetItem(m_outputitem);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(0, QString("Output order"));
    item->setText(1, QString("List"));
    item->setToolTip(0, QString("The output traces will be sorted to the specified order. "));
    item->setToolTip(1, QString("CommonShot, CommonCDP, CommonOffset, CommonReceiver"));
    switch(m_sd2d->getOutputOrder()){
    case DisplayGatherType::CommonShot:
        item->setText(3, QString("CommonShot"));
        break;
    case DisplayGatherType::CommonDepthPoint:
        item->setText(3, QString("CommonCDP"));
        break;
    case DisplayGatherType::CommonOffset:
        item->setText(3, QString("CommonOffset"));
        break;
    case DisplayGatherType::CommonReceiver:
        item->setText(3, QString("CommonReceiver"));
        break;
    }

    cbox = new QCheckBox();
    cbox->setCheckState(Qt::Checked);
    cbox->setEnabled(false);
    m_paraTree->setItemWidget(item, 2, cbox);


    item = new QTreeWidgetItem(m_outputitem);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(0, QString("DiscardBadTraces"));
    item->setText(1, QString("Bool"));
    if(m_sd2d->isOutputDiscardBadTraces()){
        item->setText(3, QString("True"));
    } else {
        item->setText(3, QString("False"));
    }
    item->setToolTip(0, QString("Discard all traces flaged as bad trace"));

    cbox = new QCheckBox();
    cbox->setCheckState(Qt::Checked);
    cbox->setEnabled(false);
    m_paraTree->setItemWidget(item, 2, cbox);

    item = new QTreeWidgetItem(m_outputitem);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(0, QString("ApplyTopMute"));
    item->setText(1, QString("Bool"));
    if(m_sd2d->isOutputApplyTopMute()){
        item->setText(3, QString("True"));
    } else {
        item->setText(3, QString("False"));
    }
    item->setToolTip(0, QString("Apply top mute on the output data."));

    cbox = new QCheckBox();
    cbox->setCheckState(Qt::Checked);
    cbox->setEnabled(false);
    m_paraTree->setItemWidget(item, 2, cbox);

    item = new QTreeWidgetItem(m_outputitem);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(0, QString("ApplyBottomMute"));
    item->setText(1, QString("Bool"));
    if(m_sd2d->isOutputApplyBtmMute()){
        item->setText(3, QString("True"));
    } else {
        item->setText(3, QString("False"));
    }
    item->setToolTip(0, QString("Apply bottommute on the output data."));

    cbox = new QCheckBox();
    cbox->setCheckState(Qt::Checked);
    cbox->setEnabled(false);
    m_paraTree->setItemWidget(item, 2, cbox);

}

void Sdp2dProcessJobDockWidget::setCurrentDataPointer(SeismicData2D* sd2d)
{
    m_sd2d = sd2d;
    setVisible(m_sd2d->isJobdockVisible());
    int function = m_sd2d->getInteractiveFunction();

    cout << "setCurrent item: " << m_sd2d->getSEGYFileName().c_str() << " function=" << function << endl;

    if(function > 0)  {
        interactiveFunctionEnabled(function);
    } else {
        interactiveFunctionDisAbled(function);
    }
}

void Sdp2dProcessJobDockWidget::showOutputFileView()
{
    cleanTreeView();
    m_apply2all->setCheckState(Qt::Checked);
    show();
    raise();
}

void Sdp2dProcessJobDockWidget::runJob()
{
    checkRequiredParameters(true);
    SeismicData2DPreStack* sp = dynamic_cast<SeismicData2DPreStack *>(m_sd2d);
    m_paraTree->setParameterValuesToDom();

    QString moduleName = m_domval->getModuleName();
    cout << "m_processWholeData=" << m_processWholeData << " m_interactFunction="<<m_interactFunction << " moduleName=" << moduleName.toStdString().c_str() << endl;

    if(m_interactFunction > 0) {
        if(m_interactFunction == InteractiveFunctions::FrequencyAnalysis) {            
            gathers* gp = sp->getGatherIndexPointer();
            sp->bandPassFilterSelectedDataAndDisplay(m_domval, gp);
            sp->getFreqAnalysisPointer()->setParametersToDom(m_domdoc);
            saveDomDocument("iFilter");
        } else if(m_interactFunction == InteractiveFunctions::PickingMute) {            
            sp->updateMuteParameters(m_domval);
            sp->setMuteValuesOfCurrentGather(true);
            sp->getCurrentMutePointer()->setParametersToDom(m_domdoc);
            saveDomDocument("iMute");
        } else if(m_interactFunction == InteractiveFunctions::StackVelAnalysis) {
            if(m_processWholeData)  setOutputParametersToSD2D();
            sp->setupStackVelocityAnalysis(m_domval, m_processWholeData);
            sp->getVelSemblanceDisplay()->setParametersToDom(m_domdoc);
            saveDomDocument("iVelAna");
        } else {

        }
    } else {
        if(m_processWholeData){
            setOutputParametersToSD2D();
            m_mainWindow->processWholeData(m_domval);
        } else {
            if(m_outputitem == nullptr || m_outputitem->isHidden()) {
                m_mainWindow->processCurrentGather(m_domval);
            } else {
                sp->outputSegyWithLocalFormat();
            }
        }
    }
    //cout << "RunJob done" << endl;
}

void Sdp2dProcessJobDockWidget::setAvailableModules()
{
    m_moduleList->addItem("None");

    QStringList modulesList;

    QString fileName = QDir::homePath() + QString("/.sdp2d/modules");
    QFile moduleFile(fileName);
    if(moduleFile.exists()){
        if(moduleFile.open(QIODevice::ReadOnly | QIODevice::Text)){
            QTextStream modules(&moduleFile);            
            while (!modules.atEnd()) {
                  modulesList <<  modules.readLine().trimmed();
            }
        }else{
            std::cout <<"the modules file cannot open!" << endl;
        }
    }
    moduleFile.close();

    modulesList.sort();

    for(int idx = 0; idx < modulesList.count(); idx++ ){
        QString moduleName = modulesList.at(idx);
        Sdp2dQDomDocument* domdoc = new Sdp2dQDomDocument("sdp2dProcessingModule");
        QString xmlfile = QDir::homePath() + QString("/.sdp2d/") + moduleName + QString(".xml");
        QFile file(xmlfile);
        file.open(QFile::ReadOnly);
        domdoc->setContent(&file, true);
        file.close();
        QDomElement root = domdoc->documentElement();
        m_moduleList->addItem(moduleName);
        m_moduleList->setItemData(idx+1, root.attribute("Title"), Qt::ToolTipRole );
        m_moduleList->setItemData(idx+1, root.attribute("Help"), Qt::WhatsThisRole);

        delete domdoc;
    }
}

void Sdp2dProcessJobDockWidget::interactiveFunctionEnabled(int function)
{

    m_saveFunctionToSD2D = false;

    m_interactFunction = function;
    m_moduleList->setCurrentIndex(0);
    m_moduleList->setDisabled(true);

    cleanTreeView();
    if(function == InteractiveFunctions::PickingMute) {
        loadDomDocument("iMute");
        m_sd2d->getCurrentMutePointer()->setParametersToDom(m_domdoc);
        m_paraTree->processModuleChanged();
    }else if(function == InteractiveFunctions::FrequencyAnalysis) {
        loadDomDocument("iFilter");
        m_sd2d->getFreqAnalysisPointer()->setParametersToDom(m_domdoc);
        m_paraTree->processModuleChanged();
    }else if(function == InteractiveFunctions::StackVelAnalysis) {
        loadDomDocument("iVelAna");
        SeismicData2DPreStack* sp = dynamic_cast<SeismicData2DPreStack *>(m_sd2d);
        Sdp2dVelSemblanceDisplayArea* vsda = sp->getVelSemblanceDisplay();
        if(vsda != nullptr) vsda->setParametersToDom(m_domdoc);
        m_paraTree->processModuleChanged();
    }

    m_saveFunctionToSD2D = true;

    if(m_interactFunction != InteractiveFunctions::StackVelAnalysis){
        m_runjobbtn->setEnabled(true);
        m_runjobbtn->setText("Apply");
        m_validatebtn->setDisabled(true);
        m_apply2all->setDisabled(true);
    }
}

void Sdp2dProcessJobDockWidget::interactiveFunctionDisAbled(int function)
{    
    Q_UNUSED(function);

    m_saveFunctionToSD2D = false;

    m_interactFunction = 0;
    m_moduleList->setDisabled(false);
    m_validatebtn->setDisabled(false);
    m_apply2all->setDisabled(false);    
    m_runjobbtn->setText("Run");

    m_moduleList->setCurrentIndex(0);

    QString moduleName = m_sd2d->getProcessingFunction();    

    if(moduleName.compare("None") == 0){
        cleanTreeView();
    } else {
        processModuleChanged(moduleName);
    }

    if(m_sd2d->getProcessWholeData()){
        m_apply2all->setCheckState(Qt::Checked);
    } else {
        m_apply2all->setCheckState(Qt::Unchecked);
    }

    m_saveFunctionToSD2D = true;
}


void Sdp2dProcessJobDockWidget::processModuleChanged(const QString moduleName)
{
    cleanTreeView();
    for(int i=0; i< m_moduleList->count(); i++){
        if(moduleName.compare(m_moduleList->itemText(i)) == 0) {
            if(m_saveFunctionToSD2D) m_sd2d->setProcessingFunction(moduleName);
            //cout << "processModuleChanged: m_saveFunctionToSD2D = " << m_saveFunctionToSD2D << " modulename=" << moduleName.toStdString().c_str() << endl;
            m_moduleList->setCurrentIndex(i);
            break;
        }
    }
    if(moduleName.compare("None") == 0) return;
    loadDomDocument(moduleName);
    m_paraTree->processModuleChanged();    
}

void Sdp2dProcessJobDockWidget::loadDomDocument(const QString moduleName)
{
    QString xmlfile = QDir::homePath() + QString("/.sdp2d/") + moduleName + QString(".xml");
    QFile file(xmlfile);
    file.open(QFile::ReadOnly);
    m_domdoc->setContent(&file, true);
    file.close();
}

void Sdp2dProcessJobDockWidget::saveDomDocument(const QString moduleName)
{
    if(m_domdoc->getModuleName().compare(moduleName) != 0) {
        cout << "Module name in Dom file  is: " << m_domdoc->getModuleName().toStdString().c_str() << endl;
        cout << "Module name of job table is: " << moduleName.toStdString().c_str() << endl;
        cout << "Please double check!! Something in code is WRONG!!!" << endl;
    }
    QString xmlfile = QDir::homePath() + QString("/.sdp2d/") + moduleName + QString(".xml");
    QFile file(xmlfile);
    const int IndentSize = 4;
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream out(&file);
    m_domdoc->save(out, IndentSize);
    file.close();
}

void Sdp2dProcessJobDockWidget::parameterItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column); 
    QString info = item->data(0, Qt::ToolTipRole).toString();
    m_infoView->setPlainText(info);
}

void Sdp2dProcessJobDockWidget::setProcessWholeData(int checked)
{
    cout << "setProcessWholeData checked="<< checked << endl;
    m_processWholeData = checked;
    //cout << "setProcessWholeData: m_saveFunctionToSD2D = " << m_saveFunctionToSD2D << " checked="<< checked << endl;
    if(m_saveFunctionToSD2D)  m_sd2d->setProcessWholeData(checked);

    int nitem = m_paraTree->topLevelItemCount();
    int index = m_paraTree->indexOfTopLevelItem(m_outputitem);

    //cout <<"nitem=" << nitem << " index=" << index << endl;
    bool newCreated = false;
    if(checked) {
        if(m_outputitem == nullptr) {
            createOutputTreeitem();
            newCreated = true;
        }

        if(nitem == 0){
            m_paraTree->addTopLevelItem(m_outputitem);
        } else if(nitem == 1 && index==-1) {
            m_paraTree->insertTopLevelItem(1, m_outputitem);
        } else {
            m_outputitem->setHidden(false);
        }
        m_outputitem->setExpanded(true);
        if(newCreated) m_runjobbtn->setEnabled(false);
        if(m_interactFunction == InteractiveFunctions::StackVelAnalysis){
            m_outputitem->child(1)->setText(3, QString("CommonCDP"));
        }
    } else {
        if(m_outputitem != nullptr) {
            m_outputitem->setHidden(true);
            if(nitem ==  1) m_runjobbtn->setEnabled(false);
        }
    }
}

void Sdp2dProcessJobDockWidget::checkRequiredParameters(bool quite)
{
    int count = m_paraTree->topLevelItemCount();
    if(count <= 0 ) {
        m_runjobbtn->setEnabled(false);
        return;
    }
    if(count==1 && m_outputitem){
        if(m_outputitem->isHidden()) {
            m_runjobbtn->setEnabled(false);
            return;
        }
    }

    if(count == 2 && !m_processWholeData) count = 1;

    bool allValidate = true;
    for(int i=0; i< count; i++){
        QTreeWidgetItem* topItem = m_paraTree->topLevelItem(i);
        bool itemValidate = checkItemStates(topItem, quite);
        allValidate = allValidate && itemValidate;
    }
    //cout << "allValidate = " << allValidate << endl;
    if(allValidate) {
        m_runjobbtn->setEnabled(true);

        if((count==1 && !m_processWholeData) || count == 2) {
            m_paraTree->setParameterValuesToDom();
            m_mainWindow->setupParameters(m_domval);
        }
        setOutputParametersToSD2D();

    } else {
        m_runjobbtn->setEnabled(false);
    }
}

void Sdp2dProcessJobDockWidget::setOutputParametersToSD2D(void)
{
    if(m_processWholeData){
       m_sd2d->setOutputSegyFileName(m_ofname->text());
       QString order = m_outputitem->child(1)->data(3, Qt::EditRole).toString();

       cout << "Output file name = " << m_sd2d->getOutputSegyFileName().toStdString().c_str() << endl;
       cout << "order = "<< order.toStdString().c_str() << endl;

       if(order.compare("CommonShot") == 0) {
           m_sd2d->setOutputOrder(DisplayGatherType::CommonShot);
       } else if(order.compare("CommonCDP") == 0) {
           m_sd2d->setOutputOrder(DisplayGatherType::CommonDepthPoint);
       } else if(order.compare("CommonOffset") == 0) {
           m_sd2d->setOutputOrder(DisplayGatherType::CommonOffset);
       } else if(order.compare("CommonReceiver") == 0) {
           m_sd2d->setOutputOrder(DisplayGatherType::CommonReceiver);
       }

       if(m_outputitem->child(2)->data(3, Qt::EditRole).toString().compare("True") == 0) {
           m_sd2d->setOutputDiscardBadTraces(true);
       } else {
           m_sd2d->setOutputDiscardBadTraces(false);
       }

       if(m_outputitem->child(3)->data(3, Qt::EditRole).toString().compare("True") == 0) {
           m_sd2d->setOutputApplyTopMute(true);
       } else {
           m_sd2d->setOutputApplyTopMute(false);
       }

       if(m_outputitem->child(4)->data(3, Qt::EditRole).toString().compare("True") == 0) {
           m_sd2d->setOutputApplyBtmMute(true);
       } else {
           m_sd2d->setOutputApplyBtmMute(false);
       }
    }
}

bool Sdp2dProcessJobDockWidget::checkItemStates(QTreeWidgetItem *item, bool quite)
{
    //QString name = item->data(0, Qt::EditRole).toString();
    QString type = item->data(1, Qt::EditRole).toString();
    QString value = item->data(3, Qt::EditRole).toString();
    //cout << "Checking name="<< name.toStdString().c_str() << " value=" << value.toStdString().c_str() << endl;

    if(type.compare("Group") == 0) {
        QCheckBox* cbox =  dynamic_cast<QCheckBox*>(m_paraTree->itemWidget(item, 2));        
        if(cbox->isChecked() == false && item->isExpanded() == false) return true;
    }

    int count = item->childCount();
    for(int i=0; i< count; i++){
        QTreeWidgetItem* child = item->child(i);
        bool validate = checkItemStates(child);
        if(!validate) return false;
    }

    if(type.count() < 1) return true;

    QCheckBox* cbox =  dynamic_cast<QCheckBox*>(m_paraTree->itemWidget(item, 2));

    if(cbox->isChecked()){
        if(type.compare("Group") != 0) {
            int nchar = value.count();
            if(nchar == 0 && !quite){
                QMessageBox message(QMessageBox::NoIcon,
                                    "Warn", QString("Parameter %1 is required!").arg(item->data(0, Qt::EditRole).toString()), QMessageBox::Ok, NULL);
                message.exec();
                return false;
            }
        }
    }
    return true;
}

void Sdp2dProcessJobDockWidget::outputSEGYFilenameChanged()
{
    QString fileName = m_ofname->text();
    m_sd2d->setOutputSegyFileName(fileName);
    cout << "Change file name = " << m_sd2d->getOutputSegyFileName().toStdString().c_str() << endl;

    m_outputitem->child(0)->setText(3, fileName);
}

void Sdp2dProcessJobDockWidget::cleanTreeView(void)
{
    m_domdoc->clear();
    m_domval->clear();
    m_paraTree->clear();
    m_outputitem = nullptr;

    m_infoView->setPlainText(QString(""));
    m_runjobbtn->setEnabled(false);

    m_apply2all->setCheckState(Qt::Unchecked);

    m_processWholeData = false;
}
