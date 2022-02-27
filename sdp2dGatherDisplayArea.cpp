#include "sdp2dGatherDisplayArea.h"
#include "seismicdataprocessing2d.h"
#include "sdp2dUtils.h"
#include "seismicdata2dprestack.h"
#include "seismicdata2d.h"
#include "qcustomplot.h"

Sdp2dGatherDisplayArea::Sdp2dGatherDisplayArea(SeismicData2D* sd2d, QWidget *parent)
    : QScrollArea(parent)
{
    setBackgroundRole(QPalette::Background);
    setMinimumSize(400,400);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setAlignment(Qt::AlignLeft|Qt::AlignTop);
    setMouseTracking(true);
    setCursor(Qt::ArrowCursor);
    setAttribute(Qt::WA_DeleteOnClose);

    m_mainWindow = dynamic_cast<SeismicDataProcessing2D*>(parent);
    m_statusbar = m_mainWindow->getStatusbarPointer();

    m_colorWigTrace = QColor(105, 105, 105);
    m_colorEleCurve = QColor(Qt::blue);
    m_colorOffCurve = QColor(116, 0, 161);

    m_customPlot = nullptr;
    m_colorScale = nullptr;
    m_colorMap = nullptr;

    m_displayTraceHeader = false;

    m_gatherAxisRect = nullptr;
    m_offsetAxisRect = nullptr;
    m_offgraph = nullptr;
    m_elegraph = nullptr;
    m_offRange = new QCPRange();
    m_eleRange = new QCPRange();

    m_wigTrcGraphs.resize(0);
    m_wigRefGraphs.resize(0);

    m_ns  = sd2d->getSamplesPerTraces();
    m_dt  = sd2d->getTimeSampleRateInUs()/1000000.;

    m_applyTopMute = false;
    m_applyBtmMute = false;
    m_hideBadTraces= false;

    m_plotMaxTime = (m_ns-1)*m_dt;
    m_plotTitle   = QString("Common Shot Gather 1");
    m_plotXLabel  = QString("Trace Index");
    m_plotYLabel  = QString("Time(s)");
    m_plotGathereType = DisplayGatherType::CommonShot;
    m_plotDisplayType = SeismicDisplayTpye::Color;
    m_plotColormapType = QCPColorGradient::gpGrayscale;
    m_plotClipPercent = 99;
    m_plotSymRange    = 1;
    m_plotGroupStep   = 1;
    m_plotWiggleScale = 3;
    m_plotXAxisType = 1;
    m_plotRevPolarity = 0;
    m_xscale = 3;
    m_yscale = 100;
    m_gatherIdx = 1;

    m_offEdge = sd2d->getOffsetSpacing();

    m_gatherIn =  nullptr;
    m_traceIn = new float [m_ns];

    m_time.resize(m_ns);
    m_tval.resize(m_ns);
    m_tref.resize(m_ns);
    for(int it=0; it< m_ns; it++) m_time[it] = it*m_dt;

}

Sdp2dGatherDisplayArea::~Sdp2dGatherDisplayArea()
{
    if(m_gatherIn != nullptr) Sdp2dUtils::free2float(m_gatherIn);

    m_customPlot->clearPlottables();
    delete [] m_traceIn;
    m_time.clear();
    m_tval.clear();
    m_tref.clear();
    m_wigTrcGraphs.clear();
    m_wigRefGraphs.clear();

    delete m_offRange;
    delete m_eleRange;
}


void Sdp2dGatherDisplayArea::resizeEvent(QResizeEvent *event)
{
    QScrollArea::resizeEvent(event);
    //cout <<"Sdp2dMainGatherDisplayArea resizeEvent width=" << this->width() << " height=" << this->height() << endl;

    if(m_mainWindow->isGatherDisplayFitToScrollWindow()) {
       resizeToFitTheDisplay();
    }
}

void Sdp2dGatherDisplayArea::resizeToFitTheDisplay()
{
    m_customPlot->resize(this->width(),this->height());
    //cout << "resizeToFitTheDisplay width="<< this->width() << " height=" << this->height() << endl;
    calculatePlotScales();
    replotSelection();
}

void Sdp2dGatherDisplayArea::calculatePlotScales(void)
{

    QPointF topLeft = m_colorMap->coordsToPixels(1, 0);
    QPointF btmRight = m_colorMap->coordsToPixels(m_ntr, m_plotMaxTime);
    QPointF plotSize = QPointF(btmRight.x() - topLeft.x(), btmRight.y() - topLeft.y());
    QPointF plotEdge = QPointF(m_customPlot->width() - m_gatherAxisRect->width(), m_customPlot->height() -m_gatherAxisRect->height());

    m_xscale = plotSize.x()/(m_ntr-1.0);
    m_yscale = plotSize.y()/m_plotMaxTime;
    m_sd2d->setDisplayScales(m_xscale, m_yscale, plotEdge);
}


void Sdp2dGatherDisplayArea::resizeWithXYScales()
{
    int edge = m_plotWiggleScale;
    if(edge > 5) edge = 5;
    if(edge < 1) edge = 1;
    QPointF plotEdge = m_sd2d->getPlotEdge();
    double wchange = (m_ntr-1.0+2*edge)*m_xscale + plotEdge.x();
    double hchange = m_plotMaxTime * m_yscale + plotEdge.y();
    m_customPlot->resize(wchange,hchange);
}

void Sdp2dGatherDisplayArea::createDisplayElements(void)
{
    createGatherDisplayAxisRect();
    createGatherColorDisplayBase();
    createOffsetDisplayAxisRect();
}

void Sdp2dGatherDisplayArea::setupCustomPlot(void)
{
    m_customPlot->plotLayout()->clear();
    m_customPlot->plotLayout()->addElement(0, 0, new QCPTextElement(m_customPlot, m_plotTitle, 18));

    int rowIdx = 1;
    if(m_displayTraceHeader) {
        m_customPlot->plotLayout()->addElement(1, 0, m_offsetAxisRect);
        rowIdx = 2;
    }

    m_customPlot->plotLayout()->addElement(rowIdx, 0, m_gatherAxisRect);
    m_customPlot->plotLayout()->addElement(rowIdx, 1, m_colorScale);
    if(m_sd2d->getDisplayType() != SeismicDisplayTpye::Wiggle){
        m_colorMap->setVisible(true);
        m_colorScale->setVisible(true);
    } else {
        m_colorMap->setVisible(false);
        m_colorScale->setVisible(false);
    }
}

void Sdp2dGatherDisplayArea::createGatherColorDisplayBase(void)
{
    m_colorScale = new QCPColorScale(m_customPlot);
    m_colorScale->axis()->setLabel("Seismic Amplitude");
    m_colorScale->axis()->setNumberFormat("eb");
    m_colorScale->axis()->setNumberPrecision(1);
    m_colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, m_marginVGroup);

    m_colorMap = new QCPColorMap(m_gatherAxisRect->axis(QCPAxis::atTop), m_gatherAxisRect->axis(QCPAxis::atLeft));
    m_colorMap->setInterpolate(true);
    m_colorMap->setColorScale(m_colorScale); // associate the color map with the color scale

}

void Sdp2dGatherDisplayArea::createOffsetDisplayAxisRect(void)
{
    if(!m_displayTraceHeader) return;

    m_offsetAxisRect = new QCPAxisRect(m_customPlot, false);
    m_offsetAxisRect->setupFullAxesBox(true);
    m_offsetAxisRect->setSizeConstraintRect(QCPLayoutElement::SizeConstraintRect(0));

    m_offsetAxisRect->setMaximumSize(QSize(2000,100));
    m_offsetAxisRect->setMinimumSize(QSize(200,100));

    QMargins m = m_offsetAxisRect->margins();
    m.setBottom(0);
    m.setTop(0);
    m_offsetAxisRect->setMargins(m);

    m_offsetAxisRect->axis(QCPAxis::atRight)->setBasePen(QPen(Qt::black, 2));
    m_offsetAxisRect->axis(QCPAxis::atRight)->setTickPen(QPen(Qt::black, 2));
    m_offsetAxisRect->axis(QCPAxis::atRight)->setVisible(true);
    m_offsetAxisRect->axis(QCPAxis::atRight)->setTickLength(0, 5);
    m_offsetAxisRect->axis(QCPAxis::atRight)->setSubTickLength(0, 3);
    m_offsetAxisRect->axis(QCPAxis::atRight)->setTickLabels(true);
    m_offsetAxisRect->axis(QCPAxis::atRight)->setTickLabelColor(m_colorEleCurve);
    m_offsetAxisRect->axis(QCPAxis::atRight)->setLabelColor(m_colorEleCurve);
    m_offsetAxisRect->axis(QCPAxis::atRight)->setLabel("Elevation(m)");
    m_offsetAxisRect->axis(QCPAxis::atRight)->grid()->setPen(QPen(QColor(140, 140, 140), 4, Qt::DotLine));
    m_offsetAxisRect->axis(QCPAxis::atRight)->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 4, Qt::DotLine));
    m_offsetAxisRect->axis(QCPAxis::atRight)->grid()->setSubGridVisible(true);

    m_offsetAxisRect->axis(QCPAxis::atLeft)->setVisible(true);
    m_offsetAxisRect->axis(QCPAxis::atLeft)->setBasePen(QPen(Qt::black, 2));
    m_offsetAxisRect->axis(QCPAxis::atLeft)->setTickPen(QPen(Qt::black, 2));
    m_offsetAxisRect->axis(QCPAxis::atLeft)->setTickLength(0, 5);
    m_offsetAxisRect->axis(QCPAxis::atLeft)->setSubTickLength(0, 3);
    m_offsetAxisRect->axis(QCPAxis::atLeft)->setTickLabels(true);
    m_offsetAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(m_colorOffCurve);
    m_offsetAxisRect->axis(QCPAxis::atLeft)->setLabelColor(m_colorOffCurve);
    m_offsetAxisRect->axis(QCPAxis::atLeft)->setLabel("Offset(m)");

    m_offsetAxisRect->axis(QCPAxis::atRight)->ticker()->setTickCount(3);
    m_offsetAxisRect->axis(QCPAxis::atLeft)->ticker()->setTickCount(4);

    m_offsetAxisRect->axis(QCPAxis::atTop)->setVisible(true);
    m_offsetAxisRect->axis(QCPAxis::atBottom)->setVisible(true);
    m_offsetAxisRect->axis(QCPAxis::atTop)->setTickLabels(false);
    m_offsetAxisRect->axis(QCPAxis::atBottom)->setTickLabels(false);

    m_offsetAxisRect->axis(QCPAxis::atBottom)->setTickLabelPadding(0);
    m_offsetAxisRect->axis(QCPAxis::atBottom)->setLabelPadding(0);

    m_offgraph = new QCPGraph(m_offsetAxisRect->axis(QCPAxis::atTop), m_offsetAxisRect->axis(QCPAxis::atLeft));
    m_elegraph = new QCPGraph(m_offsetAxisRect->axis(QCPAxis::atTop), m_offsetAxisRect->axis(QCPAxis::atRight));

    m_offgraph->setPen(QPen(m_colorOffCurve));
    m_offgraph->setLineStyle(QCPGraph::lsLine);

    m_elegraph->setPen(QPen(m_colorEleCurve));
    m_elegraph->setLineStyle(QCPGraph::lsLine);
    m_elegraph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, m_colorEleCurve, Qt::white, 7));

    QCPMarginGroup* g = new QCPMarginGroup(m_customPlot);
    m_gatherAxisRect->setMarginGroup(QCP::msLeft | QCP::msRight, g);
    m_offsetAxisRect->setMarginGroup(QCP::msLeft | QCP::msRight, g);
}

void Sdp2dGatherDisplayArea::createGatherDisplayAxisRect(void)
{
    m_gatherAxisRect = new QCPAxisRect(m_customPlot);
    m_gatherAxisRect->setupFullAxesBox(true);
    m_gatherAxisRect->setSizeConstraintRect(QCPLayoutElement::SizeConstraintRect(0));

    QMargins m = m_gatherAxisRect->margins();
    m.setTop(0);
    m.setBottom(0);
    m_gatherAxisRect->setMargins(m);

    m_marginVGroup = new QCPMarginGroup(m_customPlot);
    m_gatherAxisRect->setMarginGroup(QCP::msBottom|QCP::msTop, m_marginVGroup);

    m_gatherAxisRect->axis(QCPAxis::atRight)->setVisible(true);
    m_gatherAxisRect->axis(QCPAxis::atRight)->setTickLength(0, 5);
    m_gatherAxisRect->axis(QCPAxis::atRight)->setSubTickLength(0, 3);
    m_gatherAxisRect->axis(QCPAxis::atRight)->setTickLabels(false);
    m_gatherAxisRect->axis(QCPAxis::atRight)->ticker()->setTickCount(9);
    m_gatherAxisRect->axis(QCPAxis::atRight)->setRangeReversed(true);
    m_gatherAxisRect->axis(QCPAxis::atRight)->setBasePen(QPen(Qt::black, 2));
    m_gatherAxisRect->axis(QCPAxis::atRight)->setTickPen(QPen(Qt::black, 2));
    m_gatherAxisRect->axis(QCPAxis::atRight)->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    m_gatherAxisRect->axis(QCPAxis::atRight)->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    m_gatherAxisRect->axis(QCPAxis::atRight)->grid()->setSubGridVisible(true);
    m_gatherAxisRect->axis(QCPAxis::atRight)->setNumberPrecision(3);

    m_gatherAxisRect->axis(QCPAxis::atLeft)->grid()->setZeroLinePen(QPen(Qt::black, 1));
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setLabel(m_plotYLabel);
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setVisible(true);
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setTickLabels(true);
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setNumberPrecision(3);
    m_gatherAxisRect->axis(QCPAxis::atLeft)->ticker()->setTickCount(9);
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setBasePen(QPen(Qt::black, 2));
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setTickPen(QPen(Qt::black, 2));
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setSubTickPen(QPen(Qt::black, 1));
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setTickLabelColor(Qt::black);
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setTickLength(0, 5);
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setSubTickLength(0, 3);
    m_gatherAxisRect->axis(QCPAxis::atLeft)->setRangeReversed(true);

    m_gatherAxisRect->axis(QCPAxis::atTop)->ticker()->setTickCount(9);
    m_gatherAxisRect->axis(QCPAxis::atTop)->setVisible(true);

    m_gatherAxisRect->axis(QCPAxis::atTop)->setBasePen(QPen(Qt::black, 2));
    m_gatherAxisRect->axis(QCPAxis::atTop)->setTickPen(QPen(Qt::black, 2));
    m_gatherAxisRect->axis(QCPAxis::atTop)->setSubTickPen(QPen(Qt::black, 1));
    m_gatherAxisRect->axis(QCPAxis::atTop)->setTickLabelColor(Qt::black);
    m_gatherAxisRect->axis(QCPAxis::atTop)->setTickLength(0, 5);
    m_gatherAxisRect->axis(QCPAxis::atTop)->setSubTickLength(0, 3);
    m_gatherAxisRect->axis(QCPAxis::atTop)->setTickLabels(true);

    m_gatherAxisRect->axis(QCPAxis::atTop)->setTickLabelPadding(1);
    m_gatherAxisRect->axis(QCPAxis::atTop)->setLabelPadding(0);

    m_gatherAxisRect->axis(QCPAxis::atBottom)->setVisible(true);
    m_gatherAxisRect->axis(QCPAxis::atBottom)->setTickLength(0, 5);
    m_gatherAxisRect->axis(QCPAxis::atBottom)->setSubTickLength(0, 3);
    m_gatherAxisRect->axis(QCPAxis::atBottom)->setTickLabels(false);
    m_gatherAxisRect->axis(QCPAxis::atTop)->setLabel(m_plotXLabel);

}


void Sdp2dGatherDisplayArea::setHeaderDisplay()
{
    if(!m_displayTraceHeader) return;
    if(m_offgraph == nullptr || m_elegraph== nullptr || m_offsetAxisRect == nullptr) return;

    int edge = m_plotWiggleScale;
    if(edge > 5) edge = 5;
    if(edge < 1) edge = 1;

    if(m_plotXAxisType == XAxisType::TraceIdx) {
        m_offsetAxisRect->axis(QCPAxis::atTop)->setRange(1-edge, m_ntr+edge);
        m_offsetAxisRect->axis(QCPAxis::atBottom)->setRange(1-edge, m_ntr+edge);
    } else if(m_plotXAxisType == XAxisType::OffsetVal) {
        double lower = m_offRange->lower - edge*m_offEdge;
        double upper = m_offRange->upper + edge*m_offEdge;
        m_offsetAxisRect->axis(QCPAxis::atTop)->setRange(lower, upper);
        m_offsetAxisRect->axis(QCPAxis::atBottom)->setRange(lower, upper);
    }

    m_offsetAxisRect->axis(QCPAxis::atLeft)->setRange(m_offRange->lower, m_offRange->upper);
    m_offsetAxisRect->axis(QCPAxis::atRight)->setRange(m_eleRange->lower, m_eleRange->upper);
    m_offsetAxisRect->axis(QCPAxis::atLeft)->ticker()->setTickOrigin(int(m_offRange->lower));
    m_offsetAxisRect->axis(QCPAxis::atRight)->ticker()->setTickOrigin(int(m_eleRange->lower));

    if(m_plotXAxisType == XAxisType::TraceIdx) {
        m_traceidx.resize(m_ntr);
        for(int i=0; i<m_ntr; i++) m_traceidx[i] = i+1;
        m_offgraph->setData(m_traceidx, m_offset);
        m_elegraph->setData(m_traceidx, m_elevation);
    } else if(m_plotXAxisType == XAxisType::OffsetVal) {
        m_offgraph->setData(m_offset, m_offset);
        m_elegraph->setData(m_offset, m_elevation);
    }
}

void Sdp2dGatherDisplayArea::setColorDisplay()
{
    int index = m_customPlot->plotLayout()->rowColToIndex(0, 0);
    QCPTextElement* e1 = dynamic_cast<QCPTextElement*>(m_customPlot->plotLayout()->elementAt(index));
    e1->setText(m_plotTitle);

    QCPColorGradient a((QCPColorGradient::GradientPreset)m_plotColormapType);
    m_colorMap->setGradient(a);

    int edge = m_plotWiggleScale;
    if(edge > 5) edge = 5;
    if(edge < 1) edge = 1;

    if(m_plotXAxisType == XAxisType::TraceIdx) {
        m_gatherAxisRect->axis(QCPAxis::atTop)->setRange(1-edge, m_ntr+edge);
        m_gatherAxisRect->axis(QCPAxis::atBottom)->setRange(1-edge, m_ntr+edge);
    } else if(m_plotXAxisType == XAxisType::OffsetVal) {
        double lower = m_offRange->lower - edge*m_offEdge;
        double upper = m_offRange->upper + edge*m_offEdge;
        m_gatherAxisRect->axis(QCPAxis::atTop)->setRange(lower, upper);
        m_gatherAxisRect->axis(QCPAxis::atBottom)->setRange(lower, upper);
    }

    m_gatherAxisRect->axis(QCPAxis::atLeft)->setRange(0, m_plotMaxTime);
    m_gatherAxisRect->axis(QCPAxis::atRight)->setRange(0, m_plotMaxTime);

    int ns = int((1000*m_plotMaxTime)/(1000.0*m_dt))+1;

    if(!m_colorMap->data()->isEmpty())   m_colorMap->data()->clear();
    m_colorMap->data()->setSize(m_ntr, ns);
    m_colorMap->data()->setRange(QCPRange(1, m_ntr), QCPRange(0, m_plotMaxTime));

    if(m_plotDisplayType == SeismicDisplayTpye::Wiggle) {
        m_colorMap->setVisible(false);
        if(m_colorScale!=nullptr) m_colorScale->setVisible(false);
    } else {
        m_colorMap->setVisible(true);
        if(m_colorScale!=nullptr) m_colorScale->setVisible(true);
    }
}

void Sdp2dGatherDisplayArea::setWiggleDisplay()
{
    //cout << " m_wigTrcGraphs.size() = " << m_wigTrcGraphs.size() << endl;
    for(int i=0; i< m_wigTrcGraphs.size(); i++){
        if(m_wigTrcGraphs[i] != nullptr){
            m_customPlot->removeGraph(m_wigTrcGraphs[i]);
            m_customPlot->removeGraph(m_wigRefGraphs[i]);
        }
    }

    int keySize = m_colorMap->data()->keySize();
    m_wigTrcGraphs.resize(keySize);
    m_wigRefGraphs.resize(keySize);
    for(int trSeq=0; trSeq < keySize; trSeq++){
        m_wigTrcGraphs[trSeq] = nullptr;
        m_wigRefGraphs[trSeq] = nullptr;
    }
}

QPointF Sdp2dGatherDisplayArea::getTopLeftOfPlotArea()
{
    double key, val;
    key = 1;
    val = 0;
    QPointF topleft = m_colorMap->coordsToPixels(key, val);
    return topleft;
}

QPointF Sdp2dGatherDisplayArea::getBtmRightOfPlotArea()
{
    double key, val;
    key = m_ntr;
    val = m_plotMaxTime;
    QPointF btmright = m_colorMap->coordsToPixels(key, val);
    return btmright;
}

void Sdp2dGatherDisplayArea::setTraceHeaderDisplay(bool checked)
{
    if(m_displayTraceHeader == checked) return;
    m_displayTraceHeader = checked;

    if(m_offsetAxisRect == nullptr && checked) {
        createOffsetDisplayAxisRect();
        setHeaderDisplay();
        m_customPlot->plotLayout()->insertRow(1);
        m_customPlot->plotLayout()->addElement(1, 0, m_offsetAxisRect);
    } else {
        QCPLayoutElement* element = dynamic_cast<QCPLayoutElement*>(m_offsetAxisRect);
        m_customPlot->removeGraph(m_offgraph);
        m_customPlot->removeGraph(m_elegraph);
        m_customPlot->plotLayout()->remove(element);
        m_customPlot->plotLayout()->simplify();
        m_offsetAxisRect = nullptr;
        m_offgraph = nullptr;
        m_elegraph = nullptr;
    }

    m_customPlot->replot();
    replotSelection();
}

void Sdp2dGatherDisplayArea::setPlotTitle(QString text)
{
    if(m_plotTitle.compare(text) == 0) return;
    m_plotTitle = text;
    int index = m_customPlot->plotLayout()->rowColToIndex(0, 0);
    QCPTextElement* e1 = dynamic_cast<QCPTextElement*>(m_customPlot->plotLayout()->elementAt(index));
    e1->setText(m_plotTitle);
    m_customPlot->replot();
}

void Sdp2dGatherDisplayArea::setPlotXLabel(QString text)
{
    if(m_plotXLabel.compare(text) == 0) return;
    m_plotXLabel = text;
    m_customPlot->xAxis2->setLabel(m_plotXLabel);
    m_customPlot->replot();
}

void Sdp2dGatherDisplayArea::setPlotYLabel(QString text)
{
    if(m_plotYLabel.compare(text) == 0) return;
    m_plotYLabel = text;
    m_customPlot->yAxis->setLabel(m_plotYLabel);
    m_customPlot->replot();
}

void Sdp2dGatherDisplayArea::setColorMapIndex(int colormapIdx)
{
    //cout << "colormapIdx=" << colormapIdx << endl;
    switch(colormapIdx){
    case 0:  m_plotColormapType = QCPColorGradient::gpGrayscale;   break;
    case 1:  m_plotColormapType = QCPColorGradient::gpBwr;         break;
    case 2:  m_plotColormapType = QCPColorGradient::gpHot;         break;
    case 3:  m_plotColormapType = QCPColorGradient::gpCold;        break;
    case 4:  m_plotColormapType = QCPColorGradient::gpNight;       break;
    case 5:  m_plotColormapType = QCPColorGradient::gpCandy;       break;
    case 6:  m_plotColormapType = QCPColorGradient::gpGeography;   break;
    case 7:  m_plotColormapType = QCPColorGradient::gpIon;         break;
    case 8:  m_plotColormapType = QCPColorGradient::gpThermal;     break;
    case 9:  m_plotColormapType = QCPColorGradient::gpPolar;       break;
    case 10: m_plotColormapType = QCPColorGradient::gpSpectrum;    break;
    case 11: m_plotColormapType = QCPColorGradient::gpJet;         break;
    case 12: m_plotColormapType = QCPColorGradient::gpHues;        break;
    default: m_plotColormapType = QCPColorGradient::gpGrayscale;
    }

    QCPColorGradient a((QCPColorGradient::GradientPreset)m_plotColormapType);
    m_colorMap->setGradient(a);
    m_colorMap->rescaleDataRange(true);
    m_customPlot->replot();
}

void Sdp2dGatherDisplayArea::setMaxDisplayTime(float tlen)
{
    if(abs(m_plotMaxTime - tlen) < 0.0001) return;
    m_plotMaxTime = tlen;
    setColorDisplay();
    setDataForColorDisplay();
    m_customPlot->replot();
}

void Sdp2dGatherDisplayArea::setSymmetryRange(int symRange)
{
    if(symRange == m_plotSymRange) return;
    //cout << "symRange = " << symRange << endl;
    m_plotSymRange = symRange;
    setDataForColorDisplay();
    m_customPlot->replot();
}

void Sdp2dGatherDisplayArea::setGroupStep(int value)
{
    m_plotGroupStep = value;
}

void Sdp2dGatherDisplayArea::setPlotXScale(float val, QPointF plotEdge, bool replot)
{
    if(fabs(val - m_xscale)  < 0.001) return;
    m_xscale = val;

    if(!replot) return;

    double wchange = (m_ntr-1.0)*m_xscale + plotEdge.x();
    double hchange = m_plotMaxTime * m_yscale + plotEdge.y();
    m_customPlot->resize(wchange,hchange);
    replotSelection();
    m_customPlot->replot();
}

void Sdp2dGatherDisplayArea::setPlotYScale(float val, QPointF plotEdge, bool replot)
{
    if(fabs(val - m_yscale) < 0.001) return;
    m_yscale = val;

    if(!replot) return;

    double wchange = (m_ntr-1.0)*m_xscale + plotEdge.x();
    double hchange = m_plotMaxTime * m_yscale + plotEdge.y();
    m_customPlot->resize(wchange,hchange);
    replotSelection();
    m_customPlot->replot();
}

QPointF Sdp2dGatherDisplayArea::convertPosToAxisValues(QPointF pos)
{
    double x = pos.x();
    double y = pos.y();
    double key=0, value=0;
    m_colorMap->pixelsToCoords( x,  y,  key,  value);
    //cout << " x=" << x << " y="<< y << " key="<< key << " val=" << value << endl;
    if(m_ntr <= 1 && m_plotXAxisType == 2) {
        return QPointF(1, value);
    }

    if(m_plotXAxisType == 2){
        float kmin = m_offset[0] - (m_offset[1] -  m_offset[0])*0.5 - m_offEdge;
        float kmax = m_offset[m_ntr-1] + m_offEdge;
        float kval = key;

        int i=0;
        if(kval < kmin || kval > kmax) {
            key = -1;
        } else {
            for(i=1; i< m_ntr; i++){
                if((m_offset[i] + m_offset[i-1])*0.5 > kval) break;
            }
            key = i;   //return traceIndex = traceSeq + 1
        }
        //cout << "XAxisType=" << m_plotXAxisType << " x=" << x << " key=" << key  << " kval="<< kval << " i=" << i << " off0="<< m_offset[0] << " off1="<< m_offplus[0] << " edge=" << m_offEdge<<endl;
    } else if(m_plotXAxisType == 1){
        if(key < -1 || key > m_ntr+1) {
            key = -1;
        }else if (key < 0.5) {
            key = 1;
        } else if (key > m_ntr-0.5) {
            key = m_ntr;
        } else {
            key += 0.5; //return traceIndex = traceSeq + 1
        }
    }
    //cout << "XAxisType=" << m_plotXAxisType << " key=" << key << endl;
    //if(value < -0.01 || value > m_plotMaxTime+0.01){
    //    value = 0;
    //}

    return QPointF(key,  value);
}

void Sdp2dGatherDisplayArea::convertAxisValuesToPos(double key, double value, double& x, double& y)
{
    if(m_plotXAxisType == 2){
        int idx = int(key) - 1;
        double offset = m_offset[idx];
         m_colorMap->coordsToPixels(offset, value, x, y);
    } else {
        m_colorMap->coordsToPixels(key, value, x, y);
    }
}

void Sdp2dGatherDisplayArea::hideInputGather(bool hide)
{
    if(hide){
        m_colorMap->setVisible(false);
        for(int trSeq=0; trSeq < m_ntr; trSeq++){
            if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(false);
        }
    } else {
        if(m_plotDisplayType == SeismicDisplayTpye::Wiggle) {
            for(int trSeq=0; trSeq < m_ntr; trSeq++){
               if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(true);
            }
        } else if(m_plotDisplayType == SeismicDisplayTpye::Color){
            m_colorMap->setVisible(true);
        } else {
            m_colorMap->setVisible(true);
            for(int trSeq=0; trSeq < m_ntr; trSeq++){
                if(m_wigTrcGraphs[trSeq] != nullptr) m_wigTrcGraphs[trSeq]->setVisible(true);
            }
        }
    }
}

QRect Sdp2dGatherDisplayArea::getSeismicDataRange(QPoint& s, QPoint& e)
{
    int ntr = getNumTracesOfCurrentGather();
    float dt = getTimeSampRateOfCurrentGather();
    int ns = getTimeSamplesOfCurrentView();

    double x1 = s.x();
    double y1 = s.y();
    QPointF t1 = convertPosToAxisValues(QPointF(x1, y1));

    int trace = int(t1.x());
    int time  = int((t1.y()*1000.)/(dt*1000.)+0.5);
    if(trace < 1) trace = 1;
    if(trace > ntr) trace = ntr;
    if(time < 0) time = 0;
    if(time > ns-1) time = ns-1;
    QPoint start(trace, time);

    double x2 = e.x();
    double y2 = e.y();
    QPointF t2 = convertPosToAxisValues(QPointF(x2, y2));
    trace = int(t2.x());
    time  = int((t2.y()*1000.)/(dt*1000.)+0.5);
    if(trace < 1) trace = 1;
    if(trace > ntr) trace = ntr;
    if(time < 0) time = 0;
    if(time > ns-1) time = ns-1;
    QPoint end(trace, time);

    QRect newRect = QRect(start, end).normalized();
    return newRect;
}

QCPRange Sdp2dGatherDisplayArea::getDisplayedDataValueRange(void)
{
    return m_colorMap->dataRange();
}

void Sdp2dGatherDisplayArea::setDataForDisplayedSingleTrace(QCPGraph* wavtr, QCPGraph* reftr, int refIdx)
{
    if(wavtr == nullptr) return;
    //cout << "refValue="<< refValue << endl;
    if(refIdx < 1) return;

    double* traces = m_colorMap->datap();
    int keySize = m_colorMap->data()->keySize();
    int valSize = m_colorMap->data()->valueSize();

    QCPRange drange = m_colorMap->dataRange();
    drange.normalize();
    double normalizeFactor = abs(drange.upper);

    float scale = m_plotWiggleScale/normalizeFactor;

    //cout << "scale = " << scale << "  m_plotWiggleScale=" << m_plotWiggleScale << " normalizeFactor="<< normalizeFactor << endl;

    float refValue = refIdx;
    if(m_plotXAxisType == 2){
        refValue = m_offset[refIdx-1];
        scale = scale * m_offEdge;
    }
    double val;
    for(int it=0; it< valSize; it++){
        int idx = it * keySize + refIdx -1;

        val = refValue + traces[idx]*scale;
        m_tval[it] = val;
        if(val < refValue){
            m_tref[it] = val;
        } else {
            m_tref[it] = refValue;
        }
    }
    for(int it=valSize; it < m_ns; it++){
        m_tval[it] = refValue;
        m_tref[it] = refValue;
    }
    wavtr->setData(m_time, m_tval);
    reftr->setData(m_time, m_tref);
}
