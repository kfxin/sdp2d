#include "sdp2dAmplitudeDisplayDock.h"
#include "sdp2dMainGatherDisplayArea.h"
#include "seismicdataprocessing2d.h"
#include "seismicdata2dprestack.h"
#include "seismicdata2d.h"
#include "sdp2dUtils.h"

Sdp2dAmpDisplayQCPGraph::Sdp2dAmpDisplayQCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis) :
    QCPGraph(keyAxis, valueAxis)
{

}

Sdp2dAmpDisplayQCPGraph::~Sdp2dAmpDisplayQCPGraph()
{

}

void Sdp2dAmpDisplayQCPGraph::draw(QCPPainter *painter)
{
    applyScattersAntialiasingHint(painter);

    double w = scatterStyle().size();
    QCPRange arange = mColorScale->dataRange();
    QCPColorGradient gradient = mColorScale->gradient();

    QCPAxis *keyAxis = mParentPlot->xAxis;
    QCPAxis *valueAxis = mParentPlot->yAxis;

    //int width  = keyAxis->axisRect()->width();
    //int height = keyAxis->axisRect()->height();

    //double rx = keyAxis->range().size();
    //double ry = valueAxis->range().size();

    //int pxsize = width*0.2/rx*markerSize;
    //int pxsize = markerSize;
    //if(pxsize<1) pxsize = 1;

    //int pysize = height*0.5/ry;
    //if(pysize<1) pysize = 1;
    //int wx = pxsize/2;
    //int wy = pysize/2;
    //cout << " Axis width=" << keyAxis->axisRect()->width() << " height="<< keyAxis->axisRect()->height() << endl;
    //cout << " rx=" << rx << " ry="<< ry << " pxsize=" << pxsize << " pysize=" << pysize <<  endl;

    double ylower = valueAxis->range().lower;
    double yupper = valueAxis->range().upper;
    double xlower = keyAxis->range().lower;
    double xupper = keyAxis->range().upper;

    int npoints = mDataContainer->constEnd() - mDataContainer->constBegin();
    if(npoints <=0 ) return;

    for(int i=0; i< npoints; i++)
    {
        double key = mDataContainer->at(i)->key;
        double value = mDataContainer->at(i)->value;

        if(value < ylower || value > yupper || key < xlower || key > xupper) continue;

        double x = keyAxis->coordToPixel(key);
        double y = valueAxis->coordToPixel(value);

        qreal amp = mScatterScale[i];
        if(mTraceFlag[i] == SeismicTraceFlag::PickedBadTrace || mTraceFlag[i] == SeismicTraceFlag::DeadTrace) {            
            painter->setPen(QPen(QColor(Qt::black)));
            painter->setBrush(QBrush(QColor(Qt::white)));
            painter->drawEllipse(QPointF(x, y), w, w);
            painter->drawLine(QLineF(x-w*0.707, y-w*0.707, x+w*0.670, y+w*0.670));
            painter->drawLine(QLineF(x-w*0.707, y+w*0.670, x+w*0.670, y-w*0.707));
        } else {
            if(amp < arange.lower) amp = arange.lower;
            else if(amp > arange.upper) amp = arange.upper;

            QColor c = QColor(gradient.color(amp, arange));
            painter->setPen(QPen(c));
            painter->setBrush(QBrush(c));
            painter->drawEllipse(QPointF(x , y), w, w);
        }

    }
}


Sdp2dAmplitudeDisplayDock::Sdp2dAmplitudeDisplayDock(SeismicData2DPreStack* sd2d, QString sd2dName, QWidget *mainWindow) :
    QDockWidget(mainWindow)
{    
    setWindowTitle(QString("Amplitude of ")+sd2dName);
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setMinimumSize(400,300);
    setFloating(true);

    m_sd2d = sd2d;
    m_sd2d->addDisplayWidgets(this);
    m_sd2d->setAmplitudeDockPointer(this);

    double maxTime = double(m_sd2d->getSamplesPerTraces()*m_sd2d->getTimeSampleRateInUs())/1000000.;
    m_muteRange = QCPRange(0, maxTime);

    m_mainWindow = dynamic_cast<SeismicDataProcessing2D*>(mainWindow);
    //m_mainWindow->addDockWidget(Qt::RightDockWidgetArea, this);
    m_statusbar = m_mainWindow->getStatusbarPointer();

    m_isHide = true;

    m_clipPercentage = 99;
    m_markerSize = 2;
    m_attType = 2;
    m_xaxisType = 0;

    m_pointKey.resize(1);
    m_pointVal.resize(1);

    m_trIdx = -1;
    m_trSeq = -1;

    m_myMenu = new QMenu(this);
    m_myMenu->addAction("Show Trace in Shot Gather", this, &Sdp2dAmplitudeDisplayDock::showTraceInShotGather);
    m_myMenu->addAction("Show Trace in CDP Gather", this, &Sdp2dAmplitudeDisplayDock::showTraceInCDPsGather);
    m_myMenu->addAction("Show Trace in Receiver Gather", this, &Sdp2dAmplitudeDisplayDock::showTraceInRecvGather);
    m_myMenu->addAction("Show Trace in Offset Gather", this, &Sdp2dAmplitudeDisplayDock::showTraceInOffsetGather);
    m_myMenu->addSeparator();
    m_myMenu->addAction("Hide the location point", this, &Sdp2dAmplitudeDisplayDock::hideTraceLocationPoint);


    loadAttributeInformation();
    createLayout();
    drawGraphy();

    m_customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_customPlot, &QCustomPlot::customContextMenuRequested, this, &Sdp2dAmplitudeDisplayDock::onCustomContextMenuRequested);
    connect(m_customPlot, &QCustomPlot::mouseMove, this, &Sdp2dAmplitudeDisplayDock::onMouseMove);
    connect(m_customPlot, &QCustomPlot::mouseDoubleClick, this, &Sdp2dAmplitudeDisplayDock::onMouseDoubleClick);    
}

Sdp2dAmplitudeDisplayDock::~Sdp2dAmplitudeDisplayDock()
{
    m_aveamp.clear();
    m_absamp.clear();
    m_rmsamp.clear();
    m_mutetime.clear();

    m_shotidx.clear();
    m_cdpsidx.clear();
    m_recvidx.clear();
    m_offsval.clear();
}

void Sdp2dAmplitudeDisplayDock::hideEvent(QHideEvent *event)
{
    //cout << "Sdp2dAmplitudeDisplayDock::hideEvent" << endl;
    QDockWidget::hideEvent(event);
}

void Sdp2dAmplitudeDisplayDock::closeEvent(QCloseEvent *event)
{
    m_isHide = true;
    //cout << "Sdp2dAmplitudeDisplayDock::closeEvent" << endl;
    QDockWidget::closeEvent(event);
}

void Sdp2dAmplitudeDisplayDock::showEvent(QShowEvent *event)
{
    m_isHide = false;
    //cout << "Sdp2dAmplitudeDisplayDock::showEvent" << endl;
    QDockWidget::showEvent(event);
}

void Sdp2dAmplitudeDisplayDock::createLayout(void)
{    
    QWidget *dockWidgetContents = new QWidget();
    this->setWidget(dockWidgetContents);
    dockWidgetContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QHBoxLayout* hbox0 = new QHBoxLayout();    
    createPlotArea(hbox0);

    QHBoxLayout* hbox1 = new QHBoxLayout();    
    QLabel* amptypeLabel = new QLabel("Attribute Type:");
    m_attTypeCBox = new QComboBox(this);
    //ampType->setFrame(false);
    QStringList v1;
    v1 << "aveAmplitude" << "absAmplitude" << "rmsAmplitude" << "topMute" << "bottomMute";
    m_attTypeCBox->addItems(v1);
    m_attTypeCBox->setCurrentIndex(m_attType);
    hbox1->addWidget(amptypeLabel);
    hbox1->addWidget(m_attTypeCBox);

    QHBoxLayout* hbox2 = new QHBoxLayout();
    QLabel* xaxislabel = new QLabel("X Axis:");
    m_xAxisCBox = new QComboBox(this);
    QStringList v2;
    v2 << "Shot Index" << "CDP Index" << "Receiver Index";
    m_xAxisCBox->addItems(v2);
    hbox2->addWidget(xaxislabel);
    hbox2->addWidget(m_xAxisCBox);

    QHBoxLayout* hbox3 = new QHBoxLayout();
    QLabel* cliplabel = new QLabel("Clip Percentage:");
    QDoubleSpinBox* spinbx1 = new QDoubleSpinBox(this);
    spinbx1->setSingleStep(0.1);
    spinbx1->setMinimum(1);
    spinbx1->setMaximum(100);
    spinbx1->setValue(99);
    hbox3->addWidget(cliplabel);
    hbox3->addWidget(spinbx1);

    QHBoxLayout* hbox4 = new QHBoxLayout();
    QLabel* markerLable = new QLabel("Marker Scaler:");
    QDoubleSpinBox* spinbx2 = new QDoubleSpinBox(this);
    spinbx2->setSingleStep(0.1);
    spinbx2->setMinimum(0.1);
    spinbx2->setMaximum(100);
    spinbx2->setValue(2);
    hbox4->addWidget(markerLable);
    hbox4->addWidget(spinbx2);

    QHBoxLayout* hbox5 = new QHBoxLayout();
    QLabel* colorLabel = new QLabel("Color Map:");
    QComboBox* colormap = new QComboBox(this);
    QStringList colors;
    colors << "Grayscale" <<"RedBlue" <<"Hot"<< "Cold" << "Night"<< "Candy"<< "Geography" << "Ion"<< "Thermal" << "Polar"<< "Spectrum"<< "Jet" << "Hues";
    colormap->addItems(colors);
    colormap->setCurrentIndex(9);
    hbox5->addWidget(colorLabel);
    hbox5->addWidget(colormap);

    QIcon icon;
    icon.addFile(QString::fromUtf8(":/images/fitscreen.png"), QSize(), QIcon::Normal, QIcon::Off);
    QPushButton* zoomFitBtn = new QPushButton(icon, "Zoom Fit");    

    QGridLayout* gridLayout = new QGridLayout(dockWidgetContents);

    amptypeLabel->setMinimumWidth(40);
    m_attTypeCBox->setMinimumWidth(40);
    xaxislabel->setMinimumWidth(40);
    m_xAxisCBox->setMinimumWidth(40);
    cliplabel->setMinimumWidth(40);
    spinbx1->setMinimumWidth(40);
    markerLable->setMinimumWidth(40);
    spinbx2->setMinimumWidth(40);
    colorLabel->setMinimumWidth(40);
    colormap->setMinimumWidth(40);
    zoomFitBtn->setMinimumWidth(40);

    gridLayout->addLayout(hbox0, 1, 0, 10, 6);
    gridLayout->addLayout(hbox1, 0, 0, 1, 1);
    gridLayout->addLayout(hbox2, 0, 1, 1, 1);
    gridLayout->addLayout(hbox3, 0, 2, 1, 1);
    gridLayout->addLayout(hbox4, 0, 3, 1, 1);
    gridLayout->addLayout(hbox5, 0, 4, 1, 1);
    gridLayout->addWidget(zoomFitBtn, 0, 5, 1, 1);

    connect(m_attTypeCBox, &QComboBox::currentTextChanged, this, &Sdp2dAmplitudeDisplayDock::attTypeChanged);
    connect(m_xAxisCBox, &QComboBox::currentTextChanged, this, &Sdp2dAmplitudeDisplayDock::xAxisTypeChanged);    
    connect(zoomFitBtn, &QPushButton::clicked, this, &Sdp2dAmplitudeDisplayDock::zoomFitBtnClicked);
    connect(spinbx1, &QDoubleSpinBox::textChanged, this, &Sdp2dAmplitudeDisplayDock::amplitudeClipPercentageChanged);
    connect(spinbx2, &QDoubleSpinBox::textChanged, this, &Sdp2dAmplitudeDisplayDock::amplitudeMarkerScaleChanged);
    connect(colormap, &QComboBox::currentTextChanged, this, &Sdp2dAmplitudeDisplayDock::amplitudeColormapChanged);
}

void Sdp2dAmplitudeDisplayDock::attTypeChanged(QString text)
{
    if(text.compare("aveAmplitude")==0) {
        if(m_attType == 0) return;
        m_attType=0;
        m_colorScale->setDataRange(m_aveAmpRange);
        m_colorScale->axis()->setLabel("Seismic Average Amplitude");
    } else if(text.compare("absAmplitude")==0) {
        if(m_attType == 1) return;
        m_attType=1;
        m_colorScale->setDataRange(m_absAmpRange);
        m_colorScale->axis()->setLabel("Seismic Absolute Amplitude");
    } else if(text.compare("rmsAmplitude")==0){
        if(m_attType == 2) return;
        m_attType=2;
        m_colorScale->setDataRange(m_rmsAmpRange);
        m_colorScale->axis()->setLabel("Seismic RMS Amplitude");
    }else if(text.compare("topMute")==0){
        if(m_attType == 3) return;
        m_attType=3;
        m_colorScale->setDataRange(m_muteRange);
        m_colorScale->axis()->setLabel("Seismic Top Mute");
    } else if(text.compare("bottomMute")==0){
        if(m_attType == 4) return;
        m_attType=4;
        m_colorScale->setDataRange(m_muteRange);
        m_colorScale->axis()->setLabel("Seismic Bottom Mute");
    }
    QCPRange xaxis = m_customPlot->xAxis->range();
    QCPRange yaxis = m_customPlot->yAxis->range();
    drawGraphy();
    m_customPlot->xAxis->setRange(xaxis);
    m_customPlot->yAxis->setRange(yaxis);
    m_customPlot->xAxis2->setRange(xaxis);
    m_customPlot->yAxis2->setRange(yaxis);
}

void Sdp2dAmplitudeDisplayDock::xAxisTypeChanged(QString text)
{
    if( text.compare(QString("Shot Index")) == 0) {
        if(m_xaxisType == 0) return;
        m_xaxisType=0;
        m_customPlot->xAxis->setLabel("Shot Index");
        m_attScatters->setData(m_shotidx, m_offsval, true);
    } else if( text.compare(QString("CDP Index")) == 0) {
        if(m_xaxisType == 1) return;
        m_xaxisType=1;
        m_customPlot->xAxis->setLabel("CDP Index");
        m_attScatters->setData(m_cdpsidx, m_offsval, true);
    } else if( text.compare(QString("Receiver Index")) == 0) {
        if(m_xaxisType == 2) return;
        m_xaxisType=2;
        m_customPlot->xAxis->setLabel("Receiver Index");
        m_attScatters->setData(m_recvidx, m_offsval, true);
    }
    displayCurrentPointLocation();
    zoomFitBtnClicked();
}

void Sdp2dAmplitudeDisplayDock::loadAttributeInformation(void)
{
    int ntr = m_sd2d->getNumberOfTraces();
    //cout << "ntr=" << ntr << endl;
    if(m_aveamp.size() != ntr) m_aveamp.resize(ntr);
    if(m_absamp.size() != ntr) m_absamp.resize(ntr);
    if(m_rmsamp.size() != ntr) m_rmsamp.resize(ntr);
    if(m_mutetime.size() != ntr) m_mutetime.resize(ntr);

    if(m_shotidx.size() != ntr) m_shotidx.resize(ntr);
    if(m_cdpsidx.size() != ntr) m_cdpsidx.resize(ntr);
    if(m_recvidx.size() != ntr) m_recvidx.resize(ntr);
    if(m_offsval.size() != ntr) m_offsval.resize(ntr);    

    m_sd2d->getAveageAmplitude(m_aveamp.data());
    m_sd2d->getAbsoluteAmplitude(m_absamp.data());
    m_sd2d->getRMSAmplitude(m_rmsamp.data());

    //cout << " Ave Amp 1st=" << m_aveamp.at(0) << " last=" << m_aveamp.at(ntr-1) << endl;
    //cout << " Abs Amp 1st=" << m_absamp.at(0) << " last=" << m_absamp.at(ntr-1) << endl;
    //cout << " RMS Amp 1st=" << m_rmsamp.at(0) << " last=" << m_rmsamp.at(ntr-1) << endl;

    for(int i=0; i<ntr; i++) m_mutetime[i] = 0;

    int* p = m_sd2d->getCDPsIdPointer();
    for(int i=0; i<ntr; i++) m_cdpsidx[i] = p[i];

    p = m_sd2d->getShotIdPointer();
    for(int i=0; i<ntr; i++)  m_shotidx[i] = p[i];

    p = m_sd2d->getRecvIdPointer();
    for(int i=0; i<ntr; i++) m_recvidx[i] = p[i];

    float *fp = m_sd2d->getOffsetValuePointer();
    for(int i=0; i<ntr; i++) m_offsval[i] = fp[i];


    findDataRangeWithClipPercentage(m_aveamp, m_aveAmpRange);
    findDataRangeWithClipPercentage(m_absamp, m_absAmpRange);
    findDataRangeWithClipPercentage(m_rmsamp, m_rmsAmpRange);

    m_absAmpRange.lower = 0;
    m_rmsAmpRange.lower = 0;
    //cout << "m_aveAmpRange: lower="<< m_aveAmpRange.lower << " upper="<< m_aveAmpRange.upper << endl;
    //cout << "m_absAmpRange: lower="<< m_absAmpRange.lower << " upper="<< m_absAmpRange.upper << endl;
    //cout << "m_rmsAmpRange: lower="<< m_rmsAmpRange.lower << " upper="<< m_rmsAmpRange.upper << endl;
}

void Sdp2dAmplitudeDisplayDock::findDataRangeWithClipPercentage(QVector<double> amp,  QCPRange& range)
{
    int ntr = amp.size();

    float* temp = new float [ntr];
    for(int i=0; i< ntr; i++) temp[i] = amp.at(i);

    if(m_clipPercentage > 99.9){
        int idx = ntr-1;
        Sdp2dUtils::qkfind (idx, ntr, temp);
        range.upper =  temp[idx];

        idx = 1;
        Sdp2dUtils::qkfind (idx, ntr, temp);
        range.lower =  temp[idx];
    }else {
        int idx = int(ntr * m_clipPercentage / 100.0);
        Sdp2dUtils::qkfind (idx, ntr, temp);
        range.upper =  temp[idx];

        idx = int(ntr * (100. - m_clipPercentage) / 100.0);
        Sdp2dUtils::qkfind (idx, ntr, temp);
        range.lower =  temp[idx];
    }

    delete [] temp;

}

void Sdp2dAmplitudeDisplayDock::createPlotArea(QLayout *parent)
{
    m_customPlot = new QCustomPlot();
    m_customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes );
    parent->addWidget(m_customPlot);
    //m_customPlot->plotLayout()->insertRow(0);
    //m_customPlot->plotLayout()->addElement(0, 0, new QCPTextElement(m_customPlot, "Amplite of traces", QFont("sans", 12, QFont::Bold)));

    m_customPlot->axisRect()->setupFullAxesBox(true);

    m_colorScale = new QCPColorScale(m_customPlot);
    m_customPlot->plotLayout()->addElement(0, 1, m_colorScale); // add it to the right of the main axis rect        
    QCPColorGradient a(QCPColorGradient::gpPolar);
    m_colorScale->setGradient(a);    
    m_colorScale->setDataRange(m_rmsAmpRange);
    m_colorScale->axis()->setLabel("Seismic RMS Amplitude");
    m_colorScale->axis()->setNumberFormat("ebc");
    m_colorScale->axis()->setNumberPrecision(1);

    QCPMarginGroup* marginGroup = new QCPMarginGroup(m_customPlot);
    m_customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    m_colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);

    m_customPlot->xAxis->setBasePen(QPen(Qt::black, 1));
    m_customPlot->xAxis->setVisible(true);
    m_customPlot->xAxis->setTickLength(0, 5);
    m_customPlot->xAxis->setSubTickLength(0, 3);
    m_customPlot->xAxis->setTickLabels(false);
    m_customPlot->xAxis->setLabel("Shot Index");
    m_customPlot->xAxis->setTickLabels(true);

    m_customPlot->xAxis2->setVisible(true);    
    m_customPlot->xAxis2->ticker()->setTickCount(9);
    m_customPlot->xAxis2->setBasePen(QPen(Qt::black, 2));
    m_customPlot->xAxis2->setTickPen(QPen(Qt::black, 2));
    m_customPlot->xAxis2->setSubTickPen(QPen(Qt::black, 1));
    m_customPlot->xAxis2->setTickLabelColor(Qt::black);
    m_customPlot->xAxis2->setTickLength(0, 5);
    m_customPlot->xAxis2->setSubTickLength(0, 3);

    m_customPlot->yAxis->setLabel("Offset(m)");

    m_customPlot->yAxis->ticker()->setTickCount(9);
    m_customPlot->yAxis->setVisible(true);    
    m_customPlot->yAxis->setBasePen(QPen(Qt::black, 2));
    m_customPlot->yAxis->setTickPen(QPen(Qt::black, 2));
    m_customPlot->yAxis->setSubTickPen(QPen(Qt::black, 1));
    m_customPlot->yAxis->setTickLabelColor(Qt::black);
    m_customPlot->yAxis->setTickLength(0, 5);
    m_customPlot->yAxis->setSubTickLength(0, 3);
    m_customPlot->yAxis->grid()->setPen(QPen(QColor(140, 140, 140), 1, Qt::DotLine));
    //m_customPlot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    //m_customPlot->yAxis->grid()->setSubGridVisible(true);

    m_customPlot->yAxis2->setVisible(true);
    m_customPlot->yAxis2->setTickLength(0, 5);
    m_customPlot->yAxis2->setSubTickLength(0, 3);

    float minoff = m_sd2d->getMinOffset();
    float maxoff = m_sd2d->getMaxOffset();
    if(minoff < 0 ) minoff *= 1.2;
    else minoff *= 0.8;

    if(maxoff < 0 ) maxoff *= 0.8;
    else maxoff *= 1.2;

    m_customPlot->yAxis->setRange(minoff, maxoff);
    m_customPlot->yAxis2->setRange(minoff, maxoff);

    int minIdx = m_sd2d->getMinShotIndex() * 0.5;
    int maxIdx = m_sd2d->getMaxShotIndex() *1.1;
    m_customPlot->xAxis->setRange(minIdx, maxIdx);
    m_customPlot->xAxis2->setRange(minIdx, maxIdx);

    m_attScatters = new Sdp2dAmpDisplayQCPGraph(m_customPlot->xAxis, m_customPlot->yAxis);
    m_attScatters->setLineStyle(QCPGraph::lsNone);
    m_attScatters->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, m_markerSize));
    m_attScatters->setColorScale(m_colorScale);
    m_attScatters->setTraceFlagForScatters(m_sd2d->getTraceFlags());

    m_currentPoint = new QCPGraph(m_customPlot->xAxis, m_customPlot->yAxis);
    m_currentPoint->setLineStyle(QCPGraph::lsNone);
    m_currentPoint->setBrush(QBrush(QColor(Qt::green)));
    m_currentPoint->setPen(QPen(QColor(Qt::green)));
    m_currentPoint->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 12));

}

void Sdp2dAmplitudeDisplayDock::drawGraphy()
{
    if(m_xaxisType == 0){
        m_attScatters->setData(m_shotidx, m_offsval, true);
    }else if(m_xaxisType == 1){
        m_attScatters->setData(m_cdpsidx, m_offsval, true);
    }else if(m_xaxisType == 2){
        m_attScatters->setData(m_recvidx, m_offsval, true);
    }
    if(m_attType==0) {
        m_attScatters->setColorScaleDataForScatters(m_aveamp.data());
    } else if(m_attType==1) {
        m_attScatters->setColorScaleDataForScatters(m_absamp.data());
    } else if(m_attType==2){
        m_attScatters->setColorScaleDataForScatters(m_rmsamp.data());
    } else if(m_attType==3){
        generateMuteScatterValues(1);
    } else if(m_attType==4){
        generateMuteScatterValues(2);
    }
    m_customPlot->replot();
}

void Sdp2dAmplitudeDisplayDock::replot()
{
    m_customPlot->replot();
}

void Sdp2dAmplitudeDisplayDock::generateMuteScatterValues(int muteType)
{
    int ntr = m_sd2d->getNumberOfTraces();
    double dt = double(m_sd2d->getTimeSampleRateInUs())/1000000.0;
    short* mutes = nullptr;
    if(muteType ==  MuteType::TopMute){
        mutes = m_sd2d->getTopMuteForAllTraces();
    } else {
        mutes = m_sd2d->getBtmMuteForAllTraces();
    }
    for(int i=0; i<ntr; i++) m_mutetime[i] = mutes[i]*dt;    

    m_attScatters->setColorScaleDataForScatters(m_mutetime.data());
    m_customPlot->replot(QCustomPlot::rpImmediateRefresh);
    //cout << "in generateMuteScatterValues" << endl;
}

void Sdp2dAmplitudeDisplayDock::zoomFitBtnClicked()
{
    int minIdx=0, maxIdx=0;
    if( m_xaxisType == 0) {
        minIdx = m_sd2d->getMinShotIndex() * 0.5;
        maxIdx = m_sd2d->getMaxShotIndex() * 1.05;
    } else if( m_xaxisType == 1) {
        minIdx = m_sd2d->getMinCDPIndex() * 0.5;
        maxIdx = m_sd2d->getMaxCDPIndex() * 1.05;
    } else if( m_xaxisType == 2) {
        minIdx = m_sd2d->getMinRecvIndex() * 0.5;
        maxIdx = m_sd2d->getMaxRecvIndex() * 1.05;
    }
    m_customPlot->xAxis->setRange(minIdx, maxIdx);
    m_customPlot->xAxis2->setRange(minIdx, maxIdx);

    float minoff = m_sd2d->getMinOffset();
    float maxoff = m_sd2d->getMaxOffset();
    if(minoff < 0 ) minoff *= 1.2;
    else minoff *= 0.8;

    if(maxoff < 0 ) maxoff *= 0.8;
    else maxoff *= 1.2;

    m_customPlot->yAxis->setRange(minoff, maxoff);
    m_customPlot->yAxis2->setRange(minoff, maxoff);

    m_customPlot->replot();
}

void Sdp2dAmplitudeDisplayDock::amplitudeClipPercentageChanged(QString text)
{
    m_clipPercentage = text.toFloat();

    findDataRangeWithClipPercentage(m_aveamp, m_aveAmpRange);
    findDataRangeWithClipPercentage(m_absamp, m_absAmpRange);
    findDataRangeWithClipPercentage(m_rmsamp, m_rmsAmpRange);
    m_absAmpRange.lower = 0;
    m_rmsAmpRange.lower = 0;
    if(m_attType==0) {
        m_colorScale->setDataRange(m_aveAmpRange);
    } else if(m_attType==1) {
        m_colorScale->setDataRange(m_absAmpRange);
    } else if(m_attType==2){
        m_colorScale->setDataRange(m_rmsAmpRange);
    } else if(m_attType==3 || m_attType==4) {
        m_colorScale->setDataRange(m_muteRange);
    }

    m_customPlot->replot();
}

void Sdp2dAmplitudeDisplayDock::amplitudeMarkerScaleChanged(QString text)
{
    m_markerSize = text.toFloat();
    m_attScatters->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, m_markerSize));
    m_customPlot->replot();
}

void Sdp2dAmplitudeDisplayDock::amplitudeColormapChanged(QString text)
{
    int colormapIdx = QCPColorGradient::gpGrayscale;
    if(text.compare("RedBlue")==0)        colormapIdx=QCPColorGradient::gpBwr;
    else if(text.compare("Hot")==0)       colormapIdx=QCPColorGradient::gpHot;
    else if(text.compare("Cold")==0)      colormapIdx=QCPColorGradient::gpCold;
    else if(text.compare("Night")==0)     colormapIdx=QCPColorGradient::gpNight;
    else if(text.compare("Candy")==0)     colormapIdx=QCPColorGradient::gpCandy;
    else if(text.compare("Geography")==0) colormapIdx=QCPColorGradient::gpGeography;
    else if(text.compare("Ion")==0)       colormapIdx=QCPColorGradient::gpIon;
    else if(text.compare("Thermal")==0)   colormapIdx=QCPColorGradient::gpThermal;
    else if(text.compare("Polar")==0)     colormapIdx=QCPColorGradient::gpPolar;
    else if(text.compare("Spectrum")==0)  colormapIdx=QCPColorGradient::gpSpectrum;
    else if(text.compare("Jet")==0)       colormapIdx=QCPColorGradient::gpJet;
    else if(text.compare("Hues")==0)      colormapIdx=QCPColorGradient::gpHues;

    if(colormapIdx == QCPColorGradient::gpCold){
        m_currentPoint->setBrush(QBrush(QColor(Qt::red)));
        m_currentPoint->setPen(QPen(QColor(Qt::red)));
    } else if(colormapIdx <= QCPColorGradient::gpGeography){
        m_currentPoint->setBrush(QBrush(QColor(Qt::green)));
        m_currentPoint->setPen(QPen(QColor(Qt::green)));
    } else if(colormapIdx != QCPColorGradient::gpThermal){
        m_currentPoint->setBrush(QBrush(QColor(Qt::white)));
        m_currentPoint->setPen(QPen(QColor(Qt::white)));
    } else {
        m_currentPoint->setBrush(QBrush(QColor(Qt::green)));
        m_currentPoint->setPen(QPen(QColor(Qt::green)));
    }

    QCPColorGradient a((QCPColorGradient::GradientPreset)colormapIdx);
    m_colorScale->setGradient(a);
    m_customPlot->replot();
}

void Sdp2dAmplitudeDisplayDock::onMouseMove(QMouseEvent *event)
{
    double x, y, key, val;
    x = event->pos().x();
    y = event->pos().y();

    m_attScatters->pixelsToCoords(x, y, key, val);
    if(key < m_customPlot->xAxis->range().lower || key > m_customPlot->xAxis->range().upper) return;
    if(val < m_customPlot->yAxis->range().lower || val > m_customPlot->yAxis->range().upper) return;

    int trIdx = -1;
    int trSeq = -1;
    int index = int(key+0.5);
    float offset = val;
    float tol = 2;

    switch (m_xaxisType) {
    case 0:
        trIdx = m_sd2d->findTraceIndexWithShotIdxAndOffset(index, offset, trSeq, tol);
        break;
    case 1:
        trIdx = m_sd2d->findTraceIndexWithCdpIdxAndOffset(index, offset, trSeq, tol);
        break;
    case 2:
        trIdx = m_sd2d->findTraceIndexWithRecvIdxAndOffset(index, offset, trSeq, tol);
        break;
    }
    if(trIdx < 0 || trSeq < 0) {
        m_statusbar->clearMessage();
    }else{        
        m_statusbar->showMessage(QString("ShotIndex: %1,  CDPIndex: %2, ReceiverIndex: %3, Offset: %4, AveAmplitude: %5, AbsAmplitude: %6, RMSAmplitude: %7")
                                 .arg(m_shotidx[trIdx]).arg(m_cdpsidx[trIdx]).arg(m_recvidx[trIdx]).arg(m_offsval[trIdx])
                                 .arg(m_aveamp[trIdx]).arg(m_absamp[trIdx]).arg(m_rmsamp[trIdx]));
    }

    //cout << " x="<< x << " y=" << y << " key=" << key << " val=" << val << endl;
}

void Sdp2dAmplitudeDisplayDock::onMouseDoubleClick(QMouseEvent *event)
{
    if(m_sd2d != m_mainWindow->getCurrentDataPointer()) {
        QPoint globalPos = this->mapToGlobal(event->pos());
        QMessageBox message(QMessageBox::Information,
             "Warn", QString("This amplitude figure is not belong to the current displayed seismic data."), QMessageBox::Ok, NULL);
        message.move(globalPos);
        message.exec();
        return;
    }

    if(m_sd2d->getInteractiveFunction() ==InteractiveFunctions::StackVelAnalysis) return;

    double x, y, key, val;
    x = event->pos().x();
    y = event->pos().y();

    m_attScatters->pixelsToCoords(x, y, key, val);
    if(key < m_customPlot->xAxis->range().lower || key > m_customPlot->xAxis->range().upper) return;
    if(val < m_customPlot->yAxis->range().lower || val > m_customPlot->yAxis->range().upper) return;

    int trIdx = -1;
    int trSeq = -1;

    int index = int(key+0.5);
    float offset = val;    

    //int height = m_customPlot->yAxis->axisRect()->height();
    //float ry = m_customPlot->yAxis->range().size();
    //float tol = height/ry*0.5;
    //if(tol < 1) tol = 2;
    float tol = 2;

    switch (m_xaxisType) {
    case 0:
        trIdx = m_sd2d->findTraceIndexWithShotIdxAndOffset(index, offset, trSeq, tol);
        break;
    case 1:
        trIdx = m_sd2d->findTraceIndexWithCdpIdxAndOffset(index, offset, trSeq, tol);
        break;
    case 2:
        trIdx = m_sd2d->findTraceIndexWithRecvIdxAndOffset(index, offset, trSeq, tol);
        break;
    }
    if(trIdx < 0 || trSeq < 0) {        
        return;
    }
    m_trIdx = trIdx;
    m_trSeq = trSeq;

    displayCurrentPointLocation();
    m_customPlot->replot();
}

void Sdp2dAmplitudeDisplayDock::displayCurrentPointLocation()
{
    if(m_trIdx < 0) return;
    int gatherType = 0;
    int gatherIdx = -1;
    int traceIdx = m_trSeq+1;
    switch (m_xaxisType) {
    case 0:
        gatherType = DisplayGatherType::CommonShot;
        gatherIdx = m_shotidx[m_trIdx];
        break;
    case 1:
        gatherType = DisplayGatherType::CommonDepthPoint;
        gatherIdx = m_cdpsidx[m_trIdx];
        break;
    case 2:
        gatherType = DisplayGatherType::CommonReceiver;
        gatherIdx = m_recvidx[m_trIdx];
        break;
    }
    m_sd2d->setGatherDisplayWithHelightTrace(gatherType, gatherIdx, traceIdx);
    m_pointKey[0] = gatherIdx;
    m_pointVal[0] = m_offsval[m_trIdx];
    m_currentPoint->setData(m_pointKey, m_pointVal, true);
    m_currentPoint->setVisible(true);
}

void Sdp2dAmplitudeDisplayDock::setDataForCurrentTracePoint(int traceIdx)
{
    m_trIdx = traceIdx;
    int gatherIdx = -1;
    switch (m_xaxisType) {
    case 0:
        gatherIdx = m_shotidx[m_trIdx];
        break;
    case 1:
        gatherIdx = m_cdpsidx[m_trIdx];
        break;
    case 2:
        gatherIdx = m_recvidx[m_trIdx];
        break;
    }

    m_pointKey[0] = gatherIdx;
    m_pointVal[0] = m_offsval[m_trIdx];

    m_currentPoint->setData(m_pointKey, m_pointVal, true);
    m_currentPoint->setVisible(true);
    m_customPlot->replot();
}

void Sdp2dAmplitudeDisplayDock::hideTraceLocationPoint()
{
    m_currentPoint->setVisible(false);
    m_sd2d->getInputGatherDisplayPointer()->removeHighLightedTrace();
    m_customPlot->replot();
}

void Sdp2dAmplitudeDisplayDock::onCustomContextMenuRequested(const QPoint &pos)
{
    if(m_trIdx < 0 || m_trSeq < 0) return;
    QPoint globalPos = this->mapToGlobal(pos);
    m_myMenu->exec(globalPos);
}

void Sdp2dAmplitudeDisplayDock::showTraceInShotGather(void)
{
    int gatherType = DisplayGatherType::CommonShot;
    int gatherIdx = m_shotidx[m_trIdx];
    int traceIdx  = m_sd2d->findTraceSequenceWithinGather(gatherType, gatherIdx, m_trIdx);
    if(traceIdx == 0) return;
    m_sd2d->setGatherDisplayWithHelightTrace(gatherType, gatherIdx, traceIdx);
}

void Sdp2dAmplitudeDisplayDock::showTraceInCDPsGather(void)
{
    int gatherType = DisplayGatherType::CommonDepthPoint;
    int gatherIdx = m_cdpsidx[m_trIdx];
    int traceIdx  = m_sd2d->findTraceSequenceWithinGather(gatherType, gatherIdx, m_trIdx);;
    if(traceIdx == 0) return;
    m_sd2d->setGatherDisplayWithHelightTrace(gatherType, gatherIdx, traceIdx);
}

void Sdp2dAmplitudeDisplayDock::showTraceInRecvGather(void)
{
    int gatherType = DisplayGatherType::CommonReceiver;
    int gatherIdx = m_recvidx[m_trIdx];
    int traceIdx  = m_sd2d->findTraceSequenceWithinGather(gatherType, gatherIdx, m_trIdx);
    if(traceIdx == 0) return;
    m_sd2d->setGatherDisplayWithHelightTrace(gatherType, gatherIdx, traceIdx);
}

void Sdp2dAmplitudeDisplayDock::showTraceInOffsetGather(void)
{
    int gatherType = DisplayGatherType::CommonOffset;
    int gatherIdx = m_sd2d->getOIdxOfATrace(m_trIdx);
    int traceIdx  = m_sd2d->findTraceSequenceWithinGather(gatherType, gatherIdx, m_trIdx);
    if(traceIdx == 0) return;
    m_sd2d->setXAxisType(1);
    m_sd2d->setGatherDisplayWithHelightTrace(gatherType, gatherIdx, traceIdx);
}

