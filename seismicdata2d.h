#ifndef SEISMICDATA2D_H
#define SEISMICDATA2D_H

#include <sys/stat.h>

#include "sdp2dSegy.h"
#include <QObject>
#include <QWidget>
#include <QPointF>
#include <QList>

QT_FORWARD_DECLARE_CLASS(QStringList)

class SeismicDataProcessing2D;
class Sdp2dMapDiaplayDockWidget;
class Sdp2dGatherDisplayArea;
class Sdp2dMainGatherDisplayArea;
class Sdp2dProcessedGatherDisplayArea;
class Sdp2dDisplayParamTab;
class Sdp2dFrequencyAnalysisDock;
class Sdp2dPreStackMutePicks;
class Sdp2dQDomDocument;


using namespace std;

struct value_idx{
    int idx;
    float val;

    value_idx(int i, float v)
    {
        idx = i;
        val = v;
    }
    value_idx()
    {
        idx = 0;
        val = 0;
    }

    bool operator==(const value_idx& a) const
    {
        return (this->idx == a.idx && qAbs(this->val - a.val) < 0.001);
    }
};


typedef struct One2DGather{
    int ntraces;
    float groupValue1;   // average X value for CSG, CRG, CDP. average offset for COG.
    float groupValue2;   // average Y value for CSG, CRG, CDP. ZERO for COG.
    float groupValue3;   // average elevation for CSG, CRG, CDP.ZERO for COG.
    float groupValue4;   // average source depth for CSG. ZERO for CRG, CDP and COG.
    std::vector<int> traceIdx;

    One2DGather()
    {
        ntraces = 0;
        groupValue1 = 0.;
        groupValue2 = 0.;
        groupValue3 = 0.;
        groupValue4 = 0.;
    }

    ~One2DGather()
    {
        traceIdx.clear();
    }

}gather_info;

typedef struct TwoDGathersInfo {
    int gather_type; // 1. common shot; 2. common cdp; 3. common offset; 4. common receiver; 5. stack
    int ngroups;
    int idxType;      // see Note 1
    int minGroupValue;
    int maxGroupValue;
    float minGroupSpace;
    float maxGroupSpace;
    float aveGroupSpace;
    float useGroupSpace;
    QPointF firstGroupXY; // record negative min/max offset for common offset gather
    QPointF lastGroupXY;  // record positive min/max offset for common offset gather
    std::vector<gather_info> group;

    TwoDGathersInfo(int type)
    {
         gather_type = type;
         ngroups = 0;
    }

    ~TwoDGathersInfo()
    {
        group.clear();
    }
} gathers;

typedef struct IndexValue{
    int trIdx;
    float time;

    IndexValue(int i, float v)
    {
        trIdx = i;
        time = v;
    }
    IndexValue()
    {
        trIdx = 0;
        time = 0;
    }

} IdxVal;

typedef struct Index2Value{
    int trIdx;
    int intVal;
    float time;

    Index2Value(int i, int c, float v)
    {
        trIdx = i;
        intVal = c;
        time = v;
    }
    Index2Value()
    {
        trIdx = 0;
        intVal = 0;
        time = 0;
    }

} Idx2Val;

enum SeismicTraceFlag{
     DeadTrace=0, LiveTrace=1, UnusedTrace=2, PickedBadTrace=3
};

enum FreqAnaType{
     NoSelection=0, SingleTrace=1, RectArea=2, WholeGather=3
};

enum SeismicDisplayTpye{
     Color=1, Wiggle=2, Both=3
};

enum XAxisType{
     TraceIdx=1, OffsetVal=2, CDPVal=3
};

enum SeismicDataType{
    UnKnow=0, Stack=1, PreStack=2, Attribure=3
};

enum DisplayGatherType{
    CommonShot=1, CommonDepthPoint=2, CommonOffset=3, CommonReceiver=4, PostStack=5, Attribute=6
};

enum MuteType{
    TopMute=1, BottomMute=2
};

class SeismicData2D: public QObject
{
    Q_OBJECT

public:
    SeismicData2D();
    SeismicData2D(const std::string& segy_file_name, SeismicDataProcessing2D* parent, bool loadSEGYHeader=true);
    virtual ~SeismicData2D();

    int getDataType(void) const { return m_datatype; }

    std::string& getSEGYFileName();
    Sdp2dSegy* getSegyhandle(void) const {return m_sgy; };
    int getNumberOfTraces(void) const { return m_ntr; };
    int getSamplesPerTraces(void) const { return m_ns; };
    int getTimeSampleRateInUs(void) const { return m_dtus; }

    int* getCDPsIdPointer(void) { return m_cdpid; }    
    short* getTraceFlagPointer(void) { return m_trflag; }
    short* getTopMutePointer(void) { return m_topMute; }
    short* getBtmMutePointer(void) { return m_btmMute; }

    int getMinCDPIndex() { return m_pcdp->minGroupValue; }
    int getMaxCDPIndex() { return m_pcdp->maxGroupValue; }

    void getAveageAmplitude(double* amp);
    void getAbsoluteAmplitude(double* amp);
    void getRMSAmplitude(double* amp);

    QPointF& getFirstPoint(void)  { return m_firstXY; }
    QPointF& getLastPoint(void)  { return m_lastXY; }

    float getAngleOf2DLine(void) const { return m_angle; }

    void setCDPSpacing(float value);
    void setOffsetSpacing(float value);
    void setMinOffset(float);
    void setMaxOffset(float value);

    float getCDPSpacing(void) const { return m_dcdp; }
    float getOffsetSpacing(void) const { return m_doff; }
    float getMinOffset(void) const { return m_minoff; }
    float getMaxOffset(void) const { return m_maxoff; }

    void getAllCDPXY(QList<QPointF>&);

    bool isIndexFileExist();
    void checkIndexStructure(gathers* p);

    gathers* getCDPGatherIndex() const { return m_pcdp; }   

    bool isOutputDiscardBadTraces(void) const { return m_discardBadTraces; }
    bool isOutputApplyTopMute(void) const { return m_outputApplyTopMute; }
    bool isOutputApplyBtmMute(void) const { return m_outputApplyBtmMute; }

    int getOutputOrder(void) const { return m_outputOrder; }
    QString getOutputSegyFileName(void) const { return m_outSegyName; }

    void setOutputDiscardBadTraces(bool val) { m_discardBadTraces = val; }
    void setOutputApplyTopMute(bool val) { m_outputApplyTopMute = val; }
    void setOutputApplyBtmMute(bool val) { m_outputApplyBtmMute = val; }
    void setOutputSegyFileName(QString name) { m_outSegyName = name; }
    void setOutputOrder(int val) { m_outputOrder=val; }

    void hideAllDisplayWidgets();
    virtual void showAllDisplayWidgets();
    void addDisplayWidgets(QWidget* widget);
    virtual void removeAdjunctiveDisplays(void);

    virtual QStringList& getDataSummary(void);
    virtual void createDataSummaryInfo(bool moreflag=false);

    void createFrequencyAnaDock(bool visible = false);

    void setFreqAnalysisPointer(Sdp2dFrequencyAnalysisDock* p){ m_fanaDock = p; }
    void setMapDisplayPointer(Sdp2dMapDiaplayDockWidget* p) { m_mapDock = p; }    
    void setInputGatherDisplayPointer(Sdp2dMainGatherDisplayArea* p);
    void setProcessedGatherDisplayPointer(Sdp2dProcessedGatherDisplayArea* p) { m_outGthDisplay = p;}
    void setDisplayParamTabPointer(Sdp2dDisplayParamTab* p);

    Sdp2dProcessedGatherDisplayArea* getProcessedGatherDisplayPointer(void) const { return m_outGthDisplay;}
    Sdp2dMainGatherDisplayArea* getfocusedDisplayPointer(void) const { return m_focusedDisplay; }
    Sdp2dMainGatherDisplayArea* getInputGatherDisplayPointer(void) const { return m_inGthDisplay; }
    Sdp2dMapDiaplayDockWidget* getMapDisplayPointer(void) const { return m_mapDock; }
    Sdp2dFrequencyAnalysisDock* getFreqAnalysisPointer(void) const { return m_fanaDock; }
    Sdp2dDisplayParamTab* getDisplayParamTabPointer(void) const { return m_displayParaTab; }

    bool getTraceHeaderDisplay(void);
    QString getPlotTitle();
    QString getPlotXLabel();
    QString getPlotYLabel();
    int getDataClipPercentage();
    int getGatherType();
    int getDisplayType();
    int getColorMapIndex();
    float getMaxDisplayTime();
    int getSymmetryRange(void);
    int getGroupStep(void);
    int getPlotGroupIndex(void);
    float getPlotXScale(void);
    float getPlotYScale(void);
    bool getPlotFitZoom(void);
    float getPlotWiggleScale(void);
    int getXAxisType(void);
    int getReversepolarity(void);

    void updatePlotTitleForDisplayParaTab(QString text);
    void updateXLableForDisplayParaTab(QString text);
    void updateYLableForDisplayParaTab(QString text);

    void setTraceHeaderDisplay(bool checked);
    void setPlotTitle(QString text);
    void setPlotXLabel(QString text);
    void setPlotYLabel(QString text);
    void setDataClipPercentage(int val);
    void setGatherType(int val, int minNtraces=1);
    void setDisplayType(int val);
    void setColorMapIndex(int val);
    void setMaxDisplayTime(float val);
    void setSymmetryRange(int val);
    void setGroupStep(int val);
    void setPlotXScale(float val);
    void setPlotYScale(float val);
    void setPlotFitZoom(bool zoomfit);
    void setPlotWiggleScale(float val);
    void setXAxisType(int xaxisType);    
    void setReversepolarity(int val);


    void setDisplayScales(float xscale, float yscale, QPointF plotEdge);
    QPointF getPlotEdge(void) const { return m_plotEdge; }

    QRect getSelectedRect(void) const {return m_selectedRect; }

    bool setPlotGroupIndex(int index);
    int setPlotGroupToTheFirst(int minNtraces = 1);
    int setPlotGroupToTheLast(int minNtraces = 1);
    void updateDisplayParameters(void);

    void setRegenerateIndexFile(bool regenerate=true);

    void cleanMainDisplay(bool funChanged=false);

    void setGatherDisplayWithHelightTrace(int gatherType, int gatherIdx, int traceIdx);
    void updateMapAndFreqAnaWithGatherTrace(int traceIdx);

    void frequencyAnalysis(int nx, int nt, float** data, QString& label);
    void frequencyAnalysis(QRect selectRect, float** data, QString& label);

    int  getInteractiveFunction(void) const { return m_interactiveFunction; }
    void setInteractiveFunction(int val);

    void bandPassFilterSelectedDataAndDisplay(Sdp2dQDomDocument* m_domval, gathers* gp);
    int  getFrequencyAnaType(void) const { return m_freqanaType; }
    void setFrequencyAnaType(int val) { m_freqanaType = val; }

    void calculateTraceWeightValues(int top, int btm, float* weight, int text=10, int bext=10);
    void setFreqComparisonFlag(bool flag);
    bool isFrequencyAnaDockHide();

    bool isJobdockVisible(void) const { return m_jobdockVisible; }
    void setJobdockVisible(bool visible) {  m_jobdockVisible = visible; }

    QString getProcessingFunction(void) const { return m_processFunction; }
    void setProcessingFunction(QString func) { m_processFunction = func; }
    bool getProcessWholeData(void) const { return m_processWholeData; }
    void setProcessWholeData(bool val) { m_processWholeData = val; }

    void updateMuteParameters(Sdp2dQDomDocument* m_domval);
    void setTopMute();
    void setBtmMute();

    Sdp2dPreStackMutePicks* getCurrentMutePointer(void) const { return m_currentMute; }
    int getMuteType(void) const { return m_muteType; }
    int getMuteTaperLength(void);


    void updateTraceHeaderWithEditedInfo(void);

    virtual void outputSegyWithLocalFormat(void);

    void updateDisplays(int traceIdx=0);

protected:
    Sdp2dSegy *m_sgy;

    std::string m_idxfile;
    qint64 m_idxPos;

    SeismicDataProcessing2D* m_mainWindow;
    QStatusBar* m_sbar;

    int m_interactiveFunction;
    int m_freqanaType;
    QRect m_selectedRect;

    int m_datatype;

    int m_ntr;
    int m_ns;
    int m_dtus;

    QPointF m_firstXY;
    QPointF m_lastXY;

    float m_slope;
    float m_intercept;
    float m_angle;

    gathers* m_pcdp;  // common CDPs
    int* m_cdpid;
    short* m_trflag;

    float* m_cdpx;
    float* m_cdpy;

    float m_dcdp;
    float m_doff;
    float m_minoff;
    float m_maxoff;

    QStringList m_datasum;

    bool m_jobdockVisible;
    bool m_processWholeData;
    QString m_processFunction;

    QString m_outSegyName;
    bool m_discardBadTraces;
    bool m_outputApplyTopMute;
    bool m_outputApplyBtmMute;
    int m_outputOrder;

    QPointF m_plotEdge;    

    QList<QWidget*> widgetList;
    Sdp2dDisplayParamTab* m_displayParaTab;
    Sdp2dMapDiaplayDockWidget* m_mapDock;
    Sdp2dFrequencyAnalysisDock* m_fanaDock;

    Sdp2dMainGatherDisplayArea* m_inGthDisplay;
    Sdp2dProcessedGatherDisplayArea* m_outGthDisplay;

    //Sdp2dGatherDisplayArea* m_focusedDisplay;

    Sdp2dMainGatherDisplayArea* m_focusedDisplay;

    Sdp2dPreStackMutePicks* m_tMute;
    Sdp2dPreStackMutePicks* m_bMute;
    Sdp2dPreStackMutePicks* m_currentMute;
    int m_muteType;
    short* m_topMute;  // unit: samples
    short* m_btmMute;  // unit: samples

    void loadSegyTracesHeader(void);
    void calculateSlopeOf2DLine(void);

    void estimateSpacing(float *x, float *y, gathers *p, float uplimit);
    void createTheIndexFileName(const std::string& segy_fname);
    void sortSubsetOfGathers(gathers *p, float* sortKey);
    void buildIndexUsingOneCoordinate(float* x, int* id, gathers* gth);

    void collectTraceHeaderInfo(void);
    virtual void collectGathersInfo(void);
    virtual void fillIndexStructure(void);
    //virtual void findLineEndPoints(void);
    //virtual void estimate2DAcquisitionGeometry(void);


    void writeOneGroup2Disk(ofstream& outfile, gathers *p, int ig);
    void getSeismicDataInRect(QRect selectedRect, float** data, gathers* gp);
    void getProcessedSeismicDataInRect(QRect selectedRect, float** data);


};

#endif // SEISMICDATA2D_H
