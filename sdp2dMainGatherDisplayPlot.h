#ifndef SDP2DMAINGATHERDISPLAYPLOT_H
#define SDP2DMAINGATHERDISPLAYPLOT_H

#include "qcustomplot.h"

class Sdp2dMainGatherDisplayArea;

class Sdp2dMainGatherDisplayPlot : public QCustomPlot
{
public:
    Sdp2dMainGatherDisplayPlot(QWidget *parent = 0);
    ~Sdp2dMainGatherDisplayPlot();

    void cleanDisplay(void);
    QPoint getSelectionStartPoint(void) const { return m_startPos; }
    QPoint getSelectionEndPoint(void) const { return m_endPos; }
    QRect getSelectedRect(void) const { return m_selectedRect; }
    void setSelectedRectForOnetrace(int tIdx);
    void setSelectedRectForWholeGather(QRect rect = QRect());
    void replotSelection(void);
    void cleanSelection(void);
    void disableHideGather(bool disable);    
    void enableSaveActionForFilter(bool enable);

    Q_SLOT void displayedGatherChanged();

private:
    void showEvent(QShowEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    Q_SLOT void on_display_customContextMenuRequested(const QPoint &pos);
    Q_SLOT void onMouseDoubleClicked(QMouseEvent *event);

    void singleTraceFreqAna(bool checked);
    void rectAreaFreqAna(bool checked);
    void wholeGatherFreqAna(bool checked);
    void hideGatherFreqAna(bool checked);

    void showTraceInShotGather(void);
    void showTraceInCDPsGather(void);
    void showTraceInRecvGather(void);
    void showTraceInOffsetGather(void);

    void createPopupMenus(void);

    void frequencyAnalysisLeftMouseDoubleClicked(QMouseEvent *event);
    void badTraceSelectionLeftMouseDoubleClicked(QMouseEvent *event);
    void StackVelAnalysisLeftMouseDoubleClicked(QMouseEvent *event);
    void pickingMuteLeftMouseDoubleClicked(QMouseEvent *event);
    void noFunctionLeftMouseDoubleClicked(QMouseEvent *event);

    void frequencyAnalysisLeftMousePress(QMouseEvent *event);
    void badTraceSelectionLeftMousePress(QMouseEvent *event);
    void pickingMuteLeftMousePress(QMouseEvent *event);
    void measureLinearVelLeftMousePress(QMouseEvent *event);

    void frequencyAnalysisLeftMouseRelease(QMouseEvent *event);
    void badTraceSelectionLeftMouseRelease(QMouseEvent *event);
    void pickingMuteLeftMouseRelease(QMouseEvent *event);
    void measureLinearVelLeftMouseRelease(QMouseEvent *event);

    void badTraceSelectionLeftMouseMove(QMouseEvent *event);
    void pickingMuteLeftMouseMove(QMouseEvent *event);
    void measureLinearVelLeftMouseMove(QMouseEvent *event);

    void pickingMuteMiddleMouseRelease(QMouseEvent *event);

    QRect setSelectionRectForFrequencyAna(QMouseEvent *event);


private:
    Sdp2dMainGatherDisplayArea* m_parent;

    QMenu* m_freqAnaMenu;
    QMenu* m_badTraceMenu;
    QMenu* m_mutePickMenu;
    QMenu* m_noFuncMenu;
    QAction* m_stFreqAna;
    QAction* m_raFreqAna;
    QAction* m_wgFreqAna;
    QAction* m_hideGather;
    QAction* m_saveTraces;
    QAction* m_loadTraces;
    QAction* m_measureLVel;

    QAction* m_topMutePick;
    QAction* m_btmMutePick;
    QAction* m_applyTopMute;
    QAction* m_applyBtmMute;
    QAction* m_hideBadTraces;

    //double m_xscale;

    QPoint m_startPos;
    QPoint m_endPos;
    QPoint m_bkPos;

    QRect m_selectedRect;

    int m_freqanaType;
    bool m_mouseLeftPressed;

    int m_minSelectedTraceSeq;
    int m_maxSelectedTraceSeq;
    int m_startTraceSeq;

};

#endif // SDP2DMAINGATHERDISPLAYPLOT_H
