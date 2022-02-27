#include "sdp2dProcessedGatherDisplayArea.h"
#include "sdp2dMainGatherDisplayArea.h"
#include "seismicdata2dprestack.h"
#include "seismicdata2d.h"

#include "seismicdataprocessing2d.h"
#include "sdp2dUtils.h"

Sdp2dProcessedGatherDisplayPlot::Sdp2dProcessedGatherDisplayPlot(Sdp2dProcessedGatherDisplayArea *parent)
    : QCustomPlot(parent)
{
    m_parent = parent;
    m_statusBar = m_parent->getStatusBar();
}

Sdp2dProcessedGatherDisplayPlot::~Sdp2dProcessedGatherDisplayPlot()
{}

void Sdp2dProcessedGatherDisplayPlot::mouseMoveEvent(QMouseEvent *event)
{
    QPointF tt = m_parent->convertPosToAxisValues(QPointF(event->pos().x(), event->pos().y()));
    int ntr = m_parent->getNumTracesOfCurrentGather();
    int ns = m_parent->getTimeSamplesOfCurrentView();
    float dt = m_parent->getTimeSampRateOfCurrentGather();
    int ix = tt.x() -1;
    int it = tt.y()/dt;
    if(ix<0 || ix >= ntr ||it<0 || it >= ns) {
        m_statusBar->clearMessage();
    } else {
        //cout << "posx="<< tt.x() << " posy="<< tt.y() << endl;
        float** data = m_parent->getDataOfInputSeismicGather();
        float val = data[ix][it];
        float time = int(tt.y()*1000 + 0.5);
        time /= 1000.0;
        m_statusBar->showMessage(QString("Trace: %1, Time: %2s, Amplitude: %3").arg(int(tt.x())).arg(time).arg(val));
    }

}


Sdp2dProcessedGatherDisplayArea::Sdp2dProcessedGatherDisplayArea(SeismicData2D* sd2d, QWidget *parent)
    : Sdp2dGatherDisplayArea(sd2d, parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    m_sd2d = dynamic_cast<SeismicData2DPreStack*>(sd2d);
    m_sd2d->setProcessedGatherDisplayPointer(this);
    m_sd2d->addDisplayWidgets(this);

    m_mainGather = m_sd2d->getInputGatherDisplayPointer();
    m_displayTraceHeader = m_mainGather->getTraceHeaderDisplay();

    m_customPlot = new Sdp2dProcessedGatherDisplayPlot(this);
    setWidget(m_customPlot);

    createGatherDisplayAxisRect();

    createGatherColorDisplayBase();

    createOffsetDisplayAxisRect();

    setupCustomPlot();

    m_myMenu = new QMenu(this);
    //m_myMenu->addAction("Close this window", m_mainGather,  &SeismicDataProcessing2D::disableGatherProcess);
    QAction* closeWin = m_myMenu->addAction("Close this window");
    connect(closeWin, SIGNAL(triggered()), m_mainWindow, SLOT(disableGatherProcess()));

    connect(m_colorMap, SIGNAL(dataRangeChanged(const QCPRange&)), this, SLOT(displayedDataChanged(const QCPRange&)));

    m_customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_customPlot, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onCustomContextMenuRequested(const QPoint&)));
}

Sdp2dProcessedGatherDisplayArea::~Sdp2dProcessedGatherDisplayArea()
{

}


void Sdp2dProcessedGatherDisplayArea::onCustomContextMenuRequested(const QPoint &pos)
{
    QPoint globalPos = this->mapToGlobal(pos);
    m_myMenu->exec(globalPos);
}

void Sdp2dProcessedGatherDisplayArea::displayedDataChanged(const QCPRange& range)
{
    // The group index in the sd2d has not changed till now.
    Q_UNUSED(range);
    //cout << " range: lower=" << range.lower << " upper=" << range.upper   << endl;

    if(m_mainWindow->isGatherDisplayFitToScrollWindow()) {
        //m_customPlot->resize(this->width()-20,this->height()-20);
        m_customPlot->resize(this->width(),this->height());
    } else {
        m_xscale = m_mainGather->getPlotXScale();
        m_yscale = m_mainGather->getPlotYScale();
        m_plotWiggleScale = m_mainGather->getPlotWiggleScale();
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


void Sdp2dProcessedGatherDisplayArea::replotSelection(void)
{

}

void Sdp2dProcessedGatherDisplayArea::displayOneGather(float** data)
{
    getDisplayParameters();
    setOneGroupData(data);
    setHeaderDisplay();
    setColorDisplay();
    setDataForColorDisplay();
    setWiggleDisplay();
    updateWiggleDisplayData();
    m_customPlot->replot();
}

void Sdp2dProcessedGatherDisplayArea::getDisplayParameters()
{
    m_ntr = m_mainGather->getNumTracesOfCurrentGather();
    m_gatherIdx = m_mainGather->getGatherIndex();
    m_plotGathereType = m_mainGather->getGatherType();

    m_plotDisplayType = m_mainGather->getDisplayType();
    m_plotColormapType = m_mainGather->getColorMapIndex();
    m_plotWiggleScale = m_mainGather->getPlotWiggleScale();
    m_xscale = m_mainGather->getPlotXScale();
    m_yscale = m_mainGather->getPlotYScale();
    m_plotMaxTime = m_mainGather->getMaxDisplayTime();

    m_displayTraceHeader = m_mainGather->getTraceHeaderDisplay();

    m_plotXLabel  = m_mainGather->getPlotXLabel();
    m_plotYLabel  = m_mainGather->getPlotYLabel();
    m_plotClipPercent = m_mainGather->getDataClipPercentage();
    m_plotSymRange    = m_mainGather->getSymmetryRange();
    m_plotGroupStep   = m_mainGather->getGroupStep();

    m_plotXAxisType = m_mainGather->getPlotXAxisType();
    m_plotRevPolarity = m_mainGather->getReversepolarity();
    m_offEdge = m_mainGather->getOffsetSpacing();

    m_offRange->lower = m_mainGather->getOffsetValueRange()->lower;
    m_offRange->upper = m_mainGather->getOffsetValueRange()->upper;
    m_eleRange->lower = m_mainGather->getElevationValueRange()->lower;
    m_eleRange->upper = m_mainGather->getElevationValueRange()->upper;

    if(m_sd2d->getInteractiveFunction() == InteractiveFunctions::StackVelAnalysis){
        m_plotTitle   = m_mainGather->getPlotTitle() + QString(" : NMO");
    } else {
        m_plotTitle   = m_mainGather->getPlotTitle() + QString(" : ") + m_sd2d->getProcessingFunction();
    }

    m_offset.resize(m_ntr);
    m_elevation.resize(m_ntr);

    QVector<double>&  qvo = m_mainGather->getOffsetOfTheGather();
    QVector<double>&  qve = m_mainGather->getElevationOfTheGather();
    for(int i=0; i< m_ntr; i++){
        m_offset[i] = qvo[i];
        m_elevation[i] = qve[i];
    }

}

void Sdp2dProcessedGatherDisplayArea::setOneGroupData(float** data)
{
    if(m_gatherIn != nullptr) Sdp2dUtils::free2float(m_gatherIn);
    m_gatherIn = Sdp2dUtils::alloc2float(m_ns, m_ntr);
    for(int i=0; i< m_ntr; i++){
        for(int j=0; j< m_ns; j++){
            m_gatherIn[i][j] = data[i][j];
        }
    }
}

void Sdp2dProcessedGatherDisplayArea::setDataForColorDisplay(void)
{
    QCPRange dRange = m_mainGather->getDisplayedDataValueRange();
    int ns = int((1000*m_plotMaxTime)/(1000.0*m_dt))+1;
    double datavalue = 0;
    for (int xIndex=0; xIndex<m_ntr; xIndex++){
        for (int yIndex=0; yIndex<ns; yIndex++){
            datavalue = m_gatherIn[xIndex][yIndex];
            if(datavalue < dRange.lower) datavalue = dRange.lower;
            if(datavalue > dRange.upper) datavalue = dRange.upper;
            if(m_plotRevPolarity) datavalue = -datavalue;
            m_colorMap->data()->setCell(xIndex, yIndex, datavalue);
        }
    }
    m_colorMap->rescaleDataRange(true);
}

int  Sdp2dProcessedGatherDisplayArea::setGatherType(int gatherType, int minNtraces)
{
    Q_UNUSED(minNtraces);
    m_plotGathereType = gatherType;
    return m_gatherIdx;
}

void Sdp2dProcessedGatherDisplayArea::updateWiggleTraces()
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
    m_customPlot->replot();
}

void Sdp2dProcessedGatherDisplayArea::updateWiggleDisplayData(void)
{
    int keySize = m_colorMap->data()->keySize();
    if( keySize != m_wigTrcGraphs.size() ){
        cout << "Processed gather keySize=" << keySize << " graphSize=" << m_wigTrcGraphs.size()<< ". Something WRONG!"  << endl;
        return;
    }

    for(int trSeq=0; trSeq < keySize; trSeq++){
        QCPGraph* wav = m_wigTrcGraphs[trSeq];
        QCPGraph* ref = m_wigRefGraphs[trSeq];
        if(wav == nullptr) {
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
}

void Sdp2dProcessedGatherDisplayArea::setDataClipPercentage(int value)
{
    if(value == m_plotClipPercent) return;
    m_plotClipPercent = value;

    setDataForColorDisplay();
    updateWiggleDisplayData();
    m_customPlot->replot();
}

void Sdp2dProcessedGatherDisplayArea::setDisplayType(int displayType)
{
    if(displayType == m_plotDisplayType) return;
    m_plotDisplayType = displayType;

    if(m_plotDisplayType == SeismicDisplayTpye::Wiggle) {
        m_colorMap->setVisible(false);
        m_colorScale->setVisible(false);
    } else {
        m_colorMap->setVisible(true);
        m_colorScale->setVisible(true);
    }

    if(m_plotDisplayType == SeismicDisplayTpye::Color){
        for(int trSeq=0; trSeq < m_ntr; trSeq++){
            if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(false);
        }
    }else{
        for(int trSeq=0; trSeq < m_ntr; trSeq++){
            if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(true);
        }
    }
    m_customPlot->replot();
}

void Sdp2dProcessedGatherDisplayArea::setPlotWiggleScale(float val)
{
    if(fabs(val - m_plotWiggleScale) < 0.001) return;
    m_plotWiggleScale = val;
    //cout << "m_plotWiggleScale = " << m_plotWiggleScale << endl;
    setHeaderDisplay();
    updateWiggleTraces();
}

void Sdp2dProcessedGatherDisplayArea::setPlotXAxisType(int val)
{
    if(m_plotXAxisType == val) return;
    m_plotXAxisType = val;
    setHeaderDisplay();
    updateWiggleTraces();
}

void Sdp2dProcessedGatherDisplayArea::setReversepolarity(int revPolarity)
{
    if(revPolarity == m_plotRevPolarity) return;
    //cout << "symRange = " << symRange << endl;
    m_plotRevPolarity = revPolarity;
    setDataForColorDisplay();
    updateWiggleDisplayData();
    m_customPlot->replot();
}
