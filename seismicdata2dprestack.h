#ifndef SEISMICDATA2DPRESTACK_H
#define SEISMICDATA2DPRESTACK_H

#include <cmath>
#include <algorithm>
#include <cfloat>
#include <climits>
#include <fstream>

#include "seismicdata2d.h"

#include <QWidget>
#include <QMultiMap>

QT_FORWARD_DECLARE_CLASS(QStringList)
QT_FORWARD_DECLARE_CLASS(QDockWidget)


class Sdp2dAmplitudeDisplayDock;
class Sdp2dQDomDocument;
class Sdp2dVelSemblanceDisplayArea;

class SeismicData2DPreStack : public SeismicData2D
{
public:
    SeismicData2DPreStack(const string& segy_file_name, SeismicDataProcessing2D* mainwin, bool loadSEGYHeader=true);
    ~SeismicData2DPreStack();

    int getFoldOfOneCDP(int cdp);
    int getFoldOfCDPs(QList<QPointF>& cdps);

    int getNumberOfCDPs(void);
    int getNumberOfShots(void);
    int getNumberOfReceivers(void);
    int getNumberOfOffsets(void);

    int getNumberOfTracesOfGather(int gType, int gIdx);

    void getTraceFlags(int gatherType, int gatherIdx, bool* btFlags);
    void setTraceFlags(int gatherType, int gatherIdx, bool* btFlags);
    short* getTraceFlags(void) { return m_trflag; }

    int getMinShotIndex() { return m_pcs->minGroupValue; }
    int getMaxShotIndex() { return m_pcs->maxGroupValue; }
    int getMinRecvIndex() { return m_pcr->minGroupValue; }
    int getMaxRecvIndex() { return m_pcr->maxGroupValue; }

    int getTracesIndexInData(int gType, int gIdx, int tIdx);

    void getAllShotXY(QList<QPointF>&);
    void getAveCDPXY(QList<QPointF>&);
    void getAllReceiverXY(QList<QPointF>&);

    int* getShotIdPointer(void) { return m_srcid; }
    int* getRecvIdPointer(void) { return m_recid; }
    int* getOffsIdPointer(void) { return m_offid; }
    float* getOffsetValuePointer(void) { return m_offset; }

    //int getReceiversOfShotXY(const QPointF &groupXY, QList<QPointF> &subsetXY);
    int getSubsetXYUsingGroupXY(const QPointF& groupXY, QList<QPointF>& subsetXY, int gatherType=1);
    QPointF getSubsetXYUsingGroupIdx(int groupIdx, QList<QPointF>& subsetXY, int gatherType);

    float** getSeismicDataOfGather(int gatherType, int gIdx, int& ntr);

    void getOffsetAndElevOfGather(int gatherType, int gIdx, int ntr, double* offset, double* elev);
    void getOffsetValuesOfGather(int gatherType, int gIdx, int ntr, float* offset);

    void getOneTraceWithTypeAndIndex(int gType, int gIdx, int tIdx, float *trace);

    gathers* getShotGatherIndex(void);
    gathers* getReceiverGatherIndex(void);
    gathers* getOffsetGatherIndex(void);
    gathers* getGatherIndexPointer(int gatherType);
    gathers* getGatherIndexPointer(void);

    void setupDataIndex();

    void processAndOutputSegyWithLocalFormat(QString moduleName);
    void outputSegyWithLocalFormat(void) override;
    void outputSegyWithLocalFormatInShotOrder(const string& segy_fname);
    void outputSegyWithLocalFormatInCDPOrder(const string& segy_fname);
    void outputSegyWithLocalFormatInOffsetOrder(const string& segy_fname);
    void outputSegyWithLocalFormatInRecvOrder(const string& segy_fname);

    float getSrcXOfATrace(int tidx) { return m_srcx[tidx]; }
    float getSrcYOfATrace(int tidx) { return m_srcy[tidx]; }
    float getRecXOfATrace(int tidx) { return m_recx[tidx]; }
    float getRecYOfATrace(int tidx) { return m_recy[tidx]; }
    float getOffsetOfATrace(int tidx) { return m_offset[tidx]; }
    float getSDepthOfATrace(int tidx) { return m_sdepth[tidx]; }
    float getSElevOfATrace(int tidx) { return m_selev[tidx]; }
    float getRelevOfATrace(int tidx) { return m_relev[tidx]; }

    int getRIdxOfATrace(int tidx) { return m_recid[tidx]; }
    int getSIdxOfATrace(int tidx) { return m_srcid[tidx]; }
    int getCIdxOfATrace(int tidx) { return m_cdpid[tidx]; }
    int getOIdxOfATrace(int tidx) { return m_offid[tidx]; }
    int getGatherIndex(int gatherType, int tidx);

    int findTraceIndexWithShotIdxAndOffset(int index, float offset, int& trSeq, float tol=1);
    int findTraceIndexWithRecvIdxAndOffset(int index, float offset, int& trSeq, float tol=1);
    int findTraceIndexWithCdpIdxAndOffset(int index, float offset, int& trSeq, float tol=1);

    int findTraceSequenceWithinGather(int gatherType, int gIdx, int trIdx);

    int getTraceIndexWithinWholeDataset(int gType, int gIdx, int tSeq);

    void setAmplitudeDockPointer(Sdp2dAmplitudeDisplayDock* p) { m_ampDock=p; }
    Sdp2dAmplitudeDisplayDock* getAmplitudeDockPointer(void) const  { return m_ampDock; }

    void hideTraceLocationPointOnAmpDock(void);

    void showAllDisplayWidgets() override;

    void saveIndexFile(void);
    void saveTraceHeaderToIndexFile(int trIdx);

    void addOneMutePickPoint(int trIdx, float time);
    void removeOneMutePickPoint(int trIdx);
    void setMuteValuesOfCurrentGather(bool visible=false);
    void setTracesMute(int gatherType, int gatherIdx, short* topMute, short* btmMute);
    void getTracesMute(int gatherType, int gatherIdx, short* muteValues, int forceFlag = 0);
    void setTraceMute(int traceIdx, short value, int writeHeader=2);
    void calMuteForAllTraces(short* allMutes, int muteType);
    short* getTopMuteForAllTraces(void);
    short* getBtmMuteForAllTraces(void);

    void setupStackVelocityAnalysis(Sdp2dQDomDocument* m_domval, bool processWholeData);
    void prepareStackVelocityAnalysis();
    Sdp2dVelSemblanceDisplayArea* getVelSemblanceDisplay(void) const { return m_velSemb; }
    void removeAdjunctiveDisplays(void) override;
    void loadPickedNMOVelocities(void);
    void unloadPickedNMOVelocities(void);

    QMultiMap<int, value_idx>& getNMOVelocityPicks(void) { return m_velPicks;}

    float** getinputGatherUsingOutputOption(int gatherIdx);


private:
    void estimate2DAcquisitionGeometry(void);

    void fillIndexStructure(void) override;
    void collectTraceHeaderInfo(bool knowOffInfo=false);
    void collectGathersInfo(void) override;
    void buildCDPIndex(void);
    void buildOffsetIndex(void);
    void findLineEndPoints(void);

    void loadIndexFile(void);
    void saveGathersIndex(ofstream& outfile, gathers *p);
    void loadGathersIndex(ifstream& infile,  gathers *p);
    bool isIdxFileHasNMOVelocities();

    int getNumberOfTracesInOneGather(int seq, gathers *p);
    void outputSegyWithLocalFormat(const string& segy_fname, gathers* p);

    void createDataSummaryInfo(bool flag=false) override;
    int findTraceIndexWithGroupIdxAndOffset(gathers* gp, int gIdx, float offset, int& trSeq, float tol);
    void updateMuteDisplayOnAttributeDock(int trIdx);

private:

    Sdp2dAmplitudeDisplayDock* m_ampDock;
    Sdp2dVelSemblanceDisplayArea* m_velSemb;
    QMultiMap<int, value_idx> m_velPicks;

    gathers* m_pco;    // common Offsets
    gathers* m_pcr;    // common Receivers
    gathers* m_pcs;    // common Shots

    int* m_srcid;
    int* m_offid;
    int* m_recid;
    float* m_srcx;
    float* m_srcy;
    float* m_recx;
    float* m_recy;

    float* m_offset;
    float* m_sdepth;
    float* m_selev;
    float* m_relev;

};

#endif // SEISMICDATA2DPRESTACK_H
