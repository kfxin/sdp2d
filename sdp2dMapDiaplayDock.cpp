#include "sdp2dMapDiaplayDock.h"
#include "seismicdataprocessing2d.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"

#include <QList>
#include <QPointF>

#include <QListWidget>
#include <QMenu>
#include <QLineSeries>
#include <QValueAxis>
#include <QVBoxLayout>
#include <QScatterSeries>
#include <QHideEvent>

Sdp2dMapDiaplayDockWidget::Sdp2dMapDiaplayDockWidget(SeismicData2D* sd2d, QWidget *parent) :
    QDockWidget(parent)
{    
    setContentsMargins(5,5,5,5);
    setCursor(Qt::PointingHandCursor);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    //setAttribute(Qt::WA_DeleteOnClose);

    m_sd2d = sd2d;
    m_sd2d->setMapDisplayPointer(this);
    m_sd2d->addDisplayWidgets(this);
    m_mainWindow = dynamic_cast<SeismicDataProcessing2D*>(parent);
    m_statusbar = m_mainWindow->getStatusbarPointer();

    m_gatherType = 1;    
    m_selectedGroupIdx = 1;
    m_groupSeries = nullptr;
    m_subsetSeries = nullptr;
    m_recvSeries = nullptr;
    m_shotSeries = nullptr;
    m_cdpSeries = nullptr;

    m_isShow = true;

    m_myMenu = new QMenu(this);
    m_myMenu->addAction("Restore Display", this, &Sdp2dMapDiaplayDockWidget::restoreDisplay);

    setMinimumSize(400,400);
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);

    switch(sd2d->getDataType()){
    case SeismicDataType::PreStack:
        cout<<"dataType=SeismicDataTpye::PreStack"<<endl;
        loadPreStackMapData();
        displayPreStackMap(1, 1);
        break;
    case SeismicDataType::Stack:
        cout<<"dataType=SeismicDataTpye::Stack"<<endl;
    case SeismicDataType::Attribure:
        cout<<"dataType=SeismicDataTpye::Attribure"<<endl;
        break;
    }

    connect(this, &QDockWidget::customContextMenuRequested, this,  &Sdp2dMapDiaplayDockWidget::onActiveMyMenu);
}

Sdp2dMapDiaplayDockWidget::~Sdp2dMapDiaplayDockWidget()
{
    if(m_recvSeries != nullptr) delete m_recvSeries;
    if(m_shotSeries != nullptr) delete m_shotSeries;
    if(m_cdpSeries != nullptr) delete m_cdpSeries;
    if(m_groupSeries != nullptr) delete m_groupSeries;
    if(m_subsetSeries != nullptr) delete m_subsetSeries ;    
}

void Sdp2dMapDiaplayDockWidget::displayPreStackMap(int gatherType, int groupIdx)
{
    m_gatherType = gatherType;
    m_selectedGroupIdx = groupIdx;
    //cout << "displayPreStackMap  gatherType=" << m_gatherType << " groupIdx="<< groupIdx << endl;
    switch(m_gatherType){
    case DisplayGatherType::CommonShot:
        displayPreStackSrcRecMap();
        break;
    case DisplayGatherType::CommonDepthPoint:
        displayPreStackCDPMap();
        break;
    case DisplayGatherType::CommonOffset:
        this->hide();
        return;
        break;
    case DisplayGatherType::CommonReceiver:
        displayPreStackRecSrcMap();
        break;
    default:
        cout << "Selected gather display type is not supported. m_gatherType=" << m_gatherType << endl;
        break;
    }

    showLocsOfSelectedGroup(m_selectedGroupIdx);
}


void Sdp2dMapDiaplayDockWidget::loadPreStackMapData()
{
    SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack*>(m_sd2d);

    QPointF firstPt = sd2d->getFirstPoint();
    QPointF lastPt  = sd2d->getLastPoint();

    double xdiff = lastPt.x() - firstPt.x();
    double ydiff = lastPt.y() - firstPt.y();

    m_fx = firstPt.x() - xdiff/10.0;
    m_lx = lastPt.x() + xdiff/10.0;

    if(ydiff < 10) ydiff = 10;
    m_fy = firstPt.y() - ydiff/10.0;
    m_ly = lastPt.y() + ydiff/10.0;

    sd2d->getAllShotXY(m_shotXY);
    sd2d->getAveCDPXY(m_cdpXY);
    sd2d->getAllReceiverXY(m_recvXY);

    m_groupSeries = new QScatterSeries();
    m_groupSeries->setColor(Qt::red);
    m_groupSeries->setBorderColor(Qt::transparent);
    m_groupSeries->setMarkerSize(12);


    m_subsetSeries = new QScatterSeries();
    m_subsetSeries->setColor(Qt::magenta);
    m_subsetSeries->setBorderColor(Qt::transparent);
    m_subsetSeries->setPointLabelsVisible(false);
    m_subsetSeries->setMarkerSize(8);

    m_recvSeries = new QScatterSeries();
    m_recvSeries->setColor(Qt::blue);
    m_recvSeries->setBorderColor(Qt::transparent);
    m_recvSeries->setMarkerSize(4);
    m_recvSeries->append(m_recvXY);
    m_recvSeries->setName("Receivers");

    m_shotSeries = new QScatterSeries();
    m_shotSeries->setColor(Qt::blue);
    m_shotSeries->setBorderColor(Qt::transparent);
    m_shotSeries->setMarkerSize(4);
    m_shotSeries->append(m_shotXY);
    m_shotSeries->setName("Sources");

    m_cdpSeries = new QScatterSeries();
    m_cdpSeries->setColor(Qt::blue);
    m_cdpSeries->setBorderColor(Qt::transparent);
    m_cdpSeries->setMarkerSize(4);
    m_cdpSeries->append(m_cdpXY);
    m_cdpSeries->setName("CDPs");

    m_chart = new QChart();

    m_chart->addSeries(m_subsetSeries);
    m_chart->addSeries(m_groupSeries);
    m_chart->addSeries(m_recvSeries);
    m_chart->addSeries(m_shotSeries);
    m_chart->addSeries(m_cdpSeries);


    m_cdpSeries->hide();
    m_shotSeries->hide();
    m_recvSeries->hide();

    m_chart->createDefaultAxes();
    //m_chart->axes(Qt::Horizontal).first()->setRange(m_fx, m_lx);
    //m_chart->axes(Qt::Vertical).first()->setRange(m_fy, m_ly);
    m_chart->axes(Qt::Horizontal).at(0)->setRange(m_fx, m_lx);
    m_chart->axes(Qt::Vertical).at(0)->setRange(m_fy, m_ly);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartview = new Sdp2dChartView(m_chart, m_mainWindow, this);
    m_chartview->setRenderHint(QPainter::Antialiasing);
    this->setWidget(m_chartview);

    connect(m_shotSeries, &QScatterSeries::hovered, this, &Sdp2dMapDiaplayDockWidget::displayPreStackGatherLocation);
    connect(m_recvSeries, &QScatterSeries::hovered, this, &Sdp2dMapDiaplayDockWidget::displayPreStackGatherLocation);
    connect(m_cdpSeries, &QScatterSeries::hovered, this, &Sdp2dMapDiaplayDockWidget::displayPreStackGatherLocation);
    connect(m_shotSeries, &QScatterSeries::doubleClicked, this, &Sdp2dMapDiaplayDockWidget::doubleClickedScatterSeries);
    connect(m_recvSeries, &QScatterSeries::doubleClicked, this, &Sdp2dMapDiaplayDockWidget::doubleClickedScatterSeries);
    connect(m_cdpSeries, &QScatterSeries::doubleClicked, this, &Sdp2dMapDiaplayDockWidget::doubleClickedScatterSeries);
}

void Sdp2dMapDiaplayDockWidget::displayPreStackSrcRecMap(void)
{
    this->setWindowTitle("Source/Receiver Map");

    m_groupSeries->setName("Selected source");
    m_subsetSeries->setName("Receivers of the selected Source");
    m_groupSeries->clear();
    m_subsetSeries->clear();
    m_cdpSeries->hide();
    m_recvSeries->hide();
    m_shotSeries->show();

}


void Sdp2dMapDiaplayDockWidget::displayPreStackCDPMap(void)
{
    setWindowTitle("CDP/Receiver Map");

    m_groupSeries->setName("Selected CDP");
    m_subsetSeries->setName("Receivers of the selected CDP");
    m_groupSeries->clear();
    m_subsetSeries->clear();
    m_shotSeries->hide();
    m_recvSeries->hide();
    m_cdpSeries->show();
}


void Sdp2dMapDiaplayDockWidget::displayPreStackRecSrcMap(void)
{
    this->setWindowTitle("Receiver/Source Map");

    m_groupSeries->setName("Selected receiver");
    m_subsetSeries->setName("Sources of the selected receiver");
    m_groupSeries->clear();
    m_subsetSeries->clear();
    m_cdpSeries->hide();
    m_shotSeries->hide();
    m_recvSeries->show();
}

void Sdp2dMapDiaplayDockWidget::displayPreStackGatherLocation(const QPointF &point, bool state)
{
    if(state) showLocsOfSelectedGroup(point);
}

void Sdp2dMapDiaplayDockWidget::doubleClickedScatterSeries(void)
{
    if(m_groupIdx == 0) return;
    SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack*>(m_sd2d);
    m_selectedGroupIdx = m_groupIdx;
    m_selectedGroupXY  = m_groupXY;
    sd2d->setPlotGroupIndex(m_groupIdx);

    if(sd2d->getInteractiveFunction() == InteractiveFunctions::StackVelAnalysis){
        sd2d->prepareStackVelocityAnalysis();
    }
}

/*

void Sdp2dMapDiaplayDockWidget::redrawFigure(bool shown)
{
    if(shown) cout <<"Sdp2dMapDiaplayDockWidge  redrawFigure width=" << this->width() << " height=" << this->height() << endl;
    else cout <<"Sdp2dMapDiaplayDockWidge hide" << endl;
}


void Sdp2dMapDiaplayDockWidget::enterEvent(QEvent *event)
{
    //cout <<"Sdp2dMapDiaplayDockWidget enterEvent width=" << this->width() << " height=" << this->height() << endl;
    //QDockWidget::focusOutEvent(event);
}
*/


void Sdp2dMapDiaplayDockWidget::showEvent(QShowEvent *event)
{
    m_isShow = true;
    QDockWidget::showEvent(event);
}


void Sdp2dMapDiaplayDockWidget::hideEvent(QHideEvent *event)
{
    m_isShow = false;
    QDockWidget::hideEvent(event);
}


void Sdp2dMapDiaplayDockWidget::leaveEvent(QEvent *event)
{
    //cout << "leaveEvent: m_gatherType=" << m_gatherType << " m_selectedGroupXY="<< m_selectedGroupXY << endl;
    showLocsOfSelectedGroup(m_selectedGroupIdx);
    QDockWidget::leaveEvent(event);
}

void Sdp2dMapDiaplayDockWidget::showLocsOfSelectedGroup(const QPointF &point)
{
    if(m_gatherType == DisplayGatherType::CommonOffset) return;
    if(m_subsetXY.count()>0) m_subsetXY.clear();
    SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack*>(m_sd2d);

    m_groupIdx = sd2d->getSubsetXYUsingGroupXY(point, m_subsetXY, m_gatherType);

    if(m_groupIdx == 0) return;
    m_groupXY = point;

    if(m_subsetSeries->count() == 0){
        m_subsetSeries->append(m_subsetXY);
    }else{
        m_subsetSeries->replace(m_subsetXY);
    }

    if(m_groupSeries->count() != 0){
        m_groupSeries->clear();
    }
    m_groupSeries->append(point);
}

void Sdp2dMapDiaplayDockWidget::showLocsOfSelectedGroup(int groupIdx)
{
    if(m_gatherType == DisplayGatherType::CommonOffset) return;
    if(groupIdx <= 0) return;
    if(m_subsetXY.count()>0) m_subsetXY.clear();
    SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack*>(m_sd2d);

    QPointF point = sd2d->getSubsetXYUsingGroupIdx(groupIdx, m_subsetXY, m_gatherType);

    m_groupIdx = groupIdx;
    m_groupXY = point;

    m_selectedGroupIdx = m_groupIdx;
    m_selectedGroupXY  = m_groupXY;

    if(m_subsetSeries->count() == 0){
        m_subsetSeries->append(m_subsetXY);
    }else{
        m_subsetSeries->replace(m_subsetXY);
    }

    if(m_groupSeries->count() != 0){
        m_groupSeries->clear();
    }
    m_groupSeries->append(point);
}

void Sdp2dMapDiaplayDockWidget::onActiveMyMenu(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    m_myMenu->exec(globalPos);
}

void Sdp2dMapDiaplayDockWidget::restoreDisplay(void)
{
    m_chart->axes(Qt::Horizontal).at(0)->setRange(m_fx, m_lx);
    m_chart->axes(Qt::Vertical).at(0)->setRange(m_fy, m_ly);
}
