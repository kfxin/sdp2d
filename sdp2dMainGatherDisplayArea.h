#ifndef SDP2DMAINGATHERDISPLAYAREA_H
#define SDP2DMAINGATHERDISPLAYAREA_H

#include <QScrollArea>

QT_BEGIN_NAMESPACE
class QString;
class QResizeEvent;
class QStatusBar;
QT_END_NAMESPACE

class SeismicData2D;
class SeismicData2DPreStack;
class SeismicDataProcessing2D;
class Sdp2dQCPColorMap;
class Sdp2dMainGatherDisplayPlot;
class QCPColorScale;
class QCPColorGradient;
class QCPRange;
class QCPCurve;
class QCPCurveData;
class QCPMarginGroup;
class QCPColorMap;
class QCPGraph;
class QCPAxisRect;
class QCPLayoutGrid;
class QCPItemText;

#include "sdp2dGatherDisplayArea.h"
#include "seismicdata2d.h"

class Sdp2dMainGatherDisplayArea : public Sdp2dGatherDisplayArea
{
    Q_OBJECT

public:
    explicit Sdp2dMainGatherDisplayArea(SeismicData2D* m_sd2d, QWidget *parent = nullptr);
    ~Sdp2dMainGatherDisplayArea();

    void displayOneGather(void);

    void setDataClipPercentage(int) override;
    int  setGatherType(int gType, int minNtraces = 1) override;
    void setDisplayType(int) override;
    void setPlotWiggleScale(float val) override;
    void setPlotXAxisType(int val) override;
    void setReversepolarity(int val) override;

    bool setPlotGroupIndex(int index);

    int setPlotGroupToTheFirst(int minNtraces = 1);
    int setPlotGroupToTheLast(int minNtraces = 1);

    void setGatherDisplayWithHelightTrace(int gatherType, int gatherIdx, int traceIdx);
    void setGatherDisplayWithHelightTrace(int gatherType, int traceSeq);

    int getInteractiveFunction();    

    bool mouseMoveOnTheGather(double key, double value);
    bool mouseMoveOnTheHeaderDisplay(QPoint pos);

    void cleanDisplay(bool funChanged=false);
    void removeHighLightedTrace(void);
    bool hasHighLightTrace(void) const { return m_displayedtraceIdx; }    

    float *getDataOfASeismicTrace(int traceIdx);

    float** getDataOfOutputSeismicGather(void);

    void drawAndMapSingleWiggleTrace(int traceIdx);
    void setTraceVisible(bool visible=false);

    void disableHideGather(bool disable);
    void cleanHighLightedtrace(int trIdx);

    void setSingleBadTrace(int tIdx);
    void setMultipelBadTraces(int min, int start, int end, int max);

    int  getFrequencyAnaType(void);
    void setFrequencyAnaType(int val);
    void changeDisplayOfWiggleTraces(void);
    void setBadTracesWithinOneGather(bool checked=true);

    bool isSelectedRectChanged(QRect selectedRect);
    QRect getSelectedRect();

    void displayProcessedSeismicTraces(QRect selectedRect, float** data);
    void displayProcessedSeismicTraces(void);

    void cleanTempGraphs(void);
    void showTempGraphs(bool show);
    bool hasTempGraphs(void);
    bool isTempGraphsVislible(void);

    void replaceWithFilteredData(void);
    void restoreRawData(void);

    void frequencyAnalysis(QRect selectRect, QString& label);

    void setMutePicksDisplay(void);
    void addOneMutePickPoint(QPoint pos);
    void removeOneMutePickPoint(QPoint pos);

    void displayMutePicks(QList<IdxVal>& left, QList<IdxVal>& middle, QList<IdxVal>& right);
    void setMutePicksVisible(bool visible=false);

    void setTopMute(bool checked);
    void setBtmMute(bool checked);
    void applyTopMuteOnData(bool checked);
    void applyBtmMuteOnData(bool checked);
    void hideBadTraces(bool checked);

    void checkMeasureLinearVelocity(bool checked);
    void drawLinearVelMeasureFirstPoint(QPoint pos);
    void drawLinearVelMeasureSecondPoint(QPoint pos);

    void getDataOfInputSeismicGather(float** outData);
    void updataGatherDisplay(void);

signals:
    void gatherChanged();

private:

    bool loadOneGroupData(void);

    void loadOneGroupOffsetElevation();    

    void drawTheHighLightedTrace(QColor col);
    void drawOneWiggleTrace(QCPGraph* wavtr, QCPGraph* reftr, int trSeq, QColor color);
    void mapSingleTraceOnAmpDisplay(int trIdx);
    void setDisplayedBadTraces(void);
    void updateWiggleDisplayData(void);

    void updateWiggleTraces(void);

    void updateStatusBarMessage(void);

    void replotSelection(void) override;

    void setDataForColorDisplay(void) override;

    Q_SLOT void displayedDataChanged(const QCPRange& range);

    void displayProcessedSeismicTraces(float **data, QRect selectedRect);

    void createLinearVelocityMeasureElements(void);
    void cleanLinearVelocityLine(void);

private:    

    float** m_gatherOut;

    QCPGraph* m_wavefrom;
    QCPGraph* m_traceref;

    QCPGraph* m_midMuteGraph;
    QCPGraph* m_lftMuteGraph;
    QCPGraph* m_rhtMuteGraph;

    QCPGraph* m_linearVelLine;
    QCPItemText* m_linearVelLable;

    bool* m_badTraces;
    QVector<QCPGraph*> m_badTrcGraphs;
    QVector<QCPGraph*> m_badRefGraphs;

    QVector<QCPGraph*> m_tmpTracesGraphs;

    QColor m_colorBadTrace;
    QColor m_colorSelTrace;    
    QColor m_colorFreTrace;
    QColor m_colorTmpCurve;

};

#endif // SDP2DMAINGATHERDISPLAYAREA_H
