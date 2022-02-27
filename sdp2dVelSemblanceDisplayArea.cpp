#include "sdp2dVelSemblanceDisplayArea.h"
#include "sdp2dVelSemblanceDisplayPlot.h"
#include "seismicdataprocessing2d.h"
#include "sdp2dUtils.h"
#include "seismicdata2dprestack.h"
#include "sdp2dStackVelocityAnalysis.h"
#include "sdp2dMainGatherDisplayArea.h"
#include "sdp2dProcessedGatherDisplayArea.h"
#include "sdp2dQDomDocument.h"
#include "qcustomplot.h"

#include <QWidget>
#include <QScrollArea>
#include <QStringList>
#include <QString>
#include <QPointF>

#include <iostream>

bool compareIDX(value_idx a, value_idx b)
{
    return (a.idx < b.idx);
}

using namespace std;

Sdp2dVelSemblanceDisplayArea::Sdp2dVelSemblanceDisplayArea(SeismicDataProcessing2D *mainWin, QWidget* parent) :
    QScrollArea(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(400,400);
    m_mainWindow = mainWin;
    m_statusbar = m_mainWindow->getStatusbarPointer();
    m_sd2d = dynamic_cast<SeismicData2DPreStack*>(m_mainWindow->getCurrentDataPointer());
    m_sd2d->addDisplayWidgets(this);

    m_velana = nullptr;

    m_mainGather = m_sd2d->getInputGatherDisplayPointer();
    m_nmoGather = m_sd2d->getProcessedGatherDisplayPointer();

    m_plotColormapType = QCPColorGradient::gpBwr;    
    m_plotTitle = QString("Velocity Semblance");
    m_plotXLabel = QString("Velocity(m/s)");
    m_plotYLabel = QString("Time(s)");
    m_plotClipPercent = 99;
    m_zoomfit = true;

    m_defaultVelAtTop = 1500;
    m_defaultVelAtBtm = 4000;
    m_stackGather = false;
    m_normpow = 1.0;

    m_rhtCDP = 0;
    m_lftCDP = 0;


    m_customPlot = new Sdp2dVelSemblanceDisplayPlot(this);

    setWidget(m_customPlot);

    createSembDisplayAxisRect();

    createSembColorDisplayBase();

    createNMOVelocityPicksCurve();

    setupCustomPlot();

}

Sdp2dVelSemblanceDisplayArea::~Sdp2dVelSemblanceDisplayArea()
{

}


void Sdp2dVelSemblanceDisplayArea::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    //cout <<"Sdp2dMainGatherDisplayArea resizeEvent width=" << this->width() << " height=" << this->height() << endl;

    if(m_mainWindow->isGatherDisplayFitToScrollWindow()) {
       resizeToFitTheDisplay();
    }
}

void Sdp2dVelSemblanceDisplayArea::resizeToFitTheDisplay()
{
    m_customPlot->resize(this->width(),this->height());
    //cout << "resizeToFitTheDisplay width="<< this->width() << " height=" << this->height() << endl;
    calculatePlotScales();
}

void Sdp2dVelSemblanceDisplayArea::calculatePlotScales(void)
{

}

void Sdp2dVelSemblanceDisplayArea::createSembDisplayAxisRect(void)
{
    m_sembAxisRect = new QCPAxisRect(m_customPlot);
    m_sembAxisRect->setupFullAxesBox(true);
    m_sembAxisRect->setSizeConstraintRect(QCPLayoutElement::SizeConstraintRect(0));

    QMargins m = m_sembAxisRect->margins();
    m.setTop(0);
    m.setBottom(0);
    m_sembAxisRect->setMargins(m);

    m_marginVGroup = new QCPMarginGroup(m_customPlot);
    m_sembAxisRect->setMarginGroup(QCP::msBottom|QCP::msTop, m_marginVGroup);

    m_sembAxisRect->axis(QCPAxis::atRight)->setVisible(true);
    m_sembAxisRect->axis(QCPAxis::atRight)->setTickLength(0, 5);
    m_sembAxisRect->axis(QCPAxis::atRight)->setSubTickLength(0, 3);
    m_sembAxisRect->axis(QCPAxis::atRight)->setTickLabels(false);
    m_sembAxisRect->axis(QCPAxis::atRight)->ticker()->setTickCount(9);
    m_sembAxisRect->axis(QCPAxis::atRight)->setRangeReversed(true);
    m_sembAxisRect->axis(QCPAxis::atRight)->setBasePen(QPen(Qt::black, 2));
    m_sembAxisRect->axis(QCPAxis::atRight)->setTickPen(QPen(Qt::black, 2));
    m_sembAxisRect->axis(QCPAxis::atRight)->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    m_sembAxisRect->axis(QCPAxis::atRight)->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    m_sembAxisRect->axis(QCPAxis::atRight)->grid()->setSubGridVisible(true);
    m_sembAxisRect->axis(QCPAxis::atRight)->setNumberPrecision(3);

    m_sembAxisRect->axis(QCPAxis::atLeft)->grid()->setZeroLinePen(QPen(Qt::black, 1));
    m_sembAxisRect->axis(QCPAxis::atLeft)->setLabel(m_plotYLabel);
    m_sembAxisRect->axis(QCPAxis::atLeft)->setVisible(true);
    m_sembAxisRect->axis(QCPAxis::atLeft)->setTickLabels(true);
    m_sembAxisRect->axis(QCPAxis::atLeft)->setNumberPrecision(3);
    m_sembAxisRect->axis(QCPAxis::atLeft)->ticker()->setTickCount(9);
    m_sembAxisRect->axis(QCPAxis::atLeft)->setBasePen(QPen(Qt::black, 2));
    m_sembAxisRect->axis(QCPAxis::atLeft)->setTickPen(QPen(Qt::black, 2));
    m_sembAxisRect->axis(QCPAxis::atLeft)->setSubTickPen(QPen(Qt::black, 1));
    m_sembAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::black);
    m_sembAxisRect->axis(QCPAxis::atLeft)->setTickLength(0, 5);
    m_sembAxisRect->axis(QCPAxis::atLeft)->setSubTickLength(0, 3);
    m_sembAxisRect->axis(QCPAxis::atLeft)->setRangeReversed(true);

    m_sembAxisRect->axis(QCPAxis::atTop)->ticker()->setTickCount(9);
    m_sembAxisRect->axis(QCPAxis::atTop)->setVisible(true);

    m_sembAxisRect->axis(QCPAxis::atTop)->setBasePen(QPen(Qt::black, 2));
    m_sembAxisRect->axis(QCPAxis::atTop)->setTickPen(QPen(Qt::black, 2));
    m_sembAxisRect->axis(QCPAxis::atTop)->setSubTickPen(QPen(Qt::black, 1));
    m_sembAxisRect->axis(QCPAxis::atTop)->setTickLabelColor(Qt::black);
    m_sembAxisRect->axis(QCPAxis::atTop)->setTickLength(0, 5);
    m_sembAxisRect->axis(QCPAxis::atTop)->setSubTickLength(0, 3);
    m_sembAxisRect->axis(QCPAxis::atTop)->setTickLabels(true);
    m_sembAxisRect->axis(QCPAxis::atTop)->ticker()->setTickCount(5);

    m_sembAxisRect->axis(QCPAxis::atTop)->setTickLabelPadding(1);
    m_sembAxisRect->axis(QCPAxis::atTop)->setLabelPadding(0);

    m_sembAxisRect->axis(QCPAxis::atBottom)->setVisible(true);
    m_sembAxisRect->axis(QCPAxis::atBottom)->setTickLength(0, 5);
    m_sembAxisRect->axis(QCPAxis::atBottom)->setSubTickLength(0, 3);
    m_sembAxisRect->axis(QCPAxis::atBottom)->setTickLabels(false);
    m_sembAxisRect->axis(QCPAxis::atTop)->setLabel(m_plotXLabel);

}

void Sdp2dVelSemblanceDisplayArea::createSembColorDisplayBase(void)
{
    m_colorScale = new QCPColorScale(m_customPlot);
    m_colorScale->axis()->setLabel("Semblance Amplitude");
    m_colorScale->axis()->setNumberFormat("eb");
    m_colorScale->axis()->setNumberPrecision(1);
    m_colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, m_marginVGroup);

    m_colorMap = new QCPColorMap(m_sembAxisRect->axis(QCPAxis::atTop), m_sembAxisRect->axis(QCPAxis::atLeft));
    m_colorMap->setInterpolate(true);
    m_colorMap->setColorScale(m_colorScale); // associate the color map with the color scale

}

void Sdp2dVelSemblanceDisplayArea::setupCustomPlot(void)
{
    m_customPlot->plotLayout()->clear();
    m_customPlot->plotLayout()->addElement(0, 0, new QCPTextElement(m_customPlot, m_plotTitle, 18));

    int rowIdx = 1;
    m_customPlot->plotLayout()->addElement(rowIdx, 0, m_sembAxisRect);
    m_customPlot->plotLayout()->addElement(rowIdx, 1, m_colorScale);

    m_colorMap->setVisible(true);
    m_colorScale->setVisible(true);

}

Sdp2dStackVelocityAnalysis* Sdp2dVelSemblanceDisplayArea::setupSembDisplay(Sdp2dQDomDocument* para)
{
    if(m_velana==nullptr){
        m_velana = new Sdp2dStackVelocityAnalysis(m_sd2d);
    }

    if(!getParametersFromDom(para)) return nullptr;

    int index = m_customPlot->plotLayout()->rowColToIndex(0, 0);
    QCPTextElement* e1 = dynamic_cast<QCPTextElement*>(m_customPlot->plotLayout()->elementAt(index));
    e1->setText(m_plotTitle);

    QCPColorGradient a((QCPColorGradient::GradientPreset)m_plotColormapType);
    m_colorMap->setGradient(a);

    int nv = m_velana->getNumberOfVelocities();
    float dt = m_velana->getTimeSampleRateOfSembelance();
    int nt = m_velana->getTimeSamplesOfSembelance();
    float maxTime = (nt-1)*dt;
    float fv = m_velana->getFirstVelocity();
    float lv = m_velana->getLastVelocity();


    m_sembAxisRect->axis(QCPAxis::atTop)->setRange(fv, lv);
    m_sembAxisRect->axis(QCPAxis::atBottom)->setRange(fv, lv);

    m_sembAxisRect->axis(QCPAxis::atLeft)->setRange(0, maxTime);
    m_sembAxisRect->axis(QCPAxis::atRight)->setRange(0, maxTime);

    if(!m_colorMap->data()->isEmpty())   m_colorMap->data()->clear();
    m_colorMap->data()->setSize(nv, nt);
    m_colorMap->data()->setRange(QCPRange(fv, lv), QCPRange(0, maxTime));

    return m_velana;
}

void Sdp2dVelSemblanceDisplayArea::setDataOfSemblance()
{
    setVelocitySembalance();
    plotLeftNMPVelocityCurve();
    plotRightNMPVelocityCurve();    
}

void Sdp2dVelSemblanceDisplayArea::setVelocitySembalance()
{
    int nv = m_velana->getNumberOfVelocities();
    int nt = m_velana->getTimeSamplesOfSembelance();
    float** data = m_velana->getSemblancePointer();

    float datavalue = 0;
    float wclip = 0;
    float bclip = 0;

    int nz = nv*nt;
    float* temp = new float [nz];
    memcpy((void*)temp, (void*)data[0], nz*sizeof(float));

    int idx = int(nz * m_plotClipPercent / 100.0);
    if(idx == nz) idx = nz-1;
    Sdp2dUtils::qkfind (idx, nz, temp);
    bclip = temp[idx];

    idx = int(nz * (100. - m_plotClipPercent) / 100.0);
    Sdp2dUtils::qkfind (idx, nz, temp);
    wclip = temp[idx];
    delete [] temp;
    //cout << "blip="<< bclip << " wclip=" << wclip << endl;

    for (int xIndex=0; xIndex<nv; xIndex++){
        for (int yIndex=0; yIndex<nt; yIndex++){
            datavalue = data[xIndex][yIndex];
            if(datavalue < wclip) datavalue = wclip;
            else if(datavalue > bclip) datavalue = bclip;
            m_colorMap->data()->setCell(xIndex, yIndex, datavalue);
        }
    }
    m_colorMap->rescaleDataRange(true);
    m_customPlot->replot();
}

QPointF Sdp2dVelSemblanceDisplayArea::convertPosToAxisValues(QPointF pos)
{
    double x = pos.x();
    double y = pos.y();
    double key=0, value=0;
    m_colorMap->pixelsToCoords( x,  y,  key,  value);
    return QPointF(key,  value);
}

void Sdp2dVelSemblanceDisplayArea::createNMOVelocityPicksCurve(void)
{
    QPen myPen;

    //top predicted curve
    myPen.setColor(QColor(Qt::yellow));
    myPen.setStyle(Qt::DashLine);
    myPen.setWidthF(3);
    m_top = new QCPGraph(m_sembAxisRect->axis(QCPAxis::atTop), m_sembAxisRect->axis(QCPAxis::atLeft));
    m_top->setPen(myPen);
    m_top->setLineStyle(QCPGraph::lsLine);

    // btm predicted curce
    m_btm = new QCPGraph(m_sembAxisRect->axis(QCPAxis::atTop), m_sembAxisRect->axis(QCPAxis::atLeft));
    m_btm->setPen(myPen);
    m_btm->setLineStyle(QCPGraph::lsLine);

    // picks' curve
    myPen.setColor(QColor(Qt::red));
    myPen.setStyle(Qt::SolidLine);
    myPen.setWidthF(3);
    m_mid = new QCPGraph(m_sembAxisRect->axis(QCPAxis::atTop), m_sembAxisRect->axis(QCPAxis::atLeft));
    m_mid->setPen(myPen);
    m_mid->setLineStyle(QCPGraph::lsLine);
    m_mid->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,  7));

    // left curve
    myPen.setColor(QColor(Qt::green));
    myPen.setStyle(Qt::DashLine);
    myPen.setWidthF(2);
    m_lft = new QCPGraph(m_sembAxisRect->axis(QCPAxis::atTop), m_sembAxisRect->axis(QCPAxis::atLeft));
    m_lft->setPen(myPen);
    m_lft->setLineStyle(QCPGraph::lsLine);

    // right curve
    myPen.setColor(QColor(Qt::cyan));
    m_rht = new QCPGraph(m_sembAxisRect->axis(QCPAxis::atTop), m_sembAxisRect->axis(QCPAxis::atLeft));
    m_rht->setPen(myPen);
    m_rht->setLineStyle(QCPGraph::lsLine);

    // right curve
    myPen.setColor(QColor(Qt::white));
    m_ped = new QCPGraph(m_sembAxisRect->axis(QCPAxis::atTop), m_sembAxisRect->axis(QCPAxis::atLeft));
    m_ped->setPen(myPen);
    m_ped->setLineStyle(QCPGraph::lsLine);
}

float** Sdp2dVelSemblanceDisplayArea::getDataOfVelocitySemblance()
{
    return m_velana->getSemblancePointer();
}

int Sdp2dVelSemblanceDisplayArea::getNumberOfVelocities()
{
    return m_velana->getNumberOfVelocities();
}

int Sdp2dVelSemblanceDisplayArea::getNumberOfTimeSamples()
{
    return m_velana->getTimeSamplesOfSembelance();
}

QCPRange Sdp2dVelSemblanceDisplayArea::getTheVelocityRange()
{
    return m_sembAxisRect->axis(QCPAxis::atTop)->range();
}

QCPRange Sdp2dVelSemblanceDisplayArea::getTheTimeRange()
{
    return m_sembAxisRect->axis(QCPAxis::atLeft)->range();
}

float Sdp2dVelSemblanceDisplayArea::getTimeSampleRateOfSembelance()
{
    return m_velana->getTimeSampleRateOfSembelance();
}

float Sdp2dVelSemblanceDisplayArea::getFirstVelocity()
{
    return m_velana->getFirstVelocity();
}

float Sdp2dVelSemblanceDisplayArea::getVelocityInterval()
{
    return m_velana->getVelocityInterval();
}

void Sdp2dVelSemblanceDisplayArea::setParametersToDom(Sdp2dQDomDocument* para)
{
    if(m_velana != nullptr)  m_velana->setParametersToDom(para);

    para->setParameterInGroup("VelocityAtTimeZero", QString::number(m_defaultVelAtTop), "Picking");
    para->setParameterInGroup("VelocityAtMaxTime", QString::number(m_defaultVelAtBtm), "Picking");
}

bool Sdp2dVelSemblanceDisplayArea::getParametersFromDom(Sdp2dQDomDocument* para)
{
    if(m_sd2d->getDataType() != SeismicDataType::PreStack){
        QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("The input seismic data must be prestack data!"), QMessageBox::Ok, NULL);
        message.exec();
        return false;
    }

    if(!m_velana->getParametersFromDom(para)) return false;

    m_defaultVelAtTop = para->getParameterInGroup("VelocityAtTimeZero", "Picking").toInt();
    m_defaultVelAtBtm = para->getParameterInGroup("VelocityAtMaxTime", "Picking").toFloat();

    m_normpow = para->getParameterInGroup("normpow", "Stack").toFloat();
    m_stackGather = false;
    if(para->getParameterInGroup("OutputStack", "Stack").compare(QString("True")) == 0){
        m_stackGather = true;
    }
    return true;
}

void Sdp2dVelSemblanceDisplayArea::insertOnePick(int itsamp, float velocity)
{
    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();
    int nt = m_sd2d->getSamplesPerTraces();
    float dt = float(m_sd2d->getTimeSampleRateInUs())/1000000.0;
    int maxTime = int((nt-1)*dt*1000);

    int sampTol = int(m_sd2d->getTimeSampleRateInUs()/1000.0)*3;

    if(itsamp < sampTol) itsamp = 0;

    value_idx tvPair(itsamp, velocity);
    int cdpIdx = m_mainGather->getGatherIndex();

    QList<value_idx> tvPairs = velPicksAll.values(cdpIdx);

    if(tvPairs.count() >=1){

        int tlower = itsamp - sampTol;
        int tupper = itsamp + sampTol;
        if(tlower < 0) tlower = 0;
        if(tupper >maxTime) tupper = maxTime;
        //cout << "tlower=" << tlower << " tupper="<< tupper << " pickedT=" << itsamp << endl;

        for(int i=0; i< tvPairs.count(); i++){
            if(tvPairs.at(i).idx >= tlower &&  tvPairs.at(i).idx <= tupper){
                int nremoved = velPicksAll.remove(cdpIdx, tvPairs.at(i));
                if(nremoved != 1) {
                    cout << "nremoved=" << nremoved << " must be sth WRONG in insertOnePick" << endl;
                }
            }
        }
    }
    velPicksAll.insert(cdpIdx, tvPair);

    if(tvPairs.count() >2 ){
        if(m_top->visible()){
            tvPair.val = m_top->data()->at(0)->key;
            if(tvPair.idx > sampTol) {
                tvPair.idx = 0;
                velPicksAll.insert(cdpIdx, tvPair);
            }
        }
        if(m_btm->visible()){
            int btmLast = m_btm->dataCount() - 1;
            tvPair.val = m_btm->data()->at(btmLast)->key;
            if(tvPair.idx < (maxTime - sampTol)){
                tvPair.idx = maxTime;
                velPicksAll.insert(cdpIdx, tvPair);
            }

        }
    }
    tvPairs.clear();

    updatePicksCurveAndNMOGather();
}

void Sdp2dVelSemblanceDisplayArea::removeOnePick(int itsamp, float velocity)
{
    Q_UNUSED(velocity);

    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();

    int cdpIdx = m_mainGather->getGatherIndex();

    QList<value_idx> tvPairs = velPicksAll.values(cdpIdx);

    if(tvPairs.count() >0){
        int nt = m_sd2d->getSamplesPerTraces();
        float dt = float(m_sd2d->getTimeSampleRateInUs())/1000000.0;
        int maxTime = int((nt-1)*dt*1000);
        int sampTol = int(m_sd2d->getTimeSampleRateInUs()/1000.0)*5;

        int tlower = itsamp - sampTol;
        int tupper = itsamp + sampTol;
        if(tlower < 0) tlower = 0;
        if(tupper >maxTime) tupper = maxTime;
        //cout << "tlower=" << tlower << " tupper="<< tupper << endl;
        for(int i=0; i< tvPairs.count(); i++){
            if(tvPairs.at(i).idx >= tlower &&  tvPairs.at(i).idx <= tupper){
                int nremoved = velPicksAll.remove(cdpIdx, tvPairs.at(i));
                if(nremoved != 1) {
                    cout << "nremoved=" << nremoved << " must be sth WRONG in insertOnePick" << endl;
                }
            }
        }
    }
    tvPairs.clear();

    updatePicksCurveAndNMOGather();

}

void Sdp2dVelSemblanceDisplayArea::plotPickedNMPVelocityCurve()
{
    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();
    int cdpIdx = m_mainGather->getGatherIndex();
    QList<value_idx> tvPairs = velPicksAll.values(cdpIdx);

    if(tvPairs.count() > 0){
        if(tvPairs.count() >1)  std::sort(tvPairs.begin(), tvPairs.end(), compareIDX);
        int nt = m_velana->getTimeSamplesOfSembelance();
        float dt = m_velana->getTimeSampleRateOfSembelance();
        int maxTime = int((nt-1)*dt*1000);
        int sampTol = int(m_sd2d->getTimeSampleRateInUs()/1000.0)*2;

        QVector<double> qvkey;
        QVector<double> qvval;
        if(tvPairs.at(0).idx > sampTol){
            double key = m_defaultVelAtTop;
            double val = 0;
            bool lftVisible = m_lft->visible();
            bool rhtVisible = m_rht->visible();
            if(lftVisible){
                if(rhtVisible){
                    key = (m_lft->data()->at(0)->key + m_rht->data()->at(0)->key) * 0.5;
                    val = (m_lft->data()->at(0)->value + m_rht->data()->at(0)->value) * 0.5;
                } else {
                    key = m_lft->data()->at(0)->key;
                    val = m_lft->data()->at(0)->value;
                }
            } else if(rhtVisible){
                key = m_rht->data()->at(0)->key;
                val = m_rht->data()->at(0)->value;
            }
            double vel = tvPairs.at(0).val;
            double tim = tvPairs.at(0).idx/1000.0;

            qvkey.append(key);
            qvval.append(val);

            qvkey.append(vel);
            qvval.append(tim);

            m_top->setData(qvkey, qvval, true);
            m_top->setVisible(true);
        } else {
            m_top->setVisible(false);
        }

        if(tvPairs.last().idx < (maxTime - sampTol)){
            qvkey.clear();
            qvval.clear();

            double key = m_defaultVelAtBtm;
            double val = maxTime/1000.0;

            bool lftVisible = m_lft->visible();
            bool rhtVisible = m_rht->visible();
            int  lftLast = m_lft->dataCount() - 1;
            int  rhtLast = m_rht->dataCount() - 1;

            if(lftVisible){
                if(rhtVisible){
                    key = (m_lft->data()->at(lftLast)->key + m_rht->data()->at(rhtLast)->key) * 0.5;
                    val = (m_lft->data()->at(lftLast)->value + m_rht->data()->at(rhtLast)->value) * 0.5;
                } else {
                    key = m_lft->data()->at(lftLast)->key;
                    val = m_lft->data()->at(lftLast)->value;
                }
            } else if(rhtVisible){
                key = m_rht->data()->at(rhtLast)->key;
                val = m_rht->data()->at(rhtLast)->value;
            }

            double vel = tvPairs.last().val;
            double tim = tvPairs.last().idx/1000.0;

            qvkey.append(vel);
            qvval.append(tim);
            qvkey.append(key);
            qvval.append(val);

            m_btm->setData(qvkey, qvval, true);
            m_btm->setVisible(true);
        } else {
            m_btm->setVisible(false);
        }

        qvkey.clear();
        qvval.clear();
        for(int i=0; i< tvPairs.count(); i++){
            double tim = tvPairs.at(i).idx/1000.0;
            double vel = tvPairs.at(i).val;
            qvkey.append(vel);
            qvval.append(tim);
        }
        m_mid->setData(qvkey, qvval, true);
        m_mid->setVisible(true);
    } else {
        m_top->setVisible(false);
        m_btm->setVisible(false);
        m_mid->setVisible(false);
    }
    m_customPlot->replot();
    tvPairs.clear();
}

void Sdp2dVelSemblanceDisplayArea::plotLeftNMPVelocityCurve()
{
    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();
    QList<int> cdps = velPicksAll.uniqueKeys();

    if(cdps.count() == 0) {
        m_lft->setVisible(false);
        return;
    }

    int cdpIdx = m_mainGather->getGatherIndex();
    m_lftCDP = 0;
    for(int i=0; i< cdps.count(); i++) {
        if(cdps.at(i) < cdpIdx) {
            m_lftCDP = cdps.at(i);
        } else {
            break;
        }
    }
    if(m_lftCDP == 0){
        m_lft->setVisible(false);
        return;
    }

    QList<value_idx> tvPairs = velPicksAll.values(m_lftCDP);

    if(tvPairs.count() > 0){
        if(tvPairs.count() >1)  std::sort(tvPairs.begin(), tvPairs.end(), compareIDX);
        int nt = m_velana->getTimeSamplesOfSembelance();
        float dt = m_velana->getTimeSampleRateOfSembelance();
        int maxTime = int((nt-1)*dt*1000);
        int sampTol = int(m_sd2d->getTimeSampleRateInUs()/1000.0)*2;

        QVector<double> qvkey;
        QVector<double> qvval;
        if(tvPairs.at(0).idx > sampTol){
            qvkey.append(m_defaultVelAtTop);
            qvval.append(0);
        }

        for(int i=0; i< tvPairs.count(); i++){
            double tim = tvPairs.at(i).idx/1000.0;
            double vel = tvPairs.at(i).val;
            qvkey.append(vel);
            qvval.append(tim);
        }

        if(tvPairs.last().idx < (maxTime - sampTol)){
            double maxt = maxTime/1000.0;
            qvkey.append(m_defaultVelAtBtm);
            qvval.append(maxt);
        }
        m_lft->setData(qvkey, qvval, true);
        m_lft->setVisible(true);
    } else {
        m_lft->setVisible(false);
    }
    tvPairs.clear();
}

void Sdp2dVelSemblanceDisplayArea::plotRightNMPVelocityCurve()
{
    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();
    QList<int> cdps = velPicksAll.uniqueKeys();

    if(cdps.count() == 0) {
        m_rht->setVisible(false);
        return;
    }

    int cdpIdx = m_mainGather->getGatherIndex();
    m_rhtCDP = 0;
    for(int i=cdps.count()-1; i>=0; i--) {
        if(cdps.at(i) > cdpIdx) {
            m_rhtCDP = cdps.at(i);
        } else {
            break;
        }
    }
    if(m_rhtCDP == 0){
        m_rht->setVisible(false);
        return;
    }

    QList<value_idx> tvPairs = velPicksAll.values(m_rhtCDP);

    if(tvPairs.count() > 0){
        if(tvPairs.count() >1)  std::sort(tvPairs.begin(), tvPairs.end(), compareIDX);
        int nt = m_velana->getTimeSamplesOfSembelance();
        float dt = m_velana->getTimeSampleRateOfSembelance();
        int maxTime = int((nt-1)*dt*1000);
        int sampTol = int(m_sd2d->getTimeSampleRateInUs()/1000.0)*2;

        QVector<double> qvkey;
        QVector<double> qvval;
        if(tvPairs.at(0).idx > sampTol){
            qvkey.append(m_defaultVelAtTop);
            qvval.append(0);
        }

        for(int i=0; i< tvPairs.count(); i++){
            double tim = tvPairs.at(i).idx/1000.0;
            double vel = tvPairs.at(i).val;
            qvkey.append(vel);
            qvval.append(tim);
        }

        if(tvPairs.last().idx < (maxTime - sampTol)){
            double maxt = maxTime/1000.0;
            qvkey.append(m_defaultVelAtBtm);
            qvval.append(maxt);
        }
        m_rht->setData(qvkey, qvval, true);
        m_rht->setVisible(true);
    } else {
        m_rht->setVisible(false);
    }
    tvPairs.clear();
}

void Sdp2dVelSemblanceDisplayArea::cleanAllVelocityPicks(void)
{
    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();
    velPicksAll.clear();
    plotLeftNMPVelocityCurve();
    plotRightNMPVelocityCurve();
    updatePicksCurveAndNMOGather();
}

void Sdp2dVelSemblanceDisplayArea::saveNMOVelocities(void)
{
    const QString fileName = QFileDialog::getSaveFileName(this, "Save file", QDir::currentPath(), "NMO Velocity Picks (*.nvp)");
    cout << "Output file name: " << fileName.toStdString().c_str() << endl;
    QFile idxf(fileName);
    idxf.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&idxf);
    out.setFieldWidth(10);

    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();
    gathers* gcdp = m_sd2d->getCDPGatherIndex();

    // write NMO velocity picks HERE
    QList<int> cdps = velPicksAll.uniqueKeys();
    int ncdps = cdps.count();

    out << "nCDPs: " << ncdps << Qt::endl;

    for(int i=0; i < ncdps; i++){
        int cdpIdx = cdps.at(i);

        QList<value_idx> tvPairs = velPicksAll.values(cdpIdx);
        int npicks = tvPairs.count();

        int gIdx = cdpIdx - gcdp->minGroupValue;
        float cdpx = gcdp->group[gIdx].groupValue1;
        float cdpy = gcdp->group[gIdx].groupValue2;

        out.setFieldWidth(10);
        out << "CDPIndex: " << cdpIdx  <<  " cdpX: " << cdpx <<  " cdpY: " << cdpy <<  " nPicks: " << npicks << Qt::endl;
        std::sort(tvPairs.begin(), tvPairs.end(), compareIDX);

        out.setFieldWidth(6);
        for(int j=0; j< npicks; j++){
            out << "Time: " << tvPairs.at(j).idx/1000.0  <<  " Vel: " << tvPairs.at(j).val << Qt::endl;
        }
    }

    idxf.close();
}

void Sdp2dVelSemblanceDisplayArea::loadNMOVelocities(void)
{
    const QString fileName = QFileDialog::getOpenFileName(this, "Open File", "/", "NMO Velocity Picks (*.nvp)");
    cout << "Input file name: " << fileName.toStdString().c_str() << endl;

    QFile idxf(fileName);
    idxf.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream in(&idxf);

    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();

    velPicksAll.clear();
    int ncdps = 0;
    QString tmp;
    in >> tmp  >> ncdps;
    //cout << "nCDPs: " << ncdps << endl;
    value_idx tvPair;

    for(int i=0; i < ncdps; i++){
        int cdpIdx =0;
        int npicks = 0;
        float cdpx = 0;
        float cdpy = 0;

        in >> tmp  >> cdpIdx >> tmp  >> cdpx >> tmp  >> cdpy  >> tmp  >> npicks;
        //cout << "CDPIndex: " << cdpIdx  <<  " cdpX: " << cdpx <<  " cdpY: " << cdpy <<  " nPicks: " << npicks << endl;
        float time = 0;
        float vel = 0;

        for(int j=0; j< npicks; j++){
            in >> tmp >> time  >> tmp >> vel;
            tvPair.idx = int(time*1000);
            tvPair.val = vel;
            velPicksAll.insert(cdpIdx, tvPair);
            //cout << "Time: " << time  <<  " Vel: " << vel << endl;
        }
    }
    idxf.close();

    plotLeftNMPVelocityCurve();
    plotRightNMPVelocityCurve();
    updatePicksCurveAndNMOGather();
}

void Sdp2dVelSemblanceDisplayArea::updatePicksCurveAndNMOGather(float** indata)
{
    plotPickedNMPVelocityCurve();

    int ntr = m_mainGather->getNumTracesOfCurrentGather();
    int ns = m_sd2d->getSamplesPerTraces();

    bool removeInData = false;
    if(indata == nullptr){
        removeInData = true;
        indata = Sdp2dUtils::alloc2float(ns, ntr);
        m_mainGather->getDataOfInputSeismicGather(indata);
    }

    float** outdata = Sdp2dUtils::alloc2float(ns, ntr);
    float* ovv = new float[ns];

    if(generateNMOVelocityOfCurrentCDP(ovv)){
        //cout << "applyNmoOnCurrentGather : " << m_mainGather->getGatherIndex() << endl;
        m_velana->applyNmoOnCurrentGather(indata, outdata, ovv, ntr);
    } else {
        memcpy((char*)outdata[0], (char*)indata[0], sizeof(float)*ns*ntr);
    }
    m_nmoGather->displayOneGather(outdata);

    delete [] ovv;
    Sdp2dUtils::free2float(outdata);
    if(removeInData) Sdp2dUtils::free2float(indata);

}

void Sdp2dVelSemblanceDisplayArea::processCurrentGather()
{
    int ntr = m_mainGather->getNumTracesOfCurrentGather();
    int ns = m_sd2d->getSamplesPerTraces();
    //cout << "ntr = " << ntr << endl;

    float** indata = Sdp2dUtils::alloc2float(ns, ntr);
    m_mainGather->getDataOfInputSeismicGather(indata);

    m_velana->processCurrentGather(indata, ntr);
    setDataOfSemblance();

    updatePicksCurveAndNMOGather(indata);

    Sdp2dUtils::free2float(indata);
}


bool Sdp2dVelSemblanceDisplayArea::generateNMOVelocityOfCurrentCDP(float* ovv)
{
    bool interpVel = m_velana->isInterpolateVelocity();
    if(m_mid->visible() == false && !interpVel) return false;

    int ns = m_sd2d->getSamplesPerTraces();
    float dt = ((double)m_sd2d->getTimeSampleRateInUs())/1000000.0;

    bool lftVisible = m_lft->visible();
    bool rhtVisible = m_rht->visible();
    bool midVisible = m_mid->visible();

    //cout <<"lftVisible=" << lftVisible << " rhtVisible="<<rhtVisible <<" midVisible=" << midVisible << endl;

    if(lftVisible && rhtVisible && !midVisible){

        int nplft = m_lft->dataCount();
        float* tlft = new float [nplft];
        float* vlft = new float [nplft];
        for(int i=0; i< nplft; i++){
            tlft[i] = m_lft->data()->at(i)->value;
            vlft[i] = m_lft->data()->at(i)->key;
        }

        int nprht = m_rht->dataCount();
        float* trht = new float [nprht];
        float* vrht = new float [nprht];
        for(int i=0; i< nprht; i++){
            trht[i] = m_rht->data()->at(i)->value;
            vrht[i] = m_rht->data()->at(i)->key;
        }

        float ft = 0;
        float t  = ft;
        float v  = 0;
        float v1 = 0;
        float v2 = 0;

        float cdpIdx = m_mainGather->getGatherIndex();
        float cdplft = m_lftCDP;
        float cdprht = m_rhtCDP;
        float a1 = (cdprht - cdpIdx) /(cdprht - cdplft);
        float a2 = (cdpIdx - cdplft) /(cdprht - cdplft);
        //cout << "interpolate velocity. m_lftCDP="<< m_lftCDP << " m_rhtCDP=" << m_rhtCDP << " cdpIdx="<< cdpIdx << endl;
        QVector<double> qvkey;
        QVector<double> qvval;

        for(int i=0; i< ns; i++){
            Sdp2dUtils::intlin(nplft, tlft, vlft, vlft[0], vlft[nplft-1], 1, &t, &v1);
            Sdp2dUtils::intlin(nprht, trht, vrht, vrht[0], vrht[nprht-1], 1, &t, &v2);

            ovv[i] = a1*1.0/(v1*v1) + a2*1.0/(v2*v2);
            t += dt;
            v = a1*v1 + a2*v2;
            //cout << "i=" << i << " t ="<< t << " v="<< v << " a1=" << a1 << " a2=" << a2 << endl;
            qvkey.append(v);
            qvval.append(t);
        }

        delete [] tlft;
        delete [] vlft;
        delete [] trht;
        delete [] vrht;


        m_ped->setData(qvkey, qvval, true);
        m_ped->setVisible(true);
    } else {
        m_ped->setVisible(false);

        float* tim = nullptr;
        float* vel = nullptr;
        int npicks = 0;

        if(midVisible){
            int nmid = m_mid->dataCount();
            int ntop = m_top->dataCount();
            int nbtm = m_btm->dataCount();
            npicks =  nmid + ntop + nbtm;

            tim = new float [npicks];
            vel = new float [npicks];

            for(int i=0; i< ntop; i++){
                tim[i] = m_top->data()->at(i)->value;
                vel[i] = m_top->data()->at(i)->key;
            }
            for(int i = 0; i< nmid; i++){
                int j = ntop + i;
                tim[j] = m_mid->data()->at(i)->value;
                vel[j] = m_mid->data()->at(i)->key;
            }
            for(int i=0; i< nbtm; i++){
                int j = ntop + nmid + i;
                tim[j] = m_btm->data()->at(i)->value;
                vel[j] = m_btm->data()->at(i)->key;
            }
        } else if(lftVisible) {
            npicks = m_lft->dataCount();
            tim = new float [npicks];
            vel = new float [npicks];
            for(int i=0; i< npicks; i++){
                tim[i] = m_lft->data()->at(i)->value;
                vel[i] = m_lft->data()->at(i)->key;
            }
        } else if(rhtVisible) {
            npicks = m_rht->dataCount();
            tim = new float [npicks];
            vel = new float [npicks];
            for(int i=0; i< npicks; i++){
                tim[i] = m_rht->data()->at(i)->value;
                vel[i] = m_rht->data()->at(i)->key;
            }
        } else {
            return false;
        }

        float ft = 0;
        float t = ft;
        float v = 0;
        for(int i=0; i< ns; i++){
            Sdp2dUtils::intlin(npicks, tim, vel, vel[0], vel[npicks-1], 1, &t, &v);
            ovv[i] = 1.0/(v*v);
            t += dt;
            //cout << "i=" << i << " t ="<< t << " v="<< v  << endl;
        }

        delete [] tim;
        delete [] vel;
    }
    m_customPlot->replot();
    return true;
}

void Sdp2dVelSemblanceDisplayArea::processWholeData(Sdp2dQDomDocument* para)
{
    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();
    QList<int> cdps = velPicksAll.uniqueKeys();

    if(cdps.count() == 0) {
        QMessageBox message(QMessageBox::Information,
             "Warn", QString("No NMO velocity is found. Will NOT output anything"), QMessageBox::Ok, NULL);
        message.exec();
        return;
    }

    if(m_velana==nullptr){
        m_velana = new Sdp2dStackVelocityAnalysis(m_sd2d);
    }
    if(!getParametersFromDom(para)) return;

    int ns = m_sd2d->getSamplesPerTraces();

    QString sgyName = m_sd2d->getOutputSegyFileName();
    ofstream outfile(sgyName.toStdString(), ofstream::binary);

    EbcdicHeader* ehdr = m_sd2d->getSegyhandle()->get3200TextHeader();
    BinaryHeader* bhdr = m_sd2d->getSegyhandle()->get400BinaryHeader();

    EbcdicHeader* tmp_ehdr = new EbcdicHeader;
    memcpy((char*)tmp_ehdr, (char*)ehdr, TEXT_HEADER_SIZE);
    Sdp2dUtils::ascii2Ebcdic(tmp_ehdr);

    BinaryHeader* tmp_bhdr = new BinaryHeader;
    memcpy((char*)tmp_bhdr, (char*)bhdr, BINARY_HEADER_SIZE);

    tmp_bhdr->samples_per_trace = ns;
    tmp_bhdr->data_sample_format_code = 5;

    outfile.write((char*)tmp_ehdr, TEXT_HEADER_SIZE);
    outfile.write((char*)tmp_bhdr, BINARY_HEADER_SIZE);

    size_t dlen = ns * sizeof(float);

    gathers* gp = m_sd2d->getCDPGatherIndex();
    float* ovv = new float[ns];

    QProgressDialog dialog;
    dialog.setLabelText(QString("NMO Processing..."));
    dialog.setMaximum(gp->ngroups-1);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAutoClose(true);


    for(int gIdx=gp->minGroupValue; gIdx<=gp->maxGroupValue; gIdx++){
        int idx = gIdx - gp->minGroupValue;

        dialog.setValue(idx);
        if (dialog.wasCanceled()) break;

        int ntr = gp->group[idx].ntraces;
        float** indata = m_sd2d->getinputGatherUsingOutputOption(gIdx);

        generateNMOVelocityOfCDP(gIdx, ovv);

        float** outdata = Sdp2dUtils::alloc2float(ns, ntr);
        m_velana->applyNmoOnCurrentGather(indata, outdata, ovv, ntr);

        if(m_stackGather) {
            if(ntr > 1) {
                for(int it = 0; it < ns; it++) {
                    if(qAbs(outdata[0][it]) > 0.000001) ovv[it] = 1.0;
                    else ovv[it] = 0.0;
                }
                for(int itr = 1; itr < ntr; itr++){
                    for(int it = 0; it < ns; it++){
                        outdata[0][it] += outdata[itr][it];
                        if(qAbs(outdata[itr][it])> 0.000001) ovv[it] += 1.0;
                    }
                }
                for(int it = 0; it < ns; it++) {
                    if(qAbs(ovv[it]) > 0.000001) outdata[0][it] /= pow(ovv[it], m_normpow);
                    else ovv[it] = 0.0;
                }
            }
            int trIdx = gp->group[idx].traceIdx[0];
            TraceHeaders* thdr = m_sd2d->getSegyhandle()->getTraceHeader(trIdx);
            thdr->number_of_horizontally_stacked_traced_yielding_this_trace = ntr;
            outfile.write((char*)thdr, TRACE_HEADER_SIZE);
            outfile.write((char*)outdata[0], dlen);
        } else {
            for(int itr = 0; itr < ntr; itr++){
                int trIdx = gp->group[idx].traceIdx[itr];
                TraceHeaders* thdr = m_sd2d->getSegyhandle()->getTraceHeader(trIdx);
                outfile.write((char*)thdr, TRACE_HEADER_SIZE);
                outfile.write((char*)outdata[itr], dlen);
            }
        }

        Sdp2dUtils::free2float(indata);
        Sdp2dUtils::free2float(outdata);
    }

    outfile.close();
    delete [] ovv;
    m_statusbar->showMessage(QString("NMO is done, The result outputs to file %1!").arg(sgyName));
}

bool Sdp2dVelSemblanceDisplayArea::generateNMOVelocityOfCDP(int gIndex, float* ovv)
{
    QMultiMap<int, value_idx>& velPicksAll = m_sd2d->getNMOVelocityPicks();
    QList<int> cdps = velPicksAll.uniqueKeys();

    QList<value_idx> tvPairs = velPicksAll.values(gIndex);

    QVector<float> qtim;
    QVector<float> qvel;

    int ns = m_sd2d->getSamplesPerTraces();
    float dt = ((double)m_sd2d->getTimeSampleRateInUs())/1000000.0;
    int maxTime = int((ns-1)*dt*1000);
    int sampTol = int(m_sd2d->getTimeSampleRateInUs()/1000.0)*2;

    int cdplft = 0;
    for(int i=0; i< cdps.count(); i++) {
        if(cdps.at(i) < gIndex) {
            cdplft = cdps.at(i);
        } else {
            break;
        }
    }

    int cdprht = 0;
    for(int i=cdps.count()-1; i>=0; i--) {
        if(cdps.at(i) > gIndex) {
            cdprht = cdps.at(i);
        } else {
            break;
        }
    }
    //cout << "cdpIdx=" << gIndex << " cdplft="<< cdplft << " cdprht="<< cdprht << endl;

    if(tvPairs.count() > 0){
        fillTVPairs(tvPairs, qtim, qvel, maxTime, sampTol);
    } else if(cdplft>0 && cdprht==0){
        tvPairs.clear();
        tvPairs = velPicksAll.values(cdplft);
        fillTVPairs(tvPairs, qtim, qvel, maxTime, sampTol);
    } else if(cdplft == 0 && cdprht > 0){
        tvPairs.clear();
        tvPairs = velPicksAll.values(cdprht);
        fillTVPairs(tvPairs, qtim, qvel, maxTime, sampTol);
    } else if(cdplft>0 && cdprht >0){
        QVector<float> qtim1;
        QVector<float> qvel1;
        QVector<float> qtim2;
        QVector<float> qvel2;

        QList<value_idx> tvPairs1 = velPicksAll.values(cdplft);
        fillTVPairs(tvPairs1, qtim1, qvel1, maxTime, sampTol);
        QList<value_idx> tvPairs2 = velPicksAll.values(cdprht);
        fillTVPairs(tvPairs2, qtim2, qvel2, maxTime, sampTol);

        int nplft = qtim1.count();
        int nprht = qtim2.count();
        float* tlft = qtim1.data();
        float* vlft = qvel1.data();
        float* trht = qtim2.data();
        float* vrht = qvel2.data();
        float ft = 0;
        float t  = ft;
        float v  = 0;
        float v1 = 0;
        float v2 = 0;
        float a1 = float(cdprht - gIndex) /float(cdprht - cdplft);
        float a2 = float(gIndex - cdplft) /float(cdprht - cdplft);
        for(int i=0; i< ns; i++){
            Sdp2dUtils::intlin(nplft, tlft, vlft, vlft[0], vlft[nplft-1], 1, &t, &v1);
            Sdp2dUtils::intlin(nprht, trht, vrht, vrht[0], vrht[nprht-1], 1, &t, &v2);
            ovv[i] = a1*1.0/(v1*v1) + a2*1.0/(v2*v2);
            t += dt;
            v = a1*v1 + a2*v2;
            //cout << "i=" << i << " t ="<< t << " v="<< v << " a1=" << a1 << " a2=" << a2 << endl;
        }
        return true;
    }

    int npicks = qtim.count();
    //cout << "npicks=" << npicks << endl;
    float* tim = qtim.data();
    float* vel = qvel.data();
    float ft = 0;
    float t = ft;
    float v = 0;
    for(int i=0; i< ns; i++){
        Sdp2dUtils::intlin(npicks, tim, vel, vel[0], vel[npicks-1], 1, &t, &v);
        ovv[i] = 1.0/(v*v);
        t += dt;
        //cout << "i=" << i << " t ="<< t << " v="<< v  << endl;
    }
    return true;
}

void Sdp2dVelSemblanceDisplayArea::fillTVPairs(QList<value_idx>& tvPairs, QVector<float>& qtim, QVector<float>& qvel, int maxTime, int sampTol)
{
    if(tvPairs.at(0).idx > sampTol){
        qtim.append(0);
        qvel.append(m_defaultVelAtTop);
    }
    for(int i=0; i< tvPairs.count(); i++){
        float tim = tvPairs.at(i).idx/1000.0;
        float vel = tvPairs.at(i).val;
        qvel.append(vel);
        qtim.append(tim);
    }
    if(tvPairs.last().idx < (maxTime - sampTol)){
        qtim.append(maxTime/1000.0);
        qvel.append(m_defaultVelAtBtm);
    }
}
