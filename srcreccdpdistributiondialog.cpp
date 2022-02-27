#include "srcreccdpdistributiondialog.h"
#include "ui_srcreccdpdistributiondialog.h"

#include "seismicdataprocessing2d.h"
#include "seismicdata2dprestack.h"

#include <QWidget>
#include <QDialog>
#include <QList>
#include <QString>
#include <QListWidget>
#include <QMenu>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QChart>
#include <QChartView>
#include <QScatterSeries>
#include <QAreaSeries>
#include <QLegend>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QScatterSeries>


using namespace QtCharts;


SrcRecCdpDistributionDialog::SrcRecCdpDistributionDialog(SeismicData2DPreStack* p, QString sd2dName, QWidget *mainWindow, QWidget* parent) :
    QDialog(parent),
    ui(new Ui::SrcRecCdpDistributionDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowMaximizeButtonHint);
    m_sd2d = p;

    m_mainWindow = dynamic_cast<SeismicDataProcessing2D*>(mainWindow);

    m_scaleFactorX = 1.0;
    m_scaleFactorY = 1.0;
    p->getAllShotXY(m_shotXY);
    p->getAllCDPXY(m_cdpXY);
    p->getAllReceiverXY(m_recvXY);

    //cout << "size of m_recvXY = " << m_recvXY.size() << endl;
    //cout << "size of m_shotXY = " << m_shotXY.size() << endl;
    //cout << "size of m_cdpXY = " << m_cdpXY.size() << endl;

    QPointF firstPt = p->getFirstPoint();
    QPointF lastPt  = p->getLastPoint();

    double xdiff = lastPt.x() - firstPt.x();
    double ydiff = lastPt.y() - firstPt.y();

    m_fx = firstPt.x() - xdiff/10.0;
    m_lx = lastPt.x() + xdiff/10.0;

    if(ydiff < 10) ydiff = 10;
    m_fy = firstPt.y() - ydiff/10.0;
    m_ly = lastPt.y() + ydiff/10.0;

    m_dx = m_lx-m_fx;
    m_dy = m_ly-m_fy;

    m_cx = (m_lx+m_fx)/2.0;
    m_cy = (m_ly+m_fy)/2.0;

    //cout << "fx=" << m_fx << " lx=" << m_lx << " fy=" << m_fy << " ly=" << m_ly << endl;
    //cout << "dx=" << m_dx << " cx=" << m_cx << " dy=" << m_dy << " cy=" << m_cy << endl;

    m_chart = new QChart();
    m_chart->setTitle(QString("Sources/Receivers/CDPs Distribution of ") + sd2dName);
    //chart->setAnimationOptions(QChart::SeriesAnimations);

    m_CDPSeries = new QScatterSeries(m_chart);
    //m_CDPSeries->setPen(QPen(QBrush(Qt::darkRed), 1));
    m_CDPSeries->setBorderColor(Qt::transparent);
    m_CDPSeries->setMarkerSize(2);
    //m_CDPSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    m_CDPSeries->setColor(Qt::darkRed);
    int nPt = m_cdpXY.count();
    int modvalue = nPt/10000;
    if( modvalue<1) modvalue =1;
    //cout << "npt=" << nPt << " modvalue = "<< modvalue << endl;
    for(int i=0; i< nPt; i++){
        if(i%modvalue == 1)  m_CDPSeries->append(m_cdpXY.at(i));
    }

    m_CDPSeries->setName("CDPs");
    m_chart->addSeries(m_CDPSeries);

    m_RecvSeries = new QScatterSeries(m_chart);
    //m_RecvSeries->setPen(QPen(QBrush(Qt::green), 2));
    m_RecvSeries->setColor(Qt::green);
    m_RecvSeries->setBorderColor(Qt::transparent);
    m_RecvSeries->setMarkerSize(2);
    m_RecvSeries->append(m_recvXY);
    m_RecvSeries->setName("Receivers");
    //m_RecvSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    //m_RecvSeries->setBrush(QBrush(Qt::green));
    m_chart->addSeries(m_RecvSeries);

    m_ShotSeries = new QScatterSeries(m_chart);
    //m_ShotSeries->setPen(QPen(QBrush(Qt::blue), 2));
    m_ShotSeries->setColor(Qt::blue);
    m_ShotSeries->setBorderColor(Qt::transparent);
    m_ShotSeries->setMarkerSize(2);
    m_ShotSeries->append(m_shotXY);
    m_ShotSeries->setName("Sources");
    //m_ShotSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    //m_ShotSeries->setBrush(QBrush(Qt::blue));
    m_chart->addSeries(m_ShotSeries);

    m_chart->createDefaultAxes();
    m_chart->axes(Qt::Horizontal).at(0)->setRange(m_fx, m_lx);
    m_chart->axes(Qt::Vertical).at(0)->setRange(m_fy,m_ly);
    m_chart->axes(Qt::Horizontal).at(0)->setTitleText("X coordinate");
    m_chart->axes(Qt::Vertical).at(0)->setTitleText("Y coordinate");

    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_chartView = new Sdp2dChartView(m_chart, m_mainWindow, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    ui->scrollArea->setWidget(m_chartView);
    ui->srcCheckBox->setChecked(true);
    ui->recCheckBox->setChecked(true);
    ui->cdpCheckBox->setChecked(true);

}

SrcRecCdpDistributionDialog::~SrcRecCdpDistributionDialog()
{
    delete ui;
}

void SrcRecCdpDistributionDialog::on_srcCheckBox_toggled(bool checked)
{
    if(checked) m_ShotSeries->show();
    else m_ShotSeries->hide();
}

void SrcRecCdpDistributionDialog::on_recCheckBox_toggled(bool checked)
{
    if(checked) m_RecvSeries->show();
    else m_RecvSeries->hide();
}

void SrcRecCdpDistributionDialog::on_cdpCheckBox_toggled(bool checked)
{
    if(checked) m_CDPSeries->show();
    else m_CDPSeries->hide();
}


void SrcRecCdpDistributionDialog::on_zoomInBtn_clicked()
{
    scaleChart(0.8, 0.8);
    //m_chart->zoomIn();
}

void SrcRecCdpDistributionDialog::on_zoomOutBtn_clicked()
{
    scaleChart(1.25, 1.25);
    //m_chart->zoomOut();
}

void SrcRecCdpDistributionDialog::on_fit2winBtn_clicked()
{
    m_scaleFactorX = 1.0;
    m_scaleFactorY = 1.0;
    //m_chart->zoomReset();
    scaleChart(1.0, 1.0);
}

void SrcRecCdpDistributionDialog::on_closeBtn_clicked()
{
    this->close();
}

void SrcRecCdpDistributionDialog::mouseDoubleClickEvent(QMouseEvent *event)
{
    //cout << "double clicked" << endl;
    this->showMaximized();
    QDialog::mouseDoubleClickEvent(event);
}

void SrcRecCdpDistributionDialog::scaleChart(float factorX, float factorY)
{
    m_scaleFactorX *= factorX;
    m_scaleFactorY *= factorY;
    double fx = m_cx - m_dx*m_scaleFactorX*0.5;
    double fy = m_cy - m_dy*m_scaleFactorY*0.5;
    double lx = m_cx + m_dx*m_scaleFactorX*0.5;
    double ly = m_cy + m_dy*m_scaleFactorY*0.5;
    //cout << "scale =" << m_scaleFactor << "fx=" << fx << " lx=" << lx << " fy=" << fy << " ly=" << ly << endl;
    m_chart->axes(Qt::Horizontal).at(0)->setRange(fx, lx);
    m_chart->axes(Qt::Vertical).at(0)->setRange(fy, ly);
}

double SrcRecCdpDistributionDialog::getScaleFactor()
{
    return m_scaleFactorX;
}

double SrcRecCdpDistributionDialog::getPlotedDataXLength()
{
    return m_dx*m_scaleFactorX;
}

double SrcRecCdpDistributionDialog::getPlotedDataYLength()
{
    return m_dy*m_scaleFactorY;
}


double SrcRecCdpDistributionDialog::getPlotedDataX0()
{
    double value = m_cx - m_dx*m_scaleFactorX*0.5;
    return value;
}

double SrcRecCdpDistributionDialog::getPlotedDataY0()
{
    double value = m_cy - m_dy*m_scaleFactorY*0.5;
    return value;
}


void SrcRecCdpDistributionDialog::on_zoomInXBtn_pressed()
{
    scaleChart(0.8, 1.0);
}

void SrcRecCdpDistributionDialog::on_zoomInYBtn_pressed()
{
    scaleChart(1.0, 0.8);
}

void SrcRecCdpDistributionDialog::on_zoomOutXBtn_pressed()
{
    scaleChart(1.25, 1.0);
}

void SrcRecCdpDistributionDialog::on_zoomOutYBtn_pressed()
{
    scaleChart(1.0, 1.25);
}
