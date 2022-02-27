#include "sdp2dMainGatherDisplayArea.h"
#include "sdp2dAmplitudeDisplayDock.h"
#include "seismicdataprocessing2d.h"
#include "sdp2dUtils.h"
#include "seismicdata2dprestack.h"
#include "seismicdata2d.h"
#include "sdp2dMainGatherDisplayPlot.h"
#include "qcustomplot.h"

#include <QColor>

#include <string.h>
#include <cmath>


Sdp2dMainGatherDisplayArea::Sdp2dMainGatherDisplayArea(SeismicData2D* sd2d, QWidget *parent) :
    Sdp2dGatherDisplayArea(sd2d, parent)
{
    m_sd2d = dynamic_cast<SeismicData2DPreStack*>(sd2d);
    m_sd2d->setInputGatherDisplayPointer(this);
    m_sd2d->addDisplayWidgets(this);

    m_colorBadTrace = QColor(140, 230, 0);
    m_colorSelTrace = QColor(230, 0 ,0);
    m_colorFreTrace = QColor(Qt::blue);

    //m_colorTmpCurve = QColor(255, 200, 20, 200);
    //m_colorTmpCurve = QColor(255, 215, 0);
    //m_colorTmpCurve = QColor(Qt::red);
    m_colorTmpCurve = QColor(230, 128, 255);

    m_midMuteGraph = nullptr;
    m_lftMuteGraph = nullptr;
    m_rhtMuteGraph = nullptr;

    m_wavefrom = nullptr;
    m_traceref = nullptr;
    m_displayedtraceIdx = 0;

    m_badTraces = nullptr;

    m_linearVelLine = nullptr;
    m_linearVelLable = nullptr;

    m_gatherOut = nullptr;

    int gidx = m_sd2d->getShotGatherIndex()->minGroupValue;
    while( m_sd2d->getNumberOfTracesOfGather(1, gidx) <=1 ) gidx++;

    m_gatherIdx = gidx;

    m_customPlot = new Sdp2dMainGatherDisplayPlot(this);
    setWidget(m_customPlot);

    createGatherDisplayAxisRect();

    createGatherColorDisplayBase();

    createOffsetDisplayAxisRect();

    setupCustomPlot();

    displayOneGather();

    connect(m_colorMap, SIGNAL(dataRangeChanged(const QCPRange&)), this, SLOT(displayedDataChanged(const QCPRange&)));
}

Sdp2dMainGatherDisplayArea::~Sdp2dMainGatherDisplayArea()
{
    if(m_gatherOut != nullptr) Sdp2dUtils::free2float(m_gatherOut);
    delete [] m_badTraces;
    m_badTrcGraphs.clear();
    m_badRefGraphs.clear();    
}

void Sdp2dMainGatherDisplayArea::displayedDataChanged(const QCPRange& range)
{
    // The group index in the sd2d has not changed till now.
    Q_UNUSED(range);
    //cout << " range: lower=" << range.lower << " upper=" << range.upper   << endl;

    if(m_mainWindow->isGatherDisplayFitToScrollWindow()) {
        //m_customPlot->resize(this->width()-20,this->height()-20);
        m_customPlot->resize(this->width(),this->height());
        calculatePlotScales();
    } else {
        int edge = m_plotWiggleScale;
        if(edge > 5) edge = 5;
        if(edge < 1) edge = 1;
        QPointF plotEdge = m_sd2d->getPlotEdge();
        double wchange = (m_ntr-1.0+2*edge)*m_xscale + plotEdge.x();
        double hchange = m_plotMaxTime * m_yscale + plotEdge.y();
        m_customPlot->resize(wchange,hchange);
    }

    m_customPlot->replot();
}

bool Sdp2dMainGatherDisplayArea::loadOneGroupData(void)
{
    if(m_gatherIn != nullptr) Sdp2dUtils::free2float(m_gatherIn);
    if(m_gatherOut != nullptr) {
        Sdp2dUtils::free2float(m_gatherOut);
        m_gatherOut = nullptr;
    }
    //cout << " in loadOneGroupData:  m_gatherIdx=" << m_gatherIdx << " m_plotGathereType="<< m_plotGathereType << endl;

    m_gatherIn = m_sd2d->getSeismicDataOfGather(m_plotGathereType, m_gatherIdx, m_ntr);
    if(m_ntr < 1) return false;

    loadOneGroupOffsetElevation();
    updateStatusBarMessage();        

    m_sd2d->updatePlotTitleForDisplayParaTab(m_plotTitle);
    if(m_plotXAxisType == XAxisType::TraceIdx) {
        m_sd2d->updateXLableForDisplayParaTab(QString("Trace Index"));
        m_sd2d->updateYLableForDisplayParaTab(QString("Time(s)"));
    } else if(m_plotXAxisType == XAxisType::OffsetVal) {
        m_sd2d->updateXLableForDisplayParaTab(QString("Offset(m)"));
        m_sd2d->updateYLableForDisplayParaTab(QString("Time(s)"));
    }
    emit gatherChanged();

    return true;
}

void Sdp2dMainGatherDisplayArea::loadOneGroupOffsetElevation()
{
    m_offset.resize(m_ntr);
    m_elevation.resize(m_ntr);
    m_sd2d->getOffsetAndElevOfGather(m_plotGathereType, m_gatherIdx, m_ntr, m_offset.data(), m_elevation.data());

    double offmin = std::numeric_limits<double>::max();
    double offmax = -std::numeric_limits<double>::max();
    double elemin = std::numeric_limits<double>::max();
    double elemax = -std::numeric_limits<double>::max();
    for(int i=0; i< m_ntr; i++){
        if(offmin > m_offset[i]) offmin = m_offset[i];
        if(offmax < m_offset[i]) offmax = m_offset[i];
        if(elemin > m_elevation[i]) elemin = m_elevation[i];
        if(elemax < m_elevation[i]) elemax = m_elevation[i];
    }

    m_eleRange->lower = elemin - (elemax-elemin)*0.05 - 1;
    m_eleRange->upper = elemax + (elemax-elemin)*0.05 + 1;

    m_offEdge = (offmax-offmin)*1.5/m_ntr;
    if(m_offEdge < 1) m_offEdge = 1;

    m_offRange->lower = offmin;
    m_offRange->upper = offmax;

}

void Sdp2dMainGatherDisplayArea::updateStatusBarMessage(void)
{
    switch(m_plotGathereType){
    case DisplayGatherType::CommonShot:
        m_plotTitle  = QString("Common Shot Gather %1").arg(m_gatherIdx);
        m_statusbar->showMessage(QString("Loading common shot gather %1. Number of traces is %2").arg(m_gatherIdx).arg(m_ntr));
        break;
    case DisplayGatherType::CommonDepthPoint:
        m_plotTitle  = QString("CDP %1").arg(m_gatherIdx);
        m_statusbar->showMessage(QString("Loading CDP gather %1. Number of traces is %2").arg(m_gatherIdx).arg(m_ntr));
        break;
    case DisplayGatherType::CommonOffset:
        m_plotTitle  = QString("Common Offset Gather %1").arg(m_gatherIdx);
        m_statusbar->showMessage(QString("Loading common offset gather %1. Number of traces is %2").arg(m_gatherIdx).arg(m_ntr));
        break;
    case DisplayGatherType::CommonReceiver:
        m_plotTitle  = QString("Common Receiver Gather %1").arg(m_gatherIdx);
        m_statusbar->showMessage(QString("Loading common receiver gather %1. Number of traces is %2").arg(m_gatherIdx).arg(m_ntr));
        break;
    }
}

void Sdp2dMainGatherDisplayArea::displayOneGather(void)
{    
    loadOneGroupData();
    setHeaderDisplay();
    setColorDisplay();
    Sdp2dMainGatherDisplayArea::setDataForColorDisplay();
    setWiggleDisplay();
    updateWiggleDisplayData();
    setBadTracesWithinOneGather();
    setMutePicksDisplay();
    cleanLinearVelocityLine();
    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::setMutePicksDisplay(void)
{
    int interactFun = m_sd2d->getInteractiveFunction();
    if(interactFun == InteractiveFunctions::PickingMute){
        m_sd2d->setMuteValuesOfCurrentGather(true);
    } else {
        m_sd2d->setMuteValuesOfCurrentGather(false);
    }
}

void Sdp2dMainGatherDisplayArea::updateWiggleDisplayData()
{    
    int keySize = m_colorMap->data()->keySize();

    if( keySize != m_wigTrcGraphs.size() ){
        cout << " keySize=" << keySize << " graphSize=" << m_wigTrcGraphs.size()<< ". Something WRONG!"  << endl;
        return;
    }

    bool*  badTraces  = new bool [keySize];
    for(int i=0; i< keySize; i++)   badTraces[i]  = false;

    //cout << "in update wiggle traces: m_sd2d->isHideBadTraces() = " << m_sd2d->isHideBadTraces() << endl;
    if(m_hideBadTraces){
        m_sd2d->getTraceFlags(m_plotGathereType, m_gatherIdx, badTraces);
    }

    for(int trSeq=0; trSeq < keySize; trSeq++){
        QCPGraph* wav = m_wigTrcGraphs[trSeq];
        QCPGraph* ref = m_wigRefGraphs[trSeq];
        if(badTraces[trSeq] && wav != nullptr) {
            m_customPlot->removeGraph(wav);
            m_customPlot->removeGraph(ref);
            m_wigTrcGraphs[trSeq] = nullptr;
            m_wigRefGraphs[trSeq] = nullptr;
            continue;
        }

        if(!badTraces[trSeq] && wav == nullptr) {
            //cout << "create wiggle trace "<< trSeq << endl;
            wav = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
            ref = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
            wav->setPen(QPen(Qt::black));
            wav->setBrush(QBrush(m_colorWigTrace));
            wav->setChannelFillGraph(ref);
            wav->antialiasedFill();
            ref->setPen(QPen(Qt::black));
            ref->setVisible(false);
            if(m_plotDisplayType == SeismicDisplayTpye::Color) wav->setVisible(false);
            m_wigTrcGraphs[trSeq] = wav;
            m_wigRefGraphs[trSeq] = ref;
        }

        setDataForDisplayedSingleTrace(wav, ref, trSeq+1.);
    }
    delete [] badTraces;
}

void Sdp2dMainGatherDisplayArea::setBadTracesWithinOneGather(bool checked)
{
    if(m_badTraces != nullptr) {
        for(int i = 0; i< m_badTrcGraphs.size(); i++){
            if(m_badTrcGraphs[i]!=nullptr){
                m_customPlot->removeGraph(m_badTrcGraphs[i]);
                m_customPlot->removeGraph(m_badRefGraphs[i]);
                //cout << " something is WRONG! i=" << i <<  endl;
            }
        }
        delete [] m_badTraces;
        m_badTraces = nullptr;
        m_badTrcGraphs.clear();
        m_badRefGraphs.clear();
    }
    if(checked == false) {
        if(m_plotDisplayType != SeismicDisplayTpye::Color){
            for(int trSeq=0; trSeq< m_ntr; trSeq++) {
                if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(true);
            }
        }
        m_customPlot->replot();
        return;
    }
    if(m_mainWindow->getInteractiveFunction() != InteractiveFunctions::BadTraceSelection) return;

    m_badTraces = new bool [m_ntr];
    m_badTrcGraphs.resize(m_ntr);
    m_badRefGraphs.resize(m_ntr);

    m_sd2d->getTraceFlags(m_plotGathereType, m_gatherIdx, m_badTraces);

    //cout << "initial bad traces of the current gather. m_ntr="<< m_ntr << endl;
    for(int trSeq=0; trSeq< m_ntr; trSeq++){
        if(m_badTraces[trSeq]){
            QCPGraph* wav = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
            QCPGraph* ref = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
            drawOneWiggleTrace(wav, ref, trSeq, m_colorBadTrace);
            m_badTrcGraphs[trSeq] = wav;
            m_badRefGraphs[trSeq] = ref;
            if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(false);
        }else{
            m_badTrcGraphs[trSeq] = nullptr;
            m_badRefGraphs[trSeq] = nullptr;
        }
    }
    m_customPlot->replot();
}

// For the time being, only top mute is implemented.

void Sdp2dMainGatherDisplayArea::setDataForColorDisplay()
{
    int ns = int((1000*m_plotMaxTime)/(1000.0*m_dt))+1;

    short* topMuteValues = nullptr;
    short* btmMuteValues = nullptr;

    bool*  badTraces  = new bool [m_ntr];
    float* weight =  new float [m_ns];
    for(int i=0; i< m_ns; i++) weight[i] =1;
    for(int i=0; i< m_ntr; i++)  badTraces[i]  = false;

    int taperLength = m_sd2d->getMuteTaperLength();

    if(m_applyTopMute){
        topMuteValues = new short [m_ntr];
        memset((void*)topMuteValues, 0, m_ntr*sizeof(short));
        m_sd2d->getTracesMute(m_plotGathereType, m_gatherIdx, topMuteValues, 1);
    }
    if(m_applyBtmMute){
        btmMuteValues = new short [m_ntr];
        memset((void*)btmMuteValues, 0, m_ntr*sizeof(short));
        m_sd2d->getTracesMute(m_plotGathereType, m_gatherIdx, btmMuteValues, 2);
    }
    if(m_hideBadTraces){
        m_sd2d->getTraceFlags(m_plotGathereType, m_gatherIdx, badTraces);
    }
    //cout << "muteData = " << muteData << endl;

    float datavalue = 0;
    float wclip = 0;
    float bclip = 0;

    int nz = m_ntr*ns;
    float* temp = new float [nz];
    memcpy((void*)temp, (void*)m_gatherIn[0], nz*sizeof(float));
    if(m_plotRevPolarity) for(int i=0; i<nz; i++) temp[i] *= -1.0;

    int idx = int(nz * m_plotClipPercent / 100.0);
    if(idx == nz) idx = nz-1;
    Sdp2dUtils::qkfind (idx, nz, temp);
    bclip = temp[idx];

    idx = int(nz * (100. - m_plotClipPercent) / 100.0);
    Sdp2dUtils::qkfind (idx, nz, temp);
    wclip = temp[idx];
    delete [] temp;

    if(m_plotSymRange){
        if(bclip > 0 ) {
            bclip = max(abs(bclip), abs(wclip));
        } else {
            bclip = -min(abs(bclip), abs(wclip));
        }
        if(wclip > 0) {
            wclip = min(abs(bclip), abs(wclip));
        } else {
            wclip = -max(abs(bclip), abs(wclip));
        }
    }

    for (int xIndex=0; xIndex<m_ntr; xIndex++){

        if(m_applyTopMute || m_applyBtmMute) {
            int top = 0;
            int btm = m_ns;
            if(m_applyTopMute) top = topMuteValues[xIndex];
            if(m_applyBtmMute) btm = btmMuteValues[xIndex];
            //cout << " xIndex=" << xIndex << " topmute=" << top << " btmMute="<< btm << endl;
            m_sd2d->calculateTraceWeightValues(top, btm, weight, taperLength);
        }

        for (int yIndex=0; yIndex<ns; yIndex++){
            datavalue = m_gatherIn[xIndex][yIndex] * weight[yIndex];
            if(datavalue < wclip) datavalue = wclip;
            else if(datavalue > bclip) datavalue = bclip;
            if(m_plotRevPolarity) datavalue = -datavalue;
            if(badTraces[xIndex]) datavalue = 0;
            m_colorMap->data()->setCell(xIndex, yIndex, datavalue);
        }
    }

    m_colorMap->rescaleDataRange(true);

    if(topMuteValues != nullptr) delete [] topMuteValues;
    if(btmMuteValues != nullptr) delete [] btmMuteValues;
    delete [] weight;
    delete [] badTraces;
}


void Sdp2dMainGatherDisplayArea::getDataOfInputSeismicGather(float** outData)
{
    short* topMuteValues = nullptr;
    short* btmMuteValues = nullptr;

    bool*  badTraces  = new bool [m_ntr];
    for(int i=0; i< m_ntr; i++) badTraces[i]  = false;

    float* weight =  new float [m_ns];
    for(int i=0; i< m_ns; i++) weight[i] =1;

    int taperLength = m_sd2d->getMuteTaperLength();

    if(m_applyTopMute){
        topMuteValues = new short [m_ntr];
        memset((void*)topMuteValues, 0, m_ntr*sizeof(short));
        m_sd2d->getTracesMute(m_plotGathereType, m_gatherIdx, topMuteValues, 1);
    }
    if(m_applyBtmMute){
        btmMuteValues = new short [m_ntr];
        memset((void*)btmMuteValues, 0, m_ntr*sizeof(short));
        m_sd2d->getTracesMute(m_plotGathereType, m_gatherIdx, btmMuteValues, 2);
    }

    if(m_hideBadTraces){
        m_sd2d->getTraceFlags(m_plotGathereType, m_gatherIdx, badTraces);
    }
    //cout << "muteData = " << muteData << endl;

    float datavalue = 0;

    for (int xIndex=0; xIndex<m_ntr; xIndex++){

        if(m_applyTopMute || m_applyBtmMute) {
            int top = 0;
            int btm = m_ns;
            if(m_applyTopMute) top = topMuteValues[xIndex];
            if(m_applyBtmMute) btm = btmMuteValues[xIndex];
            //cout << " xIndex=" << xIndex << " topmute=" << top << " btmMute="<< btm << endl;
            m_sd2d->calculateTraceWeightValues(top, btm, weight, taperLength);
        }

        for (int yIndex=0; yIndex<m_ns; yIndex++){
            datavalue = m_gatherIn[xIndex][yIndex] * weight[yIndex];
            if(badTraces[xIndex]) datavalue = 0;
            outData[xIndex][yIndex] = datavalue;
        }
    }

    if(topMuteValues != nullptr) delete [] topMuteValues;
    if(btmMuteValues != nullptr) delete [] btmMuteValues;
    delete [] weight;
    delete [] badTraces;
}

bool Sdp2dMainGatherDisplayArea::mouseMoveOnTheGather(double key, double value)
{    
    int localTrIdx = int(key);
    if(localTrIdx>=1 && localTrIdx <m_ntr && value>=0 && value <m_plotMaxTime){
        SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack*>(m_mainWindow->getCurrentDataPointer());
        int gType = sd2d->getGatherType() ;
        int gIdx  = sd2d->getPlotGroupIndex();
        int tidx = sd2d->getTracesIndexInData(gType, gIdx, localTrIdx);
        float srcx = sd2d->getSrcXOfATrace(tidx);
        float srcy = sd2d->getSrcYOfATrace(tidx);
        float recx = sd2d->getRecXOfATrace(tidx);
        float recy = sd2d->getRecYOfATrace(tidx);
        float offs = sd2d->getOffsetOfATrace(tidx);
        float sdep = sd2d->getSDepthOfATrace(tidx);
        float sele = sd2d->getSElevOfATrace(tidx);
        float rele = sd2d->getRelevOfATrace(tidx);

        int sidx = sd2d->getSIdxOfATrace(tidx);
        int ridx = sd2d->getRIdxOfATrace(tidx);
        int cidx = sd2d->getCIdxOfATrace(tidx);
        int oidx = sd2d->getOIdxOfATrace(tidx);

        float data = m_colorMap->data()->data(key,value);
        float time = int(value*1000 + 0.5);
        time /= 1000.0;
        QString mainInfo = QString("Trace=%1, Time=%2s, Amplitude=%3, Offset=%4").arg(localTrIdx).arg(time).arg(data).arg(offs);
        QString idexInfo = QString("srcIdx=%1, recIdx=%2, cdpIdx=%3, offsetIdx=%4").arg(sidx).arg(ridx).arg(cidx).arg(oidx);
        QString srxyInfo = QString("sx=%1, sy=%2, rx=%3, ry=%4").arg(srcx).arg(srcy).arg(recx).arg(recy);
        QString elevInfo = QString("srcElev=%1, recElev=%2, srcDepth=%3").arg(sele).arg(rele).arg(sdep);
        QString sep = QString(", ");
        m_statusbar->showMessage(mainInfo + sep + idexInfo + sep + srxyInfo + sep + elevInfo);
        return true;
    }
    return false;
}

bool Sdp2dMainGatherDisplayArea::mouseMoveOnTheHeaderDisplay(QPoint pos)
{
    double x = pos.x();
    double y = pos.y();

    double offset = m_offsetAxisRect->axis(QCPAxis::atLeft)->pixelToCoord(y);
    if( offset < m_offRange->lower || offset > m_offRange->upper) return false;

    double elevation = m_offsetAxisRect->axis(QCPAxis::atRight)->pixelToCoord(y);
    if( elevation < m_eleRange->lower || elevation > m_eleRange->upper) return false;

    double traceIdx = m_offsetAxisRect->axis(QCPAxis::atTop)->pixelToCoord(x);
    QCPRange xaxis = m_offsetAxisRect->axis(QCPAxis::atTop)->range();
    if( traceIdx < xaxis.lower || traceIdx > xaxis.upper) return false;

    //cout << "offset=" << offset << " elevation=" << elevation << " traceIdx="<< traceIdx << " low=" << xaxis.lower << " up=" << xaxis.upper << endl;

    m_statusbar->showMessage(QString("Offset=%1, Elevation=%2").arg(offset).arg(elevation));
    return true;
}

void Sdp2dMainGatherDisplayArea::replotSelection(void)
{
    Sdp2dMainGatherDisplayPlot* myPlot = dynamic_cast<Sdp2dMainGatherDisplayPlot*>(m_customPlot);
    myPlot->replotSelection();
}


void Sdp2dMainGatherDisplayArea::setDataClipPercentage(int value)
{
    if(value == m_plotClipPercent) return;
    //cout <<"setDataClipPercentage value=" << value << " trace visible=" << m_customPlot->graph(0)->visible() << endl;

    m_plotClipPercent = value;

    setDataForColorDisplay();
    updateWiggleDisplayData();
    changeDisplayOfWiggleTraces();
    m_customPlot->replot();
}

int Sdp2dMainGatherDisplayArea::setGatherType(int gatherType, int minNtraces)
{
    //cout <<"gatherType=" << gatherType << " m_plotGathereType=" << m_plotGathereType << endl;
    if(gatherType == m_plotGathereType) return m_gatherIdx;

    m_plotGathereType = gatherType;

    removeHighLightedTrace();    

    return setPlotGroupToTheFirst(minNtraces);
}

void Sdp2dMainGatherDisplayArea::removeHighLightedTrace()
{
    if(m_wavefrom != nullptr)  {
        m_customPlot->removeGraph(m_wavefrom);
        m_customPlot->removeGraph(m_traceref);
        m_wavefrom = nullptr;
        m_traceref = nullptr;
        m_sd2d->hideTraceLocationPointOnAmpDock();
        m_customPlot->replot();
    }
    m_displayedtraceIdx = 0;
}

void Sdp2dMainGatherDisplayArea::cleanHighLightedtrace(int trIdx)
{
    if(m_displayedtraceIdx != trIdx) return;
    removeHighLightedTrace();
    m_sd2d->hideTraceLocationPointOnAmpDock();
}

void Sdp2dMainGatherDisplayArea::setDisplayType(int displayType)
{
    if(displayType == m_plotDisplayType) return;
    m_plotDisplayType = displayType;

    //QRect sel = m_customPlot->selectionRect()->rect();

    if(m_plotDisplayType == SeismicDisplayTpye::Wiggle) {
        m_colorMap->setVisible(false);
        m_colorScale->setVisible(false);
    } else {
        m_colorMap->setVisible(true);
        m_colorScale->setVisible(true);
    }

    //int keySize = m_colorMap->data()->keySize();
    if(m_plotDisplayType == SeismicDisplayTpye::Color){
        for(int trSeq=0; trSeq < m_ntr; trSeq++){
            if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(false);
        }
    }else{
        for(int trSeq=0; trSeq < m_ntr; trSeq++){
            if(m_badTraces != nullptr) {
                 if(m_badTraces[trSeq]) continue;
            }
            if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(true);
        }
    }

    m_customPlot->replot();
}


bool Sdp2dMainGatherDisplayArea::setPlotGroupIndex(int index)
{
    if(m_gatherIdx == index) return false;
    m_ntr = m_sd2d->getNumberOfTracesOfGather(m_plotGathereType, index);
    if(m_ntr <= 1) return false;

    m_gatherIdx = index;

    removeHighLightedTrace();

    displayOneGather();

    return true;
}

int Sdp2dMainGatherDisplayArea::setPlotGroupToTheFirst(int minNtraces)
{
    int gidx = m_sd2d->getGatherIndexPointer(m_plotGathereType)->minGroupValue;
    while( m_sd2d->getNumberOfTracesOfGather(m_plotGathereType, gidx) <= minNtraces ) gidx++;
    m_ntr = m_sd2d->getNumberOfTracesOfGather(m_plotGathereType, gidx);

    if(m_ntr <= 1) return 0;

    m_gatherIdx = gidx;

    displayOneGather();
    return m_gatherIdx;
}

int Sdp2dMainGatherDisplayArea::setPlotGroupToTheLast(int minNtraces)
{
    int gidx = m_sd2d->getGatherIndexPointer(m_plotGathereType)->maxGroupValue;
    while( m_sd2d->getNumberOfTracesOfGather(m_plotGathereType, gidx) <= minNtraces ) gidx--;
    m_ntr = m_sd2d->getNumberOfTracesOfGather(m_plotGathereType, gidx);
    if(m_ntr <= 1) return 0;
    m_gatherIdx = gidx;

    displayOneGather();
    return m_gatherIdx;
}

void Sdp2dMainGatherDisplayArea::setPlotWiggleScale(float val)
{   
    if(fabs(val - m_plotWiggleScale) < 0.001) return;
    m_plotWiggleScale = val;    
    updateWiggleTraces();
}

void Sdp2dMainGatherDisplayArea::setPlotXAxisType(int val)
{
    if(m_plotXAxisType == val) return;
    m_plotXAxisType = val;
    setHeaderDisplay();
    updateWiggleTraces();
    setMutePicksDisplay();
    //cout << "setPlotXAxisType = " << val << endl;
}

void Sdp2dMainGatherDisplayArea::setReversepolarity(int revPolarity)
{
    if(revPolarity == m_plotRevPolarity) return;
    //cout << "symRange = " << symRange << endl;
    m_plotRevPolarity = revPolarity;
    setDataForColorDisplay();
    updateWiggleTraces();
    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::updateWiggleTraces()
{
    int edge = m_plotWiggleScale;
    if(edge > 5) edge = 5;
    if(edge < 1) edge = 1;

    if(m_plotXAxisType == XAxisType::TraceIdx) {
        m_gatherAxisRect->axis(QCPAxis::atTop)->setRange(1-edge, m_ntr+edge);
        m_gatherAxisRect->axis(QCPAxis::atBottom)->setRange(1-edge, m_ntr+edge);
        if(m_displayTraceHeader){
            m_offsetAxisRect->axis(QCPAxis::atTop)->setRange(1-edge, m_ntr+edge);
            m_offsetAxisRect->axis(QCPAxis::atBottom)->setRange(1-edge, m_ntr+edge);
        }
    } else if(m_plotXAxisType == XAxisType::OffsetVal) {
        double lower = m_offRange->lower - edge*m_offEdge;
        double upper = m_offRange->upper + edge*m_offEdge;
        m_gatherAxisRect->axis(QCPAxis::atTop)->setRange(lower, upper);
        m_gatherAxisRect->axis(QCPAxis::atBottom)->setRange(lower, upper);
        if(m_displayTraceHeader){
            m_offsetAxisRect->axis(QCPAxis::atTop)->setRange(lower, upper);
            m_offsetAxisRect->axis(QCPAxis::atBottom)->setRange(lower, upper);
        }
    }

    updateWiggleDisplayData();
    changeDisplayOfWiggleTraces();
    m_customPlot->replot();
}

int Sdp2dMainGatherDisplayArea::getInteractiveFunction()
{
    return m_sd2d->getInteractiveFunction();
}


void Sdp2dMainGatherDisplayArea::cleanDisplay(bool funChanged)
{
    Sdp2dMainGatherDisplayPlot* myPlot = dynamic_cast<Sdp2dMainGatherDisplayPlot*>(m_customPlot);
    myPlot->displayedGatherChanged();

    m_displayedtraceIdx = 0;
    cleanTempGraphs();
    cleanLinearVelocityLine();

    if(m_plotDisplayType != SeismicDisplayTpye::Color){
        for(int trSeq=0; trSeq < m_ntr; trSeq++){
           if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(true);
        }
    }
    if(funChanged)  removeHighLightedTrace();

    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::setTraceVisible(bool visible)
{   
    if(m_wavefrom != nullptr) m_wavefrom->setVisible(visible);    
}


bool Sdp2dMainGatherDisplayArea::isSelectedRectChanged(QRect selectedRect)
{
    QRect lastRect = m_sd2d->getSelectedRect();

    //cout << " new top=" << selectedRect.top() << " btm=" << selectedRect.bottom()  << " lft=" << selectedRect.left()  << " rht=" << selectedRect.right() << endl;
    //cout << " old top=" << lastRect.top() << " btm=" << lastRect.bottom()  << " lft=" << lastRect.left()  << " rht=" << lastRect.right() << endl;

    if(selectedRect == lastRect) return false;
    return true;
}

void Sdp2dMainGatherDisplayArea::frequencyAnalysis(QRect selectRect, QString& label)
{    
    int x1 = selectRect.left() - 1;
    int x2 = selectRect.right() - 1;
    int nx = x2 - x1 +1;

    float* weight = new float [m_ns];
    m_sd2d->calculateTraceWeightValues(selectRect.top(), selectRect.bottom(), weight);

    float** data = Sdp2dUtils::alloc2float(m_ns, nx);
    memset((void*)data[0], 0, m_ns*nx*sizeof(float));

    for(int ix = 0; ix < nx; ix++){
        for(int it = 0; it < m_ns; it++){
            data[ix][it] =  m_gatherIn[ix+x1][it] * weight[it];
        }
    }
    m_sd2d->frequencyAnalysis(selectRect, data, label);
    Sdp2dUtils::free2float(data);
    delete [] weight;
}

float *Sdp2dMainGatherDisplayArea::getDataOfASeismicTrace(int tIdx)
{
    m_sd2d->getOneTraceWithTypeAndIndex(m_plotGathereType, m_gatherIdx, tIdx, m_traceIn);
    return m_traceIn;
}

void Sdp2dMainGatherDisplayArea::mapSingleTraceOnAmpDisplay(int trIdx)
{
    Sdp2dAmplitudeDisplayDock* p =m_sd2d->getAmplitudeDockPointer();
    if(p != nullptr) {
        int traceIdx = m_sd2d->getTraceIndexWithinWholeDataset(m_plotGathereType, m_gatherIdx, trIdx);
        p->setDataForCurrentTracePoint(traceIdx);
    }
}

void Sdp2dMainGatherDisplayArea::drawAndMapSingleWiggleTrace(int tIdx)
{
    m_displayedtraceIdx = tIdx;

    drawTheHighLightedTrace(m_colorFreTrace);

    mapSingleTraceOnAmpDisplay(tIdx);
    m_customPlot->replot();
}


void Sdp2dMainGatherDisplayArea::setGatherDisplayWithHelightTrace(int gatherType, int gatherIdx, int traceIdx)
{
    m_plotGathereType = gatherType;
    m_gatherIdx = gatherIdx;
    m_displayedtraceIdx = traceIdx;    

    displayOneGather();    
    drawTheHighLightedTrace(m_colorSelTrace);


    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::setGatherDisplayWithHelightTrace(int gatherType, int traceIdx)
{
    if(gatherType == m_plotGathereType) return;
    int trGlobalIdx = m_sd2d->getTraceIndexWithinWholeDataset(m_plotGathereType, m_gatherIdx, traceIdx);

    int* idxPointer = nullptr;
    switch (gatherType) {
      case DisplayGatherType::CommonShot:
          idxPointer = m_sd2d->getShotIdPointer();
          break;
      case DisplayGatherType::CommonDepthPoint:
          idxPointer = m_sd2d->getCDPsIdPointer();
          break;
      case DisplayGatherType::CommonReceiver:
          idxPointer = m_sd2d->getRecvIdPointer();
          break;
      case DisplayGatherType::CommonOffset:
          idxPointer = m_sd2d->getOffsIdPointer();
          m_sd2d->setXAxisType(1);
          break;
    }

    int gIdx = idxPointer[trGlobalIdx];
    int tIdx  = m_sd2d->findTraceSequenceWithinGather(gatherType, gIdx, trGlobalIdx);
    m_gatherIdx = gIdx;
    m_plotGathereType = gatherType;
    m_displayedtraceIdx = tIdx;

    displayOneGather();
    drawTheHighLightedTrace(m_colorSelTrace);

    m_sd2d->updateMapAndFreqAnaWithGatherTrace(m_displayedtraceIdx);

    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::drawTheHighLightedTrace(QColor col)
{
    if(m_wavefrom != nullptr)  {
        m_customPlot->removeGraph(m_wavefrom);
        m_customPlot->removeGraph(m_traceref);
    }

    m_wavefrom = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
    m_traceref = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
    drawOneWiggleTrace(m_wavefrom, m_traceref, m_displayedtraceIdx-1, col);
    if(m_badTraces != nullptr) {
        if(m_badTraces[m_displayedtraceIdx-1]) m_wavefrom->setVisible(false);
    }    
    Sdp2dMainGatherDisplayPlot* myPlot = dynamic_cast<Sdp2dMainGatherDisplayPlot*>(m_customPlot);
    myPlot->setSelectedRectForOnetrace(m_displayedtraceIdx);
}

int  Sdp2dMainGatherDisplayArea::getFrequencyAnaType(void)
{
    return m_sd2d->getFrequencyAnaType();
}

void Sdp2dMainGatherDisplayArea::setFrequencyAnaType(int val)
{
    m_sd2d->setFrequencyAnaType(val);
}

void Sdp2dMainGatherDisplayArea::changeDisplayOfWiggleTraces(void)
{    
    setDisplayedBadTraces();
    if(m_wavefrom != nullptr && m_displayedtraceIdx > 0) {
        if(m_badTraces != nullptr){
            if(!m_badTraces[m_displayedtraceIdx-1])
                setDataForDisplayedSingleTrace(m_wavefrom, m_traceref, m_displayedtraceIdx);
        }else {
            setDataForDisplayedSingleTrace(m_wavefrom, m_traceref, m_displayedtraceIdx);
        }
    }
    displayProcessedSeismicTraces();
}

void Sdp2dMainGatherDisplayArea::setSingleBadTrace(int trSeq)
{
    if(m_badTraces[trSeq]){
        m_badTrcGraphs[trSeq]->setVisible(false);
        m_badTraces[trSeq] = false;
        if(m_plotDisplayType != SeismicDisplayTpye::Color){
            if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(true);
        }

    } else{
        if(m_badTrcGraphs[trSeq] != nullptr){
            m_badTrcGraphs[trSeq]->setVisible(true);
        } else {
            QCPGraph* wav = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
            QCPGraph* ref = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
            drawOneWiggleTrace(wav, ref, trSeq, m_colorBadTrace);
            m_badTrcGraphs[trSeq] = wav;
            m_badRefGraphs[trSeq] = ref;
        }
        m_badTraces[trSeq] = true;
        if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(false);
    }
    m_sd2d->setTraceFlags(m_plotGathereType, m_gatherIdx, m_badTraces);

    if(m_displayedtraceIdx == trSeq+1){
        if(m_wavefrom != nullptr) m_wavefrom->setVisible(!m_badTraces[trSeq]);
    }

    mapSingleTraceOnAmpDisplay(trSeq+1);
    m_customPlot->replot();    
}

void Sdp2dMainGatherDisplayArea::setMultipelBadTraces(int min, int start, int end, int max)
{
    for(int trSeq=min; trSeq<=max;trSeq++){
        if(trSeq < start || trSeq > end) {
            if(m_badTraces[trSeq] ){
                m_badTrcGraphs[trSeq]->setVisible(false);
                m_badTraces[trSeq] = false;                
            }
            if(m_plotDisplayType != SeismicDisplayTpye::Color){
                if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(true);
            }
        }else{
            if(!m_badTraces[trSeq]){
                if(m_badTrcGraphs[trSeq] != nullptr){
                    m_badTrcGraphs[trSeq]->setVisible(true);
                } else {
                    QCPGraph* wav = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
                    QCPGraph* ref = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
                    drawOneWiggleTrace(wav, ref, trSeq, m_colorBadTrace);
                    m_badTrcGraphs[trSeq] = wav;
                    m_badRefGraphs[trSeq] = ref;
                }
                m_badTraces[trSeq] = true;   // set trace as bad trace                
            }
            if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(false);
        }
        if(m_displayedtraceIdx == trSeq+1){
            if(m_wavefrom != nullptr) {
                m_wavefrom->setVisible(!m_badTraces[trSeq]);
            }
        }
    }
    mapSingleTraceOnAmpDisplay(start+1);
    m_sd2d->setTraceFlags(m_plotGathereType, m_gatherIdx, m_badTraces);
    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::drawOneWiggleTrace(QCPGraph* wavtr, QCPGraph* reftr, int trSeq, QColor color)
{    
    wavtr->setPen(QPen(color));
    wavtr->setBrush(QBrush(color));
    wavtr->setChannelFillGraph(reftr);
    wavtr->antialiasedFill();
    reftr->setPen(QPen(color));

    wavtr->setVisible(true);
    reftr->setVisible(false);

    setDataForDisplayedSingleTrace(wavtr, reftr, trSeq+1);
}

void Sdp2dMainGatherDisplayArea::setDisplayedBadTraces(void)
{
    if(m_badTraces == nullptr) return;

    for(int trSeq=0; trSeq<m_badTrcGraphs.size(); trSeq++){
        if(!m_badTraces[trSeq]) continue;        
        QCPGraph* wav = m_badTrcGraphs[trSeq];
        QCPGraph* ref = m_badRefGraphs[trSeq];
        setDataForDisplayedSingleTrace(wav, ref, trSeq+1);
        if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(false);
    }
}

QRect Sdp2dMainGatherDisplayArea::getSelectedRect()
{
    int staX = 1;
    int endX = m_ntr;
    int staY = 0;
    int endY = m_sd2d->getSamplesPerTraces() - 1;

    QPoint start(staX, staY);
    QPoint end(endX, endY);
    QRect newRect = QRect(start, end);

    int freqanaType = m_sd2d->getFrequencyAnaType();
    if(freqanaType == FreqAnaType::SingleTrace) {
        if(m_displayedtraceIdx <1 ){
            newRect = QRect(QPoint(0,0), QPoint(0,0));
        }else {
            start.setX(m_displayedtraceIdx);
            end.setX(m_displayedtraceIdx);
            newRect = QRect(start, end);
        }
    } else if(freqanaType == FreqAnaType::RectArea) {
        Sdp2dMainGatherDisplayPlot* myPlot = dynamic_cast<Sdp2dMainGatherDisplayPlot*>(m_customPlot);
        newRect = myPlot->getSelectedRect();
    } else if(freqanaType == FreqAnaType::NoSelection) {
        if(m_displayedtraceIdx >0) {
            start.setX(m_displayedtraceIdx);
            end.setX(m_displayedtraceIdx);
            newRect = QRect(start, end);
        } else{
            newRect = QRect(QPoint(0,0), QPoint(0,0));
        }
    }

    //cout << "Area of Selection: time1=" << newRect.top() << " time2=" << newRect.bottom() <<
    //        " trace1="<< newRect.left() << " trace2="<< newRect.right() << endl;
    return newRect;
}


void Sdp2dMainGatherDisplayArea::cleanTempGraphs(void)
{
    //cout << "Number of temp graphs : " << m_tmpTracesGraphs.count() << endl;
    if(m_tmpTracesGraphs.count() >0){
        for(int i = m_tmpTracesGraphs.count() - 1; i>=0; i--){
            m_customPlot->removeGraph(m_tmpTracesGraphs[i]);
        }
        m_tmpTracesGraphs.clear();        
        m_sd2d->setFreqComparisonFlag(false);
        Sdp2dMainGatherDisplayPlot* myPlot = dynamic_cast<Sdp2dMainGatherDisplayPlot*>(m_customPlot);
        myPlot->disableHideGather(true);
        m_customPlot->replot();
    }
}

bool Sdp2dMainGatherDisplayArea::hasTempGraphs(void)
{
    if(m_tmpTracesGraphs.count() >0)  return true;
    return false;
}


void Sdp2dMainGatherDisplayArea::showTempGraphs(bool show)
{
    if(m_tmpTracesGraphs.count() < 1) return;
    for(int i = 0; i< m_tmpTracesGraphs.count(); i=i+2){
        m_tmpTracesGraphs[i]->setVisible(show);
    }
    m_customPlot->replot();
}

bool Sdp2dMainGatherDisplayArea::isTempGraphsVislible(void)
{
    if(m_tmpTracesGraphs.count() < 1) return false;
    return m_tmpTracesGraphs[0]->visible();
}

void Sdp2dMainGatherDisplayArea::displayProcessedSeismicTraces(QRect selectedRect, float** data)
{
    if(selectedRect.height() < 15) return;

    if(m_gatherOut != nullptr) Sdp2dUtils::free2float(m_gatherOut);
    m_gatherOut = Sdp2dUtils::alloc2float(m_sd2d->getSamplesPerTraces(), m_ntr);

    int x1 = selectedRect.left();
    int x2 = selectedRect.right();
    for(int ix = x1, x=0; ix <=x2; ix++, x++){
        memcpy((void*)m_gatherOut[ix-1], (void*)data[x], m_ns*sizeof(float));
    }
    displayProcessedSeismicTraces(data, selectedRect);
    //cout << "nx=" << nx << " graphNum=" << m_tmpTracesGraphs.count() << endl;
}

void Sdp2dMainGatherDisplayArea::displayProcessedSeismicTraces(void)
{
    if(m_gatherOut == nullptr) return;
    if(m_tmpTracesGraphs.count() < 1) return;

    QRect selectedRect = getSelectedRect();
    if(selectedRect.height() < 15) return;

    int x1 = selectedRect.left() - 1;

    displayProcessedSeismicTraces(&m_gatherOut[x1], selectedRect);
}

/*
void Sdp2dMainGatherDisplayArea::displayProcessedSeismicTraces(QRect selectedRect)
{    
    if(m_gatherOut == nullptr) return;
    if(selectedRect.height() < 15) return;
    int x1 = selectedRect.left() - 1;
    //cout << "selectedRect.height() = " << selectedRect.height() << " x1 = " << x1 << endl;
    displayProcessedSeismicTraces(&m_gatherOut[x1], selectedRect);

}
*/

void Sdp2dMainGatherDisplayArea::displayProcessedSeismicTraces(float **data, QRect selectedRect)
{
    cleanTempGraphs();

    int x1 = selectedRect.left();
    int x2 = selectedRect.right();
    //int nx = x2 - x1 + 1;
    int t1 = selectedRect.top();
    int t2 = selectedRect.bottom();
    //t1 = 0;
    //t2 = m_ns - 1;
    int nt = t2 - t1 + 1;

    //cout << "in displayProcessedSeismictraces nx=" << nx<<  " nt=" << nt << " t1=" << t1<<  " t2=" << t2 << " x1=" << x1<<  " x2=" << x2<< endl;

    QVector<double> time;
    QVector<double> tval;
    QVector<double> tref;
    time.resize(nt);
    tval.resize(nt);
    tref.resize(nt);

    for(int it=t1; it<=t2; it++){
        int t = it-t1;
        time[t] = it * m_dt;
    }

    QCPRange drange = m_colorMap->dataRange();
    drange.normalize();
    double normalizeFactor = abs(drange.upper);

    for(int ix = x1, x=0; ix <=x2; ix++, x++){
        QCPGraph* wav = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
        QCPGraph* ref = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atLeft), m_gatherAxisRect->axis(QCPAxis::atTop));
        wav->setPen(QPen(m_colorTmpCurve));
        wav->setBrush(QBrush(m_colorTmpCurve));
        wav->setChannelFillGraph(ref);
        wav->antialiasedFill();
        ref->setPen(QPen(Qt::black));
        ref->setVisible(false);

        double refValue = ix;
        double scale = m_plotWiggleScale/normalizeFactor;

        if(m_plotXAxisType == 2){
            refValue = m_offset[ix-1];
            scale = scale * m_offEdge;
        }

        for(int it=t1, t=0; it<=t2; it++, t++){
            double val = data[x][it];
            if(val > normalizeFactor ) val = normalizeFactor;
            else if(val < -normalizeFactor ) val = -normalizeFactor;

            if(m_plotRevPolarity) val *= -1.0;
            val = val*scale + refValue;
            tval[t] = val;
            if(val < refValue){
                tref[t] = val;
            } else {
                tref[t] = refValue;
            }
        }
        wav->setData(time, tval);
        ref->setData(time, tref);
        m_tmpTracesGraphs.append(wav);
        m_tmpTracesGraphs.append(ref);
    }

    time.clear();
    tval.clear();
    tref.clear();
    Sdp2dMainGatherDisplayPlot* myPlot = dynamic_cast<Sdp2dMainGatherDisplayPlot*>(m_customPlot);
    myPlot->replotSelection();
    myPlot->disableHideGather(false);
    //m_customPlot->enableSaveActionForFilter(true);
}

void Sdp2dMainGatherDisplayArea::disableHideGather(bool disable)
{
    Sdp2dMainGatherDisplayPlot* myPlot = dynamic_cast<Sdp2dMainGatherDisplayPlot*>(m_customPlot);
    myPlot->disableHideGather(disable);
    removeHighLightedTrace();
}

void Sdp2dMainGatherDisplayArea::replaceWithFilteredData(void)
{
    //cout << "in replaceWithFilteredData" << endl;
    if(m_gatherOut == nullptr) return;
    QRect rect = m_sd2d->getSelectedRect();
    //cout << "nt = " << rect.height() << " t1=" << rect.top() << " t2="<< rect.bottom() << endl;
    if(rect.height() < 15) return;

    int x1 = rect.left();
    int x2 = rect.right();
    int nt = rect.height();

    Sdp2dSegy* segy = m_sd2d->getSegyhandle();

    float* data   = new float [m_ns];
    float* weight = new float [m_ns];
    m_sd2d->calculateTraceWeightValues(rect.top(), rect.bottom(), weight);

    for(int ix = x1; ix <=x2; ix++){
        int trGlobalIdx = m_sd2d->getTraceIndexWithinWholeDataset(m_plotGathereType, m_gatherIdx, ix);

        if(nt < m_ns ){
            //cout << "replace trace " << ix << " nt=" << nt << " tidx=" << trGlobalIdx << endl;
            segy->getTraceDataFromIntermFile(trGlobalIdx, data);            
            // weight was already applied on m_gatherOut
            for(int it = 0; it < nt; it++) data[it] = m_gatherOut[ix-1][it] + data[it] * (1 - weight[it]);
            segy->writeTraceDataToIntermFile(trGlobalIdx, data);
            for(int it = 0; it < m_ns; it++) m_gatherIn[ix-1][it] = data[it];
        } else {
            segy->writeTraceDataToIntermFile(trGlobalIdx, m_gatherOut[ix-1]);
            for(int it = 0; it < m_ns; it++) m_gatherIn[ix-1][it] = m_gatherOut[ix-1][it];
        }
    }

    delete [] data;
    delete [] weight;

    setDataForColorDisplay();
    updateWiggleDisplayData();
    cleanTempGraphs();

    Sdp2dMainGatherDisplayPlot* myPlot = dynamic_cast<Sdp2dMainGatherDisplayPlot*>(m_customPlot);
    myPlot->enableSaveActionForFilter(false);
    myPlot->replotSelection();
}

void Sdp2dMainGatherDisplayArea::restoreRawData(void)
{
    //cout << "in restoreRawData" << endl;
    if(m_gatherOut == nullptr) return;

    QRect rect = m_sd2d->getSelectedRect();
    //cout << "nt = " << rect.height() << endl;
    if(rect.height() < 15) return;

    int x1 = rect.left();
    int x2 = rect.right();

    Sdp2dSegy* segy = m_sd2d->getSegyhandle();

    for(int ix = x1; ix <=x2; ix++){
        int trGlobalIdx = m_sd2d->getTraceIndexWithinWholeDataset(m_plotGathereType, m_gatherIdx, ix);
        segy->getTraceData(trGlobalIdx, m_gatherOut[ix-1]);
        segy->writeTraceDataToIntermFile(trGlobalIdx, m_gatherOut[ix-1]);
        for(int it = 0; it < m_ns; it++) m_gatherIn[ix-1][it] = m_gatherOut[ix-1][it];
    }
    setDataForColorDisplay();
    updateWiggleDisplayData();
    cleanTempGraphs();    

    Sdp2dMainGatherDisplayPlot* myPlot = dynamic_cast<Sdp2dMainGatherDisplayPlot*>(m_customPlot);
    myPlot->enableSaveActionForFilter(true);
    myPlot->replotSelection();
}


void Sdp2dMainGatherDisplayArea::setTopMute(bool checked)
{
    if(checked) m_sd2d->setTopMute();
}

void Sdp2dMainGatherDisplayArea::setBtmMute(bool checked)
{
    if(checked) m_sd2d->setBtmMute();
}

void Sdp2dMainGatherDisplayArea::applyTopMuteOnData(bool checked)
{
    m_applyTopMute = checked;
    updataGatherDisplay();
}

void Sdp2dMainGatherDisplayArea::applyBtmMuteOnData(bool checked)
{
    m_applyBtmMute = checked;
    updataGatherDisplay();
}

void Sdp2dMainGatherDisplayArea::hideBadTraces(bool checked)
{
    m_hideBadTraces = checked;
    updataGatherDisplay();
}

void Sdp2dMainGatherDisplayArea::updataGatherDisplay(void)
{
    setDataForColorDisplay();
    updateWiggleDisplayData();
    changeDisplayOfWiggleTraces();
    m_customPlot->replot();
}


void Sdp2dMainGatherDisplayArea::addOneMutePickPoint(QPoint pos)
{
    QPointF tt = convertPosToAxisValues(QPointF(pos.x(), pos.y()));
    int trIdx = int(tt.x());    // trIdx = trSeq +1;
    if(trIdx < 1) trIdx = 1;
    if(trIdx > m_ntr) trIdx = m_ntr;
    m_sd2d->addOneMutePickPoint(trIdx, tt.y());
    setMutePicksDisplay();
}

void Sdp2dMainGatherDisplayArea::removeOneMutePickPoint(QPoint pos)
{
    QPointF tt = convertPosToAxisValues(QPointF(pos.x(), pos.y()));
    int trIdx = int(tt.x());    // trIdx = trSeq +1;
    if(trIdx < 1) trIdx = 1;
    if(trIdx > m_ntr) trIdx = m_ntr;
    m_sd2d->removeOneMutePickPoint(trIdx);
    setMutePicksDisplay();
}

void Sdp2dMainGatherDisplayArea::displayMutePicks(QList<IdxVal>& left, QList<IdxVal>& middle, QList<IdxVal>& right)
{
    //cout << "nleft=" << left.count() << " nmiddle=" << middle.count() << " nright="<<right.count() << endl;
    if(m_midMuteGraph == nullptr) {
        QPen myPen;
        myPen.setColor(QColor(Qt::blue));
        myPen.setStyle(Qt::DashLine);
        myPen.setWidthF(3);
        m_lftMuteGraph = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atTop), m_gatherAxisRect->axis(QCPAxis::atLeft));
        m_lftMuteGraph->setPen(myPen);
        m_lftMuteGraph->setLineStyle(QCPGraph::lsLine);
        m_lftMuteGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,  7));

        myPen.setColor(QColor(255, 200, 20, 200));
        m_rhtMuteGraph = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atTop), m_gatherAxisRect->axis(QCPAxis::atLeft));
        m_rhtMuteGraph->setLineStyle(QCPGraph::lsLine);
        m_rhtMuteGraph->setPen(myPen);
        m_rhtMuteGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));

        myPen.setColor(QColor(Qt::red));
        myPen.setStyle(Qt::SolidLine);
        m_midMuteGraph = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atTop), m_gatherAxisRect->axis(QCPAxis::atLeft));
        m_midMuteGraph->setLineStyle(QCPGraph::lsLine);
        m_midMuteGraph->setPen(myPen);
        m_midMuteGraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 7));
    }

    QVector<double> key;
    QVector<double> val;
    for(int i=0; i< middle.size(); i++){
        double time = middle.at(i).time;
        double tidx = middle.at(i).trIdx+1;
        //cout << "middle No. "<<i << " time=" << time << " tidx=" << tidx << endl;
        if(m_plotXAxisType == 2){
            int idx = middle.at(i).trIdx;
            if(idx > m_ntr-1) {
                //cout << " mute picks middle: i="<< i << " tridx="<<middle.at(i).trIdx << " idx="<< idx  << " m_ntr="<< m_ntr << endl;
                continue;
            }
            tidx = m_offset[idx];

        }
        key.append(tidx);
        val.append(time);
    }   
    m_midMuteGraph->setData(key, val);

    key.clear();
    val.clear();
    for(int i=0; i< left.size(); i++){
        double time = left.at(i).time;
        double tidx = left.at(i).trIdx+1;
        //cout << "left No. "<<i << " time=" << time << " tidx=" << tidx << endl;
        if(m_plotXAxisType == 2){
            int idx = left.at(i).trIdx;
            if(idx > m_ntr-1){
                //cout << " mute picks left: i="<< i << " tridx="<<left.at(i).trIdx << " idx="<< idx  << " m_ntr="<< m_ntr << endl;
                continue;
            }
            tidx = m_offset[idx];
        }
        key.append(tidx);
        val.append(time);
    }    
    m_lftMuteGraph->setData(key, val);

    key.clear();
    val.clear();
    for(int i=0; i< right.size(); i++){
        double time = right.at(i).time;
        double tidx = right.at(i).trIdx+1;
        //cout << "right No. "<<i << " time=" << time << " tidx=" << tidx << endl;
        if(m_plotXAxisType == 2){
            int idx = right.at(i).trIdx;
            if(idx > m_ntr-1){
                //cout << " mute picks right: i="<< i << " tridx="<<right.at(i).trIdx << " idx="<< idx  << " m_ntr="<< m_ntr << endl;
                continue;
            }
            tidx = m_offset[idx];
        }
        key.append(tidx);
        val.append(time);
    }
    m_rhtMuteGraph->setData(key, val);

    //m_customPlot->replot(QCustomPlot::rpImmediateRefresh);
    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::setMutePicksVisible(bool visible)
{
    if(m_midMuteGraph == nullptr) return;

    m_midMuteGraph->setVisible(visible);
    m_lftMuteGraph->setVisible(visible);
    m_rhtMuteGraph->setVisible(visible);

    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::checkMeasureLinearVelocity(bool checked)
{
    //cout << "checkMeasureLinearVelocity = " << checked << endl;
    if(checked) {
        createLinearVelocityMeasureElements();
        m_sd2d->setInteractiveFunction(InteractiveFunctions::MeasureLinearVelcity);
    }else {
        cleanLinearVelocityLine();
        m_sd2d->setInteractiveFunction(InteractiveFunctions::None);
    }
    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::drawLinearVelMeasureFirstPoint(QPoint pos)
{
    if(m_plotGathereType == DisplayGatherType::CommonOffset) return;
    createLinearVelocityMeasureElements();

    QPointF tt = convertPosToAxisValues(QPointF(pos.x(), pos.y()));
    int trIdx = int(tt.x());    // trIdx = trSeq +1;
    if(trIdx < 1) trIdx = 1;
    if(trIdx > m_ntr) trIdx = m_ntr;

    QVector<double> key;
    QVector<double> val;
    if(m_plotXAxisType == 2) {
        key.append(m_offset[trIdx-1]);
    }else{
        key.append(trIdx);
    }

    val.append(tt.y());
    //cout << "key=" << key[0] << " val=" << val[0] << endl;
    m_linearVelLine->setData(key, val);
    m_linearVelLable->setVisible(false);
    m_customPlot->replot();
}

void Sdp2dMainGatherDisplayArea::drawLinearVelMeasureSecondPoint(QPoint pos)
{
    if(m_plotGathereType == DisplayGatherType::CommonOffset) return;
    if(m_linearVelLine == nullptr) return;
    if(m_linearVelLine->dataCount() == 0) return;

    //QPoint globalPos = this->mapToGlobal(pos);

    double key1 = m_linearVelLine->dataMainKey(0);
    double val1 = m_linearVelLine->dataMainValue(0);

    QPointF tt = convertPosToAxisValues(QPointF(pos.x(), pos.y()));
    int trIdx = int(tt.x());    // trIdx = trSeq +1;
    if(trIdx < 1) trIdx = 1;
    if(trIdx > m_ntr) trIdx = m_ntr;

    double key2 = trIdx;
    double val2 = tt.y();
    if(m_plotXAxisType == 2) key2 = m_offset[trIdx-1];

    if(m_linearVelLine->dataCount() == 1) {
        m_linearVelLine->addData(key2, val2);
    } else {
        QVector<double> qvkey;
        QVector<double> qvval;
        qvkey.append(key1);
        qvval.append(val1);
        qvkey.append(key2);
        qvval.append(val2);
        m_linearVelLine->setData(qvkey, qvval, true);

    }

    // calculate velocity
    if(m_plotXAxisType == 1){
        int idx1 = key1 - 1;
        key1 = m_offset[idx1];
        int idx2 = key2 - 1;
        key2 = m_offset[idx2];
    }
    double vel = 0;
    if(abs(val2-val1) > 0.000001){
        vel = (key2 -key1)/(val2-val1);
    }

    m_linearVelLable->position->setCoords(pos.x()+10, pos.y()-10);
    m_linearVelLable->setText(QString("%1m/s").arg(abs(vel)));
    m_linearVelLable->setVisible(true);
    //cout << "x = " << m_linearVelLable->topLeft->pixelPosition().x() << " y=" << m_linearVelLable->topLeft->pixelPosition().y() << endl;
    m_customPlot->replot();
}


void Sdp2dMainGatherDisplayArea::createLinearVelocityMeasureElements()
{
    cleanLinearVelocityLine();

    QPen myPen;
    myPen.setColor(QColor(Qt::blue));
    myPen.setStyle(Qt::DashLine);
    myPen.setWidthF(3);
    m_linearVelLine = new QCPGraph(m_gatherAxisRect->axis(QCPAxis::atTop), m_gatherAxisRect->axis(QCPAxis::atLeft));
    m_linearVelLine->setPen(myPen);
    m_linearVelLine->setLineStyle(QCPGraph::lsLine);
    m_linearVelLine->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,  7));

    m_linearVelLable = new QCPItemText(m_customPlot);
    m_linearVelLable->setPen(QPen(Qt::black));
    m_linearVelLable->setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
    m_linearVelLable->position->setType(QCPItemPosition::ptAbsolute);
    m_linearVelLable->setColor(QColor(Qt::blue));
    m_linearVelLable->setBrush(QBrush(Qt::white));
    m_linearVelLable->setFont(QFont(font().family(), 20));
    m_linearVelLable->setVisible(false);

}

void Sdp2dMainGatherDisplayArea::cleanLinearVelocityLine(void)
{
    if(m_linearVelLine != nullptr){
        m_customPlot->removeGraph(m_linearVelLine);
        m_customPlot->removeItem(m_linearVelLable);
        m_linearVelLine = nullptr;
        m_linearVelLable = nullptr;
    }
}

float** Sdp2dMainGatherDisplayArea::getDataOfOutputSeismicGather(void)
{
    if(m_gatherOut != nullptr) Sdp2dUtils::free2float(m_gatherOut);
    m_gatherOut = Sdp2dUtils::alloc2float(m_sd2d->getSamplesPerTraces(), m_ntr);
    return m_gatherOut;
}
