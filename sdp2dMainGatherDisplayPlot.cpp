#include "sdp2dMainGatherDisplayPlot.h"
#include "sdp2dMainGatherDisplayArea.h"
#include "seismicdataprocessing2d.h"
#include "seismicdata2d.h"
#include "qcustomplot.h"

#include <QtGlobal>
#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QShowEvent>
#include <QMouseEvent>


#include <iostream>
using namespace std;

Sdp2dMainGatherDisplayPlot::Sdp2dMainGatherDisplayPlot(QWidget *parent) : QCustomPlot(parent)
{
    setBackgroundRole(QPalette::Background);
    m_parent = dynamic_cast<Sdp2dMainGatherDisplayArea*>(parent);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionRectMode(QCP::srmSelect);
    setCursor(Qt::CrossCursor);

    QPen myPen;
    //myPen.setColor(QColor(Qt::red));
    myPen.setColor(QColor(230, 128, 255));
    //myPen.setColor(QColor(255, 200, 20, 200));
    myPen.setStyle(Qt::DashLine);
    myPen.setWidthF(4);

    mSelectionRect->setPen(myPen);

    m_mouseLeftPressed = false;
    m_freqanaType = FreqAnaType::NoSelection;
    m_startPos = QPoint(0, 0);
    m_endPos = QPoint(0, 0);
    m_selectedRect = QRect(m_startPos, m_endPos);

    createPopupMenus();
    connect(m_parent, &Sdp2dMainGatherDisplayArea::gatherChanged, this, &Sdp2dMainGatherDisplayPlot::displayedGatherChanged);
    connect(this, &QWidget::customContextMenuRequested, this, &Sdp2dMainGatherDisplayPlot::on_display_customContextMenuRequested);
    connect(this, &QCustomPlot::mouseDoubleClick, this, &Sdp2dMainGatherDisplayPlot::onMouseDoubleClicked);
}

Sdp2dMainGatherDisplayPlot::~Sdp2dMainGatherDisplayPlot()
{

}

void Sdp2dMainGatherDisplayPlot::createPopupMenus(void)
{    
    m_badTraceMenu = new QMenu(this);
    QAction* showSrc = m_badTraceMenu->addAction("Show Trace in Shot Gather", this, &Sdp2dMainGatherDisplayPlot::showTraceInShotGather);
    QAction* showCdp = m_badTraceMenu->addAction("Show Trace in CDP Gather", this, &Sdp2dMainGatherDisplayPlot::showTraceInCDPsGather);
    QAction* showRec = m_badTraceMenu->addAction("Show Trace in Receiver Gather", this, &Sdp2dMainGatherDisplayPlot::showTraceInRecvGather);
    QAction* showOff = m_badTraceMenu->addAction("Show Trace in Offset Gather", this, &Sdp2dMainGatherDisplayPlot::showTraceInOffsetGather);

    m_stFreqAna = new QAction("Frequency Analysis for Single Trace", this);
    m_stFreqAna->setCheckable(true);
    m_stFreqAna->setChecked(false);

    m_raFreqAna = new QAction("Frequency Analysis for Rectangular Area", this);
    m_raFreqAna->setCheckable(true);
    m_raFreqAna->setChecked(false);

    m_wgFreqAna = new QAction("Frequency Analysis for Whole Gather", this);
    m_wgFreqAna->setCheckable(true);
    m_wgFreqAna->setChecked(false);

    m_hideGather = new QAction("Hide the original gather", this);
    m_hideGather->setCheckable(true);
    m_hideGather->setChecked(false);
    m_hideGather->setDisabled(true);

    m_freqAnaMenu = new QMenu(this);
    m_freqAnaMenu->addAction(m_stFreqAna);
    m_freqAnaMenu->addAction(m_raFreqAna);
    m_freqAnaMenu->addAction(m_wgFreqAna);
    m_freqAnaMenu->addSeparator();
    m_freqAnaMenu->addAction(showSrc);
    m_freqAnaMenu->addAction(showCdp);
    m_freqAnaMenu->addAction(showRec);
    m_freqAnaMenu->addAction(showOff);
    m_freqAnaMenu->addSeparator();
    m_freqAnaMenu->addAction(m_hideGather);
    m_freqAnaMenu->addSeparator();
    m_saveTraces = m_freqAnaMenu->addAction("Replace with filtered data", m_parent, &Sdp2dMainGatherDisplayArea::replaceWithFilteredData);
    m_loadTraces = m_freqAnaMenu->addAction("Restore raw data", m_parent, &Sdp2dMainGatherDisplayArea::restoreRawData);
    m_saveTraces->setDisabled(true);
    m_loadTraces->setDisabled(true);

    QActionGroup* fanaGroup = new QActionGroup(this);
    fanaGroup->addAction(m_stFreqAna);
    fanaGroup->addAction(m_raFreqAna);
    fanaGroup->addAction(m_wgFreqAna);
    fanaGroup->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);

    connect(m_stFreqAna,  &QAction::toggled, this, &Sdp2dMainGatherDisplayPlot::singleTraceFreqAna);
    connect(m_raFreqAna,  &QAction::toggled, this, &Sdp2dMainGatherDisplayPlot::rectAreaFreqAna);
    connect(m_wgFreqAna,  &QAction::toggled, this, &Sdp2dMainGatherDisplayPlot::wholeGatherFreqAna);
    connect(m_hideGather, &QAction::toggled, this, &Sdp2dMainGatherDisplayPlot::hideGatherFreqAna);

    m_mutePickMenu = new QMenu(this);
    m_topMutePick = new QAction("Pick top mute", this);
    m_topMutePick->setCheckable(true);
    m_topMutePick->setChecked(true);

    m_btmMutePick = new QAction("Pick bottom mute", this);
    m_btmMutePick->setCheckable(true);
    m_btmMutePick->setChecked(false);

    m_applyTopMute = new QAction("Apply top mute on data", this);
    m_applyTopMute->setCheckable(true);
    m_applyTopMute->setChecked(false);

    m_applyBtmMute = new QAction("Apply bottom mute on data", this);
    m_applyBtmMute->setCheckable(true);
    m_applyBtmMute->setChecked(false);

    m_hideBadTraces = new QAction("Hide bad traces", this);
    m_hideBadTraces->setCheckable(true);
    m_hideBadTraces->setChecked(false);

    m_mutePickMenu->addAction(m_topMutePick);
    m_mutePickMenu->addAction(m_btmMutePick);
    m_mutePickMenu->addSeparator();
    m_mutePickMenu->addAction(m_applyTopMute);
    m_mutePickMenu->addAction(m_applyBtmMute);
    m_mutePickMenu->addSeparator();
    m_mutePickMenu->addAction(showSrc);
    m_mutePickMenu->addAction(showCdp);
    m_mutePickMenu->addAction(showRec);
    m_mutePickMenu->addAction(showOff);

    QActionGroup* muteGroup = new QActionGroup(this);
    muteGroup->addAction(m_topMutePick);
    muteGroup->addAction(m_btmMutePick);

    m_measureLVel = new QAction("Measure linear velocity", this);
    m_measureLVel->setCheckable(true);
    m_measureLVel->setChecked(false);

    m_noFuncMenu = new QMenu(this);
    m_noFuncMenu->addAction(showSrc);
    m_noFuncMenu->addAction(showCdp);
    m_noFuncMenu->addAction(showRec);
    m_noFuncMenu->addAction(showOff);
    m_noFuncMenu->addSeparator();
    m_noFuncMenu->addAction(m_applyTopMute);
    m_noFuncMenu->addAction(m_applyBtmMute);
    m_noFuncMenu->addAction(m_hideBadTraces);
    m_noFuncMenu->addSeparator();
    m_noFuncMenu->addAction(m_measureLVel);

    connect(m_topMutePick, &QAction::toggled, m_parent, &Sdp2dMainGatherDisplayArea::setTopMute);
    connect(m_btmMutePick, &QAction::toggled, m_parent, &Sdp2dMainGatherDisplayArea::setBtmMute);
    connect(m_applyTopMute, &QAction::toggled, m_parent, &Sdp2dMainGatherDisplayArea::applyTopMuteOnData);
    connect(m_applyBtmMute, &QAction::toggled, m_parent, &Sdp2dMainGatherDisplayArea::applyBtmMuteOnData);
    connect(m_hideBadTraces, &QAction::toggled, m_parent, &Sdp2dMainGatherDisplayArea::hideBadTraces);
    connect(m_measureLVel, &QAction::toggled, m_parent, &Sdp2dMainGatherDisplayArea::checkMeasureLinearVelocity);
}

void Sdp2dMainGatherDisplayPlot::enableSaveActionForFilter(bool enable)
{
    m_loadTraces->setDisabled(enable);
}

void Sdp2dMainGatherDisplayPlot::showEvent(QShowEvent *event)
{
    QCustomPlot::showEvent(event);

    m_freqanaType = m_parent->getFrequencyAnaType();
    int interactFun = m_parent->getInteractiveFunction();
    //cout << "Sdp2dMainGatherDisplayPlot::showEvent" << " m_freqanaType=" << m_freqanaType << endl;

    if(InteractiveFunctions::FrequencyAnalysis == interactFun){
        switch (m_freqanaType) {
        case FreqAnaType::SingleTrace:
            m_stFreqAna->setChecked(true);
            break;
        case FreqAnaType::RectArea:
            m_raFreqAna->setChecked(true);
            break;
        case FreqAnaType::WholeGather:
            m_wgFreqAna->setChecked(true);
            break;
        }
    }
}

void Sdp2dMainGatherDisplayPlot::mouseMoveEvent(QMouseEvent *event)
{
    QPointF tt = m_parent->convertPosToAxisValues(QPointF(event->pos().x(), event->pos().y()));
    //cout << "mouseMoveEvent posX=" << event->pos().x() << " posY=" << event->pos().y() <<  " tt.x=" <<  tt.x() << endl;
    int interactFun = m_parent->getInteractiveFunction();

    bool showFlag = false;
    if(m_parent->getTraceHeaderDisplay()){
        showFlag = m_parent->mouseMoveOnTheHeaderDisplay(event->pos());
    }

    if(tt.x() < 0 || tt.x() > m_parent->getNumTracesOfCurrentGather() || tt.y() < -0.01) {
        if(m_mouseLeftPressed){
            //cout << "outside the gather display" << endl;
            m_mouseLeftPressed = false;
            if (mSelectionRect && mSelectionRect->isActive()) {
                mSelectionRect->cancel();
                replot(rpQueuedReplot);
            }
            m_startPos = QPoint(0, 0);
            m_endPos = QPoint(0, 0);
            m_minSelectedTraceSeq = m_parent->getNumTracesOfCurrentGather();;
            m_maxSelectedTraceSeq = 1;
            m_startTraceSeq = -1;
            m_parent->cleanTempGraphs();
        }
        if(!showFlag) m_parent->getStatusBar()->clearMessage();
        return;
    }

    if(m_mouseLeftPressed){        
        if(interactFun == InteractiveFunctions::FrequencyAnalysis && m_freqanaType == FreqAnaType::RectArea) {            
            QCustomPlot::mouseMoveEvent(event);
        }else if(interactFun == InteractiveFunctions::BadTraceSelection){
            badTraceSelectionLeftMouseMove(event);
        }else if(interactFun == InteractiveFunctions::MeasureLinearVelcity){
            measureLinearVelLeftMouseMove(event);
        }
    }

    if(!showFlag && !m_parent->mouseMoveOnTheGather(tt.x(), tt.y())) m_parent->getStatusBar()->clearMessage();
}

void Sdp2dMainGatherDisplayPlot::mousePressEvent(QMouseEvent *event)
{
    QPointF tt = m_parent->convertPosToAxisValues(QPointF(event->pos().x(), event->pos().y()));
    if(tt.x() < 0 || tt.x() > m_parent->getNumTracesOfCurrentGather() ) return;
    if(tt.y() < -0.01 || tt.y() > m_parent->getMaxDisplayTime() + 0.01) return;

    //cout << "mousePressEvent: x=" << tt.x() << " y="<< tt.y() << endl;

    int interactFun = m_parent->getInteractiveFunction();
    //cout << "interactFun = " << interactFun << endl;

    if(event->button() == Qt::LeftButton){        
        m_mouseLeftPressed = true;
        m_bkPos = m_startPos;
        m_startPos = event->pos();

        if(interactFun == InteractiveFunctions::BadTraceSelection){            
            badTraceSelectionLeftMousePress(event);
        }else if(interactFun == InteractiveFunctions::FrequencyAnalysis){
            frequencyAnalysisLeftMousePress(event);
        }else if(interactFun == InteractiveFunctions::MeasureLinearVelcity){
            measureLinearVelLeftMousePress(event);
        }

    }else if(event->button() == Qt::MidButton){        
        //cout << "Mouse middle button pressed" << endl;
    }else if(event->button() == Qt::RightButton){
        //cout << "Mouse right button pressed" << endl;
    }
    QCustomPlot::mousePressEvent(event);
}

void Sdp2dMainGatherDisplayPlot::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF tt = m_parent->convertPosToAxisValues(QPointF(event->pos().x(), event->pos().y()));
    if(tt.x() < 0 || tt.x() > m_parent->getNumTracesOfCurrentGather() || tt.y() < -0.01) return;

    //cout << "mouseReleaseEvent: x=" << tt.x() << " y="<< tt.y() << endl;
    int interactFun = m_parent->getInteractiveFunction();

    if(event->button() == Qt::LeftButton){
        m_mouseLeftPressed = false;
        if(interactFun == InteractiveFunctions::FrequencyAnalysis  ){
            frequencyAnalysisLeftMouseRelease(event);
        }else if(interactFun == InteractiveFunctions::BadTraceSelection){
            badTraceSelectionLeftMouseRelease(event);
        }else if(interactFun == InteractiveFunctions::PickingMute){
            pickingMuteLeftMouseRelease(event);
        }else if(interactFun == InteractiveFunctions::MeasureLinearVelcity){
            measureLinearVelLeftMouseRelease(event);
        }
    }else if(event->button() == Qt::MidButton){
        if(interactFun == InteractiveFunctions::PickingMute){
            pickingMuteMiddleMouseRelease(event);
        }
        //cout << "Mouse middle button released" << endl;
    }else if(event->button() == Qt::RightButton){        
        //cout << "Mouse right button released" << endl;
    }
}

void Sdp2dMainGatherDisplayPlot::onMouseDoubleClicked(QMouseEvent *event)
{
    QPointF tt = m_parent->convertPosToAxisValues(QPointF(event->pos().x(), event->pos().y()));
    if(tt.x() < 0 || tt.x() > m_parent->getNumTracesOfCurrentGather() || tt.y() < 0) return;
    int interactFun = m_parent->getInteractiveFunction();

    if(event->button() == Qt::LeftButton){        
        switch(interactFun){
            case InteractiveFunctions::FrequencyAnalysis:
                frequencyAnalysisLeftMouseDoubleClicked(event);
                break;
            case InteractiveFunctions::BadTraceSelection:
                badTraceSelectionLeftMouseDoubleClicked(event);
                break;
            case InteractiveFunctions::StackVelAnalysis:
                StackVelAnalysisLeftMouseDoubleClicked(event);
                break;
            case InteractiveFunctions::PickingMute:
                pickingMuteLeftMouseDoubleClicked(event);
                break;
            default:
                noFunctionLeftMouseDoubleClicked(event);
                break;
        }
    }else if(event->button() == Qt::MidButton){
        //cout << "Mouse middle button doubleClicked" << endl;
        if(interactFun == InteractiveFunctions::FrequencyAnalysis){
            if(m_parent->hasTempGraphs()) m_hideGather->setChecked(!m_hideGather->isChecked());
        }
    }else if(event->button() == Qt::RightButton){
        //cout << "Mouse right button doubleClicked" << endl;
    }
}

void Sdp2dMainGatherDisplayPlot::on_display_customContextMenuRequested(const QPoint &pos)
{
    double x = pos.x();
    double y = pos.y();

    QPointF tt = m_parent->convertPosToAxisValues(QPointF(x, y));
    if(tt.x() < 0 || tt.x() > m_parent->getNumTracesOfCurrentGather() || tt.y() < 0) return;
    m_startTraceSeq = int(tt.x()) - 1;    // trace index = trace Sequence + 1

    QPoint globalPos = this->mapToGlobal(pos);

    int interactFun = m_parent->getInteractiveFunction();
    m_measureLVel->setVisible(false);
    if(interactFun != InteractiveFunctions::MeasureLinearVelcity) m_measureLVel->setChecked(false);

    if(interactFun == InteractiveFunctions::FrequencyAnalysis){        
        m_freqAnaMenu->exec(globalPos);        
    } else if(interactFun == InteractiveFunctions::BadTraceSelection){
        m_badTraceMenu->exec(globalPos);
    } else if(interactFun == InteractiveFunctions::StackVelAnalysis){

    } else if(interactFun == InteractiveFunctions::PickingMute){
        m_mutePickMenu->exec(globalPos);
    } else {
        if(m_parent->getGatherType() != DisplayGatherType::CommonOffset){
            m_measureLVel->setVisible(true);
        }
        m_noFuncMenu->exec(globalPos);
    }
    if(interactFun != InteractiveFunctions::FrequencyAnalysis){
        if (mSelectionRect && mSelectionRect->isActive()) {
            mSelectionRect->cancel();
            replot(rpQueuedReplot);
        }
    }
}

void Sdp2dMainGatherDisplayPlot::singleTraceFreqAna(bool checked)
{    
    cleanSelection();

    if(checked){
        m_freqanaType = FreqAnaType::SingleTrace;
    } else {
        m_freqanaType = FreqAnaType::NoSelection;
        m_parent->setTraceVisible(false);
    }
    m_parent->setFrequencyAnaType(m_freqanaType); 
}

void Sdp2dMainGatherDisplayPlot::rectAreaFreqAna(bool checked)
{
    m_parent->setTraceVisible(false);
    cleanSelection();
    if(!checked){
        m_freqanaType = FreqAnaType::NoSelection;
    }else {                
        m_freqanaType = FreqAnaType::RectArea;
    }
    m_parent->setFrequencyAnaType(m_freqanaType);
}

void Sdp2dMainGatherDisplayPlot::wholeGatherFreqAna(bool checked)
{
    m_parent->setTraceVisible(false);
    cleanSelection();
    if(checked){
        m_freqanaType = FreqAnaType::WholeGather;
        setSelectedRectForWholeGather(m_selectedRect);
    } else {
        m_freqanaType = FreqAnaType::NoSelection;
    }
    m_parent->setFrequencyAnaType(m_freqanaType);    
}

void Sdp2dMainGatherDisplayPlot::hideGatherFreqAna(bool hide)
{    
    m_parent->hideInputGather(hide);
    //m_parent->displayProcessedSeismicTraces();
    replotSelection();
}

void Sdp2dMainGatherDisplayPlot::disableHideGather(bool hide)
{
    if(hide){
        m_hideGather->setChecked(false);
    }
    m_hideGather->setDisabled(hide);
    m_saveTraces->setDisabled(hide);
    m_loadTraces->setDisabled(hide);
}


void Sdp2dMainGatherDisplayPlot::showTraceInShotGather()
{
    int gatherType = DisplayGatherType::CommonShot;
    m_parent->setGatherDisplayWithHelightTrace(gatherType, m_startTraceSeq+1);
}

void Sdp2dMainGatherDisplayPlot::showTraceInCDPsGather()
{
    int gatherType = DisplayGatherType::CommonDepthPoint;
    m_parent->setGatherDisplayWithHelightTrace(gatherType, m_startTraceSeq+1);
}

void Sdp2dMainGatherDisplayPlot::showTraceInRecvGather()
{
    int gatherType = DisplayGatherType::CommonReceiver;
    m_parent->setGatherDisplayWithHelightTrace(gatherType, m_startTraceSeq+1);
}

void Sdp2dMainGatherDisplayPlot::showTraceInOffsetGather()
{
    int gatherType = DisplayGatherType::CommonOffset;
    m_parent->setGatherDisplayWithHelightTrace(gatherType, m_startTraceSeq+1);
}

void Sdp2dMainGatherDisplayPlot::frequencyAnalysisLeftMouseDoubleClicked(QMouseEvent *event)
{
    if(m_freqanaType == FreqAnaType::NoSelection && ! m_parent->hasHighLightTrace()) return;

    if(m_freqanaType == FreqAnaType::WholeGather){
        bool visible = m_parent->isTempGraphsVislible();
        m_parent->showTempGraphs(!visible);
    } else if(m_freqanaType == FreqAnaType::SingleTrace||  m_parent->hasHighLightTrace()) {
        if(m_parent->isSelectedRectChanged(m_selectedRect)){
            m_parent->cleanTempGraphs();
            double x = event->pos().x();
            double y = event->pos().y();
            QPointF tt = m_parent->convertPosToAxisValues(QPointF(x, y));
            int trIdx = int(tt.x());   // trIdx = trSeq +1;
            int ntr = m_parent->getNumTracesOfCurrentGather();
            if(trIdx < 1) trIdx = 1;
            if(trIdx > ntr) trIdx = ntr;

            QString label = QString("Gather %1, Trace %2").arg(m_parent->getGatherIndex()).arg(trIdx);
            m_parent->drawAndMapSingleWiggleTrace(trIdx);
            m_parent->frequencyAnalysis(m_selectedRect, label);
            if (mSelectionRect && mSelectionRect->isActive() && m_freqanaType != FreqAnaType::RectArea) {
                mSelectionRect->cancel();
                replot(rpQueuedReplot);
            }
        } else {
            bool visible = m_parent->isTempGraphsVislible();
            m_parent->showTempGraphs(!visible);
        }
    } else {
        if(!m_parent->isSelectedRectChanged(m_selectedRect)){
            bool visible = m_parent->isTempGraphsVislible();
            m_parent->showTempGraphs(!visible);
        }
    }

}

QRect Sdp2dMainGatherDisplayPlot::setSelectionRectForFrequencyAna(QMouseEvent *event)
{
    int staX = 1;
    int endX = m_parent->getNumTracesOfCurrentGather();
    int staY = 0;
    int endY = m_parent->getTimeSamplesOfCurrentView() - 1;

    QPoint start(staX, staY);
    QPoint end(endX, endY);
    QRect newRect = QRect(start, end);

    if(m_freqanaType == FreqAnaType::SingleTrace) {
        double x = event->pos().x();
        double y = event->pos().y();
        QPointF tt = m_parent->convertPosToAxisValues(QPointF(x, y));
        int trIdx = int(tt.x());   // trIdx = trSeq +1;
        int ntr = m_parent->getNumTracesOfCurrentGather();
        if(trIdx < 1) trIdx = 1;
        if(trIdx > ntr) trIdx = ntr;

        start.setX(trIdx);
        end.setX(trIdx);

        newRect = QRect(start, end);

    } else if(m_freqanaType == FreqAnaType::RectArea) {

        newRect = m_parent->getSeismicDataRange(m_startPos, m_endPos);

        if(newRect.height() < 15){
            QMessageBox message(QMessageBox::NoIcon,
                                "Warn", QString("The selected time window is too small for the frequency analysis! Number of time samples = %1").arg(newRect.height()), QMessageBox::Ok, NULL);
            message.exec();
            if (mSelectionRect && mSelectionRect->isActive()) {
                mSelectionRect->cancel();
                replot(rpQueuedReplot);
            }
            m_startPos = QPoint(0, 0);
            m_endPos = QPoint(0, 0);
            return QRect(m_startPos, m_endPos);
        }
    } else if (m_freqanaType == FreqAnaType::NoSelection) {
        newRect = QRect(QPoint(0, 0), QPoint(0, 0));
    }
    return newRect;
}

void Sdp2dMainGatherDisplayPlot::frequencyAnalysisLeftMousePress(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void Sdp2dMainGatherDisplayPlot::noFunctionLeftMouseDoubleClicked(QMouseEvent *event)
{
    double x = event->pos().x();
    double y = event->pos().y();
    QPointF tt = m_parent->convertPosToAxisValues(QPointF(x, y));
    int trIdx = int(tt.x());   // trIdx = trSeq +1;
    int ntr = m_parent->getNumTracesOfCurrentGather();
    if(trIdx < 1) trIdx = 1;
    if(trIdx > ntr) trIdx = ntr;
    m_parent->cleanHighLightedtrace(trIdx);
    m_startPos = QPoint(0, 0);
    m_endPos = QPoint(0, 0);
    m_selectedRect = QRect(m_startPos, m_endPos);
    mSelectionRect->setRect(m_selectedRect);
    if (mSelectionRect && mSelectionRect->isActive()) {
        mSelectionRect->cancel();
        replot(rpQueuedReplot);
    }
    replot();
}

void Sdp2dMainGatherDisplayPlot::frequencyAnalysisLeftMouseRelease(QMouseEvent *event)
{
    if(m_freqanaType == FreqAnaType::RectArea){
        if(mMouseHasMoved ) {
            m_parent->removeHighLightedTrace();
            m_endPos = event->pos();
            m_parent->cleanTempGraphs();            

            QRect newRect = setSelectionRectForFrequencyAna(event);
            if(newRect.height() < 15){
                if (mSelectionRect && mSelectionRect->isActive()) {
                    mSelectionRect->cancel();
                    replot(rpQueuedReplot);
                }
                m_startPos = QPoint(0, 0);
                m_endPos = QPoint(0, 0);
            } else {
                m_selectedRect = newRect;
                float dt = m_parent->getTimeSampRateOfCurrentGather();
                QString label = QString("Gather %1, Trace %2~%3, Time %4~%5").arg(m_parent->getGatherIndex()).arg(m_selectedRect.left()).arg(m_selectedRect.right()).arg((m_selectedRect.top())*dt).arg((m_selectedRect.bottom())*dt);
                m_parent->frequencyAnalysis(m_selectedRect, label);
            }
        }else{
            m_startPos = m_bkPos;
        }
        replotSelection();
    } else if(m_freqanaType == FreqAnaType::SingleTrace ||  m_freqanaType == FreqAnaType::WholeGather){
        m_selectedRect = setSelectionRectForFrequencyAna(event);
        if (mSelectionRect && mSelectionRect->isActive()) {
            mSelectionRect->cancel();
            replot(rpQueuedReplot);
        }
    }
}

void Sdp2dMainGatherDisplayPlot::badTraceSelectionLeftMouseDoubleClicked(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void Sdp2dMainGatherDisplayPlot::badTraceSelectionLeftMousePress(QMouseEvent *event)
{    
    QPointF tt = m_parent->convertPosToAxisValues(QPointF(event->pos().x(), event->pos().y()));
    int trSeq = int(tt.x()) - 1;    // trIdx = trSeq +1;
    int ntr = m_parent->getNumTracesOfCurrentGather();
    if(trSeq < 0) trSeq = 0;
    if(trSeq > ntr-1) trSeq = ntr -1;

    m_parent->setSingleBadTrace(trSeq);
    m_minSelectedTraceSeq = trSeq;
    m_maxSelectedTraceSeq = trSeq;
    m_startTraceSeq = trSeq;
}

void Sdp2dMainGatherDisplayPlot::badTraceSelectionLeftMouseMove(QMouseEvent *event)
{
    if (mSelectionRect && mSelectionRect->isActive()) {
        mSelectionRect->cancel();
        replot();
    }

    QPointF tt = m_parent->convertPosToAxisValues(QPointF(event->pos().x(), event->pos().y()));
    int trSeq = int(tt.x()) - 1;    // trIdx = trSeq +1;
    int ntr = m_parent->getNumTracesOfCurrentGather();
    if(trSeq < 0) trSeq = 0;
    if(trSeq > ntr-1) trSeq = ntr -1;
    if(trSeq < m_minSelectedTraceSeq) m_minSelectedTraceSeq = trSeq;
    if(trSeq > m_maxSelectedTraceSeq) m_maxSelectedTraceSeq = trSeq;

    //cout << "m_startTraceSeq=" << m_startTraceSeq << " trSeq=" << trSeq << endl;

    int tr1 = m_startTraceSeq;
    int tr2 = trSeq;

    if(m_startTraceSeq > trSeq){
        tr1 = trSeq;
        tr2 = m_startTraceSeq;
    }

    m_parent->setMultipelBadTraces(m_minSelectedTraceSeq, tr1, tr2, m_maxSelectedTraceSeq);
}

void Sdp2dMainGatherDisplayPlot::badTraceSelectionLeftMouseRelease(QMouseEvent *event)
{
    if (mSelectionRect && mSelectionRect->isActive()) {
        mSelectionRect->cancel();
        replot(rpQueuedReplot);
    }
    if(m_minSelectedTraceSeq < m_maxSelectedTraceSeq){
        double x = event->pos().x();
        double y = event->pos().y();
        QPointF tt = m_parent->convertPosToAxisValues(QPointF(x, y));
        int trSeq = int(tt.x()) - 1;

        int ntr = m_parent->getNumTracesOfCurrentGather();
        if(trSeq < 0) trSeq = 0;
        if(trSeq > ntr-1) trSeq = ntr -1;

        int tr1 = m_startTraceSeq;
        int tr2 = trSeq;

        if(m_startTraceSeq > trSeq){
            tr1 = trSeq;
            tr2 = m_startTraceSeq;
        }
        m_parent->setMultipelBadTraces(m_minSelectedTraceSeq, tr1, tr2, m_maxSelectedTraceSeq);
        //cout << "setMultipelBadTraces on mouse release" << endl;
    }
    m_minSelectedTraceSeq = m_parent->getNumTracesOfCurrentGather();
    m_maxSelectedTraceSeq = 1;
    m_startTraceSeq = -1;
}

void Sdp2dMainGatherDisplayPlot::pickingMuteLeftMouseDoubleClicked(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void Sdp2dMainGatherDisplayPlot::pickingMuteLeftMousePress(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void Sdp2dMainGatherDisplayPlot::pickingMuteLeftMouseMove(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void Sdp2dMainGatherDisplayPlot::pickingMuteLeftMouseRelease(QMouseEvent *event)
{
    if(event->pos() == m_bkPos) return;
    if(!mMouseHasMoved ) {
        m_parent->addOneMutePickPoint(event->pos());
        replot();
        m_bkPos = event->pos();
    }
}

void Sdp2dMainGatherDisplayPlot::pickingMuteMiddleMouseRelease(QMouseEvent *event)
{
    if(!mMouseHasMoved ) {
        m_parent->removeOneMutePickPoint(event->pos());
    }
}

void Sdp2dMainGatherDisplayPlot::StackVelAnalysisLeftMouseDoubleClicked(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void Sdp2dMainGatherDisplayPlot::measureLinearVelLeftMousePress(QMouseEvent *event)
{
    m_parent->drawLinearVelMeasureFirstPoint(event->pos());    
}

void Sdp2dMainGatherDisplayPlot::measureLinearVelLeftMouseRelease(QMouseEvent *event)
{
    Q_UNUSED(event);
}

void Sdp2dMainGatherDisplayPlot::measureLinearVelLeftMouseMove(QMouseEvent *event)
{
    m_parent->drawLinearVelMeasureSecondPoint(event->pos());
    if (mSelectionRect && mSelectionRect->isActive()) {
        mSelectionRect->cancel();
        replot(rpQueuedReplot);
    }
}

void Sdp2dMainGatherDisplayPlot::cleanDisplay(void)
{
    //cout << " in Sdp2dMainGatherDisplayPlot::cleanDisplay" << endl;
    if (mSelectionRect && mSelectionRect->isActive()) {
        mSelectionRect->cancel();        
    }

    m_startPos = QPoint(0,0);
    m_endPos = QPoint(0,0);
    m_selectedRect = QRect(m_startPos, m_endPos);
    m_mouseLeftPressed = false;

    replot(rpQueuedReplot);
}

void Sdp2dMainGatherDisplayPlot::replotSelection()
{
    if (mSelectionRect && mSelectionRect->isActive() && m_freqanaType == FreqAnaType::RectArea) {
        if(m_freqanaType == FreqAnaType::RectArea){
            QRect newRect = m_selectedRect;
            double key1 = newRect.left();
            double key2 = newRect.right();
            double val1 = newRect.top()*m_parent->getTimeSampRateOfCurrentGather();
            double val2 = newRect.bottom()*m_parent->getTimeSampRateOfCurrentGather();
            //cout << "key1="<< key1 << " val1=" << val1 << " key2="<< key2 << " val2=" << val2 << endl;

            double x1, x2, y1, y2;
            m_parent->convertAxisValuesToPos(key1, val1, x1, y1);
            m_parent->convertAxisValuesToPos(key2, val2, x2, y2);

            QPoint tl = QPoint(x1, y1);
            QPoint br = QPoint(x2, y2);
            //cout << "left="<< x1 << " right=" << x2 << " top="<< y1 << " Btm=" << y2 << endl;
            newRect = QRect(tl, br).normalized();
            if(newRect.width()<2) return;
            mSelectionRect->setRect(newRect);
        } else {
            mSelectionRect->cancel();
        }
    }
    //mSelectionRect = m_selectedRect;
    replot(rpQueuedReplot);
}

void Sdp2dMainGatherDisplayPlot::cleanSelection(void)
{
    if (mSelectionRect && mSelectionRect->isActive()) {
        mSelectionRect->cancel();
    }

    m_mouseLeftPressed = false;

    m_startPos = QPoint(0, 0);
    m_endPos = QPoint(0, 0);
    m_selectedRect = QRect(m_startPos, m_endPos);

    m_parent->cleanTempGraphs();
    replot(rpQueuedReplot);
}

void Sdp2dMainGatherDisplayPlot::displayedGatherChanged()
{
    cleanSelection();
    m_hideGather->setChecked(false);
    m_hideGather->setDisabled(true);
    m_saveTraces->setDisabled(true);
    m_loadTraces->setDisabled(true);

}

void Sdp2dMainGatherDisplayPlot::setSelectedRectForOnetrace(int trIdx)
{
    int staY = 0;
    int endY = m_parent->getTimeSamplesOfCurrentView() - 1;

    QPoint start(trIdx, staY);
    QPoint end(trIdx, endY);
    m_selectedRect = QRect(start, end);
}

void Sdp2dMainGatherDisplayPlot::setSelectedRectForWholeGather(QRect rect)
{
    QString label = QString("Gather %1").arg(m_parent->getGatherIndex());
    QPoint start(1, 0);
    QPoint end(m_parent->getNumTracesOfCurrentGather(), m_parent->getTimeSamplesOfCurrentView() - 1);
    rect = QRect(start, end);
    m_parent->frequencyAnalysis(rect, label);
}
