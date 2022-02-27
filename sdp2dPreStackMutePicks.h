#ifndef SDP2DPRESTACKMUTEPICKS_H
#define SDP2DPRESTACKMUTEPICKS_H

#include "seismicdata2d.h"

#include <QMultiMap>

class SeismicData2DPreStack;
class Sdp2dQDomDocument;

class Sdp2dPreStackMutePicks
{
public:
    Sdp2dPreStackMutePicks(SeismicData2DPreStack* sd2d, short* muteTime, int muteType);
    ~Sdp2dPreStackMutePicks();

    int getMuteType(void) const { return m_muteType;}
    void addPick(int gType, int gIdx, int tIdx, float time);
    void removePick(int gType, int gIdx, int tIdx, int writeHeader=2);
    void addPick(int tIdx, short time, int writeHeader=2);
    void addPicks();
    void setGatherTypeForInterpolatingMute(int gType) { m_pickingGatherType = gType; }

    void setExtrapolateMute(bool extra) { m_extrapolation = extra;}
    void setInterpolateMute(bool inter) { m_interpolation = inter;}
    bool getExtrapolateMute(void) const { return m_extrapolation; }
    bool getInterpolateMute(void) const { return m_interpolation; }

    void getMuteValuesOfGather(int gType, int gIdx, QList<IdxVal>& left, QList<IdxVal>& middle, QList<IdxVal>& right, bool  exp=false, bool inp=false);
    void getPickedMutesOfGather(int gType, int gIdx, QMultiMap<int, int>& picks, QList<IdxVal>& left, QList<IdxVal>& middle, QList<IdxVal>& right, bool inp=false);
    void getPickedMutesOfOffsetGather(int gIdx, QList<IdxVal>& left, QList<IdxVal>& middle, QList<IdxVal>& right, bool inp=false);

    int getMuteTaperLength(void) const {return m_taperlength; }
    void setMuteTaperLength(int val) { m_taperlength = val; }
    short* getFilledMutes(void) { return m_filledMutes; }

    void calMuteForAllTraces(gathers* gp);

    void setParametersToDom(Sdp2dQDomDocument* domDoc);
    bool getParametersFromDom(Sdp2dQDomDocument* domDoc);

private:
    SeismicData2DPreStack* m_sd2d;
    short* m_muteTime;
    float m_dt;
    int m_pickingGatherType;
    int m_muteType;
    int m_tolPicks;
    int m_tolPredicts;
    int m_taperlength;
    bool m_extrapolation;
    bool m_interpolation;
    short* m_filledMutes;

    QMultiMap<int, int> m_picksOffIdx;
    QMultiMap<int, int> m_picksSrcIdx;
    QMultiMap<int, int> m_picksRecIdx;
    QMultiMap<int, int> m_picksCdpIdx;

    void findPicksForMuteExtrapolation(int gType, int gIdx, QList<IdxVal>& picks, int fstIdx, int lstIdx, int pickedIdx);
    float calculateTimeUsingCDPIdx(int idx1, int idx2, int cdpIdx);
    float calculateTimeUsingTwoOffIdx(IdxVal& idx1, IdxVal& idx2, float* offset, float value);
};

#endif // SDP2DPRESTACKMUTEPICKS_H
