#include "sdp2dOpenedDataDock.h"
#include "seismicdataprocessing2d.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "srcreccdpdistributiondialog.h"
#include "sdp2dAmplitudeDisplayDock.h"
#include "sdp2dDataInfoTabs.h"
#include "sdp2dDisplayParamTab.h"

#include <QListWidget>
#include <QMenu>
#include <QLineSeries>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QSplitter>
#include <QFrame>

Sdp2dOpenedDataDockWidget::Sdp2dOpenedDataDockWidget(QList<SeismicData2D*>& sd2dlist, SeismicData2D* current_sd2d, SeismicDataProcessing2D *parent) :
    QDockWidget(parent), m_sd2dlist(sd2dlist)
{
    m_mainWindow = parent;
    m_infotabs = nullptr;
    m_statusbar = m_mainWindow->getStatusbarPointer();

    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
    setObjectName("OpenedDataListDock");
    setWindowTitle("Opened SEGY Data");
    setFeatures(QDockWidget::DockWidgetMovable);
    setContentsMargins(5, 5, 5, 5);

    QWidget *dockWidgetContents = new QWidget();
    setWidget(dockWidgetContents);

    m_datalist = new QListWidget;
    m_datalist->setObjectName(QString::fromUtf8("mainDataListWidget"));
    m_datalist->setContextMenuPolicy(Qt::CustomContextMenu);

    appendNewFileToList(current_sd2d);

    createFileInfoDock(current_sd2d, true);

    m_page = new QSplitter();    
    m_page->setFrameShadow(QFrame::Sunken);

    m_datalist->resize(200, 500);
    m_infotabs->resize(300, 500);
    m_page->resize(500, 500);

    m_page->addWidget(m_datalist);
    m_page->addWidget(m_infotabs);

    m_layout = new QHBoxLayout(dockWidgetContents);
    m_layout->addWidget(m_page);

    m_myMenu = new QMenu(this);
    m_myMenu->addAction("Display Scatters", this, &Sdp2dOpenedDataDockWidget::displayScattersDlg);
    m_myMenu->addAction("Display CDP fold",  this, &Sdp2dOpenedDataDockWidget::displayCDPFoldDlg);
    m_myMenu->addAction("Show file info",  this, &Sdp2dOpenedDataDockWidget::showFileInfoTabs);
    m_displayAmpDlg = new QAction("Display Amplitude");
    m_myMenu->addAction(m_displayAmpDlg);    

    connect(m_datalist, &QListWidget::customContextMenuRequested, this, &Sdp2dOpenedDataDockWidget::on_ListWidget_customContextMenuRequested);
    connect(m_datalist, &QListWidget::currentItemChanged, this, &Sdp2dOpenedDataDockWidget::on_mainDataListWidget_currentItemChanged);
    connect(m_datalist, &QListWidget::itemDoubleClicked, this, &Sdp2dOpenedDataDockWidget::on_mainDataListWidget_itemDoubleClicked);
    connect(m_displayAmpDlg, &QAction::triggered, this, &Sdp2dOpenedDataDockWidget::displayAmplitudeDlg);
}

Sdp2dOpenedDataDockWidget::~Sdp2dOpenedDataDockWidget()
{

}

void Sdp2dOpenedDataDockWidget::createFileInfoDock(SeismicData2D* sd2d, bool forceFInfoVisible)
{
    if(m_infotabs!= nullptr) {
        //cout << "m_infodock is not null" << endl;
        m_infotabs->updateInformatiom(sd2d, forceFInfoVisible);
    } else {        
        m_infotabs = new Sdp2dDataInfoTabs(sd2d, m_mainWindow);
        //connect(m_infotabs, &DataInfoDockWidget::close, this, [ = ]{m_infotabs=nullptr;});
    }
    m_infotabs->show();
}

void Sdp2dOpenedDataDockWidget::hideFileInfoTabs(void)
{
    m_infotabs->hide();
}

void Sdp2dOpenedDataDockWidget::showDisplayParameterTab(void)
{
    m_infotabs->showDisplayParameterTab();
}

void Sdp2dOpenedDataDockWidget::appendNewFileToList(SeismicData2D* current_sd2d)
{
    std::string& tmp = current_sd2d->getSEGYFileName();
    const QString namewithpath = QString::fromUtf8( tmp.data(), tmp.size() );
    int nchar = namewithpath.size() - namewithpath.lastIndexOf('/')-1;
    const QString fileName = namewithpath.right(nchar);
    QListWidgetItem* it = new QListWidgetItem(fileName);
    it->setToolTip(namewithpath);
    m_datalist->addItem(it);
    m_datalist->setCurrentItem(it);

    m_mainWindow->setCurrentDataPointer(current_sd2d);
    //SeismicData2D* sd2d = find2DSeismicDataPoint(it);
    //m_infotabs->updateInformatiom(sd2d);
}

void Sdp2dOpenedDataDockWidget::on_ListWidget_customContextMenuRequested(const QPoint &pos)
{
    if(m_datalist->count()==0) return;    
    QPoint globalPos = m_datalist->mapToGlobal(pos);
    m_myMenu->exec(globalPos);
}

void Sdp2dOpenedDataDockWidget::displayScattersDlg()
{
    if(m_datalist->count() <=0) return;

    QListWidgetItem *item = m_datalist->currentItem();
    SeismicData2DPreStack* sd2d = find2DSeismicDataPoint(item);

    if(sd2d == nullptr) return;

    SrcRecCdpDistributionDialog * acqusitionDisplayDlg = new SrcRecCdpDistributionDialog(sd2d, item->text(), m_mainWindow);
    acqusitionDisplayDlg->show();
}

void Sdp2dOpenedDataDockWidget::displayAmplitudeDlg()
{
    if(m_datalist->count() <=0) return;

    //cout << "in OpenedDataDockWidget::displayAmplitudeDlg()" << endl;
    //cout << " size of opened files list: " << m_sd2dlist.size() << endl;
    QListWidgetItem *item = m_datalist->currentItem();
    SeismicData2DPreStack* sd2d = find2DSeismicDataPoint(item);

    if(sd2d == nullptr) return;

    if(!sd2d->getSegyhandle()->getAmpCalculateStates()){
        m_statusbar->showMessage("Is working on extracting amplitude information. Please wait for a while.");
        return;
    }

    Sdp2dAmplitudeDisplayDock* ampdock = sd2d->getAmplitudeDockPointer();
    if(ampdock == nullptr){
        ampdock = new Sdp2dAmplitudeDisplayDock(sd2d, item->text(), m_mainWindow);
    } else {
        ampdock->raise();
    }

    ampdock->show();
    ampdock->setAmplitudeDockShow();
}

void Sdp2dOpenedDataDockWidget::displayCDPFoldDlg()
{
    if(m_datalist->count() <=0) return;
    //cout << "in OpenedDataDockWidget::displayCDPFold()" << endl;
    //cout << " size of opened files list: " << m_sd2dlist.size() << endl;
    QListWidgetItem *item = m_datalist->currentItem();
    SeismicData2DPreStack* sp = find2DSeismicDataPoint(item);
    //cout << "in displayCDPFold " << item->text().toStdString().c_str() << endl;

    if(sp == nullptr) return;

    QList<QPointF> cdpFoldList;
    int maxFold = sp->getFoldOfCDPs(cdpFoldList);
    int maxY = maxFold*1.1;
    int ncdp = cdpFoldList.count();

    double minX = cdpFoldList[0].x();
    double maxX = cdpFoldList[ncdp-1].x();
    double diff = maxX - minX;
    minX -= diff*0.1;
    maxX += diff*0.1;
    QChart* chart = new QChart();
    chart->setTitle(QString("Number of Traces per CDP of ") + item->text());
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QLineSeries* cdpSeries = new QLineSeries(chart);
    cdpSeries->append(cdpFoldList);
    cdpSeries->setName("CDP Fold");
    chart->addSeries(cdpSeries);
    chart->createDefaultAxes();
    chart->axes(Qt::Horizontal).at(0)->setRange(minX, maxX);
    chart->axes(Qt::Vertical).at(0)->setRange(0, maxY);
    chart->axes(Qt::Horizontal).at(0)->setTitleText("X coordinate");
    chart->axes(Qt::Vertical).at(0)->setTitleText("Number of Fold");

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).at(0));
    Q_ASSERT(axisY);
    axisY->setLabelFormat("%d ");
    Sdp2dChartView* chartView = new Sdp2dChartView(chart, m_mainWindow, this);
    chartView->setRenderHint(QPainter::Antialiasing);

    QDialog* cdpFoldDlg = new QDialog(this);
    cdpFoldDlg->setAttribute(Qt::WA_DeleteOnClose);
    cdpFoldDlg->resize(800, 600);
    QVBoxLayout *layout = new QVBoxLayout(cdpFoldDlg);
    layout->addWidget(chartView);

    cdpFoldDlg->show();
}

void Sdp2dOpenedDataDockWidget::showFileInfoTabs()
{
    if(m_datalist->count() == 0) {
        m_infotabs->hide();
        return;
    }
    QListWidgetItem *item = m_datalist->currentItem();
    if(item == nullptr) return;
    SeismicData2DPreStack* sd2d = find2DSeismicDataPoint(item);
    if(sd2d == nullptr) return;

    createFileInfoDock(sd2d, true);
}

SeismicData2DPreStack* Sdp2dOpenedDataDockWidget::find2DSeismicDataPoint(QListWidgetItem *item)
{
    if(m_sd2dlist.count()<=0) return nullptr;
    if(item == nullptr) return nullptr;

    const QString fname = item->toolTip();
    //cout << "Size of datalist "<< m_sd2dlist.size() << " to find file: "<< fname.toStdString().c_str() << endl;
    for (int i = 0; i < m_sd2dlist.size(); ++i) {
        string sname = m_sd2dlist.at(i)->getSEGYFileName();        
        const QString qname = QString::fromUtf8( sname.data(), sname.size() );
        if(qname.compare(fname) == 0) {
            //cout << "find the sd2d: " << fname.toStdString().c_str()<<endl;
            SeismicData2DPreStack* sp = dynamic_cast<SeismicData2DPreStack*>(m_sd2dlist.at(i));
            /*
            if(sp != nullptr){
                gathers* gp = sp->getReceiverGatherIndex();
                sp->checkIndexStructure(gp);
            }else{
                cout << "cannot convert SeismicData2DPreStack" << endl;
            }
            */
            return sp;
        }
    }
    return nullptr;
}

void Sdp2dOpenedDataDockWidget::setFileAsCurrentItem(QString fileName)
{
    for(int i=0; i< m_datalist->count(); i++){
        const QString qname = m_datalist->item(i)->toolTip();
        if(qname.compare(fileName) == 0) {
            m_datalist->setCurrentRow(i);
            SeismicData2D* sd2d = find2DSeismicDataPoint(m_datalist->item(i));
            createFileInfoDock(sd2d);
            return;
        }
    }
}

void Sdp2dOpenedDataDockWidget::on_mainDataListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
     Q_UNUSED(previous);
    //cout << "item="<< current << endl;
    if(m_datalist->count() == 0) {
        m_infotabs->hide();
        return;
    }
    if(current == nullptr) return;

    SeismicData2DPreStack* sd2d = find2DSeismicDataPoint(current);
    if(sd2d == nullptr) return;    
    createFileInfoDock(sd2d);

}

void Sdp2dOpenedDataDockWidget::on_mainDataListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    //cout << "current item: " << item->toolTip().toStdString().c_str() << endl;
    SeismicData2DPreStack* sd2d = find2DSeismicDataPoint(item);
    m_mainWindow->setCurrentDataPointer(sd2d);
}

SeismicData2D* Sdp2dOpenedDataDockWidget::removeCurrentItemFromDataList(void)
{
    if(m_datalist->count() <= 0) return nullptr;

    QListWidgetItem *item = m_datalist->takeItem(m_datalist->currentRow());
    SeismicData2D* sd2d = find2DSeismicDataPoint(item);
    delete item;
    return(sd2d);
}

void Sdp2dOpenedDataDockWidget::removeFilenameFromDataList(SeismicData2D* sd2d)
{
    string sname = sd2d->getSEGYFileName();
    //cout << "Size of m_sd2dlist "<<  m_datalist->count() << " fileName=" << sname.c_str() << endl;
    const QString qname = QString::fromUtf8( sname.data(), sname.size() );
    for(int i=0; i< m_datalist->count(); i++){
        QString fname = m_datalist->item(i)->toolTip();
        if(fname.compare(qname) == 0){            
            m_datalist->takeItem(i);
            break;
        }
    }
    //cout << "Size of m_sd2dlist "<<  m_datalist->count() << " removed fileName=" << sname.c_str() << endl;
}

void Sdp2dOpenedDataDockWidget::leaveEvent(QEvent *event)
{
    if(m_datalist->count() <= 0) return;
    string sname = m_mainWindow->getCurrentDataPointer()->getSEGYFileName();
    QString qname = QString::fromUtf8( sname.data(), sname.size() );
    setFileAsCurrentItem(qname);
    QDockWidget::leaveEvent(event);
}

void Sdp2dOpenedDataDockWidget::showEvent(QShowEvent *event)
{
    m_mainWindow->setCheckStatusOfOpenFilesMenuItem(true);
    QDockWidget::showEvent(event);
}


void Sdp2dOpenedDataDockWidget::hideEvent(QHideEvent *event)
{
    m_mainWindow->setCheckStatusOfOpenFilesMenuItem(false);
    QDockWidget::hideEvent(event);
}

Sdp2dDisplayParamTab* Sdp2dOpenedDataDockWidget::getDisplayParamTabPointer(void)
{
    return m_infotabs->getDisplayParamTabPointer();
}
