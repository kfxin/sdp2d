#include "sdp2dPreStackMutePicks.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "sdp2dQDomDocument.h"
#include <QList>

using namespace std;

Sdp2dPreStackMutePicks::Sdp2dPreStackMutePicks(SeismicData2DPreStack* sd2d, short* muteTime, int muteType)
{
    m_sd2d = sd2d;
    m_muteTime = muteTime;
    m_muteType = muteType;
    m_dt = sd2d->getTimeSampleRateInUs()/1000000.0;
    m_tolPicks = 2;
    m_tolPredicts = 4;
    m_extrapolation = true;
    m_interpolation = true;
    m_taperlength = 0;

    int ntr = m_sd2d->getNumberOfTraces();
    m_filledMutes = new short [ntr];
}

Sdp2dPreStackMutePicks::~Sdp2dPreStackMutePicks()
{
    m_picksOffIdx.clear();
    m_picksSrcIdx.clear();
    m_picksRecIdx.clear();
    m_picksCdpIdx.clear();
    delete [] m_filledMutes;
}

void Sdp2dPreStackMutePicks::addPicks()
{
    int ntr = m_sd2d->getNumberOfTraces();
    for(int i=0; i<ntr; i++) {
        if(m_muteTime[i] > 0) {
            addPick(i, m_muteTime[i], 0);
            //cout << "i="<<i<< " time="<<m_muteTime[i] << endl;
        }
    }
    gathers* gp = m_sd2d->getShotGatherIndex();
    calMuteForAllTraces(gp);
}

void Sdp2dPreStackMutePicks::addPick(int traceIdx, short time, int writeHeader)
{
    int isg = m_sd2d->getGatherIndex(DisplayGatherType::CommonShot, traceIdx);
    int ist = m_sd2d->findTraceSequenceWithinGather(DisplayGatherType::CommonShot, isg, traceIdx) - 1;
    if(!m_picksSrcIdx.contains(isg, ist)){
        m_picksSrcIdx.insert(isg, ist);
    }

    int icg = m_sd2d->getGatherIndex(DisplayGatherType::CommonDepthPoint, traceIdx);
    int ict = m_sd2d->findTraceSequenceWithinGather(DisplayGatherType::CommonDepthPoint, icg, traceIdx) - 1;
    if(!m_picksCdpIdx.contains(icg, ict)){
        m_picksCdpIdx.insert(icg, ict);
    }

    int iog = m_sd2d->getGatherIndex(DisplayGatherType::CommonOffset, traceIdx);
    int iot = m_sd2d->findTraceSequenceWithinGather(DisplayGatherType::CommonOffset, iog, traceIdx) - 1;
    if(!m_picksOffIdx.contains(iog, iot)){
        m_picksOffIdx.insert(iog, iot);
    }

    int irg = m_sd2d->getGatherIndex(DisplayGatherType::CommonReceiver, traceIdx);
    int irt = m_sd2d->findTraceSequenceWithinGather(DisplayGatherType::CommonReceiver, irg, traceIdx) - 1;
    if(!m_picksRecIdx.contains(irg, irt)){
        m_picksRecIdx.insert(irg, irt);
    }
    //cout << "traceIdx = " << traceIdx << " iog=" << iog << " iot=" << iot << " icg=" << icg << " ict=" << ict  << " isg=" << isg << " ist=" << ist << " irg=" << irg << " irt=" << irt << " time=" << time << " m_dt=" << m_dt << endl;
    m_muteTime[traceIdx] = time;
    m_sd2d->setTraceMute(traceIdx, time, writeHeader);
}

void Sdp2dPreStackMutePicks::addPick(int gType, int gIdx, int tIdx, float time)
{
    QList<IdxVal> left;
    QList<IdxVal> middle;
    QList<IdxVal> right;

    getMuteValuesOfGather(gType, gIdx, left, middle, right);

    if(middle.count() >= 1) {
        if(left.count() >= 1){
            for(int i=0; i< left.size(); i++) {
                if(left.at(i).trIdx+1 > tIdx){
                    int traceIdx = m_sd2d->getTraceIndexWithinWholeDataset(gType, gIdx, left.at(i).trIdx+1);
                    int itime = int(left.at(i).time/m_dt);
                    addPick(traceIdx, itime, 1);
                }
            }
        }
        if(right.count() >= 1){
            for(int i=0; i< right.size(); i++) {
                if(right.at(i).trIdx+1 < tIdx){
                    int traceIdx = m_sd2d->getTraceIndexWithinWholeDataset(gType, gIdx, right.at(i).trIdx+1);
                    int itime = int(right.at(i).time/m_dt);
                    addPick(traceIdx, itime, 1);
                }
            }
        }
    }

    left.clear();
    middle.clear();
    right.clear();

    int start = tIdx - m_tolPicks;
    int end = tIdx + m_tolPicks;
    int ntr = m_sd2d->getNumberOfTracesOfGather(gType, gIdx);

    if(start<1) start=1;
    if(end>ntr) end=ntr;
    for(int i=start; i<= end; i++) removePick(gType, gIdx, i, 1);

    int traceIdx = m_sd2d->getTraceIndexWithinWholeDataset(gType, gIdx, tIdx);
    //cout << "gType="<< gType << " gIdx=" << gIdx << " tIdx=" << tIdx << " traceIdx="<< traceIdx << endl;
    int itime =int(time/m_dt + 0.5);
    if(itime==0) itime = 1;
    addPick(traceIdx, itime);
}

void Sdp2dPreStackMutePicks::removePick(int gType, int gIdx, int tIdx, int writeHeader)
{
    //cout << "in removePick" << endl;
    int traceIdx = m_sd2d->getTraceIndexWithinWholeDataset(gType, gIdx, tIdx);

    m_muteTime[traceIdx] = 0;
    m_sd2d->setTraceMute(traceIdx, 0, writeHeader);

    int ig = m_sd2d->getGatherIndex(DisplayGatherType::CommonShot, traceIdx);
    int it = m_sd2d->findTraceSequenceWithinGather(DisplayGatherType::CommonShot, ig, traceIdx) - 1;
    if(m_picksSrcIdx.contains(ig, it)){
        m_picksSrcIdx.remove(ig, it);
    }

    ig = m_sd2d->getGatherIndex(DisplayGatherType::CommonDepthPoint, traceIdx);
    it = m_sd2d->findTraceSequenceWithinGather(DisplayGatherType::CommonDepthPoint, ig, traceIdx) - 1;
    if(m_picksCdpIdx.contains(ig, it)){
        m_picksCdpIdx.remove(ig, it);
    }

    ig = m_sd2d->getGatherIndex(DisplayGatherType::CommonOffset, traceIdx);
    it = m_sd2d->findTraceSequenceWithinGather(DisplayGatherType::CommonOffset, ig, traceIdx) - 1;
    if(m_picksOffIdx.contains(ig, it)){
        m_picksOffIdx.remove(ig, it);
    }

    ig = m_sd2d->getGatherIndex(DisplayGatherType::CommonReceiver, traceIdx);
    it = m_sd2d->findTraceSequenceWithinGather(DisplayGatherType::CommonReceiver, ig, traceIdx) - 1;
    if(m_picksRecIdx.contains(ig, it)){
        m_picksRecIdx.remove(ig, it);
    }
}

void Sdp2dPreStackMutePicks::getMuteValuesOfGather(int gType, int gIdx, QList<IdxVal>& left, QList<IdxVal>& middle, QList<IdxVal>& right, bool  exp, bool inp)
{
    int dType = m_sd2d->getDataType();
    if(dType != SeismicDataType::PreStack) return;

    switch (gType) {
    case DisplayGatherType::CommonShot:
        getPickedMutesOfGather(gType, gIdx, m_picksSrcIdx, left, middle, right, inp);
        break;
    case DisplayGatherType::CommonDepthPoint:
        getPickedMutesOfGather(gType, gIdx, m_picksCdpIdx, left, middle, right, inp);
        break;
    case DisplayGatherType::CommonOffset:
        //need to further check
        getPickedMutesOfOffsetGather(gIdx, left, middle, right, inp);
        break;
    case DisplayGatherType::CommonReceiver:
        getPickedMutesOfGather(gType, gIdx, m_picksRecIdx, left, middle, right, inp);
        break;
    }

    //cout << "nleft="<<left.size() << " nmiddle="<<middle.size() << " nright=" << right.size() << endl;    
    //cout << "m_extrapolation = " << m_extrapolation << endl;
    if(!m_extrapolation && !exp) return;

    QMap<int, float> picks;
    for(int i=0; i< left.size(); i++) picks.insert(left.at(i).trIdx, left.at(i).time);
    for(int i=0; i< middle.size(); i++) picks.insert(middle.at(i).trIdx, middle.at(i).time);
    for(int i=0; i< right.size(); i++) picks.insert(right.at(i).trIdx, right.at(i).time);

    QList<int> tids = picks.keys();
    if(tids.count() < 2)  return;

    //cout << "number of picks: " << tids.count();
    std::sort(tids.begin(), tids.end());

    int ntr = m_sd2d->getNumberOfTracesOfGather(gType, gIdx);
    int nmid = middle.count();
    int nleft = left.count();
    int nright = right.count();    

    //cout << "number of picks: " << tids.count() << " nleft="<< nleft << " nmid=" << nmid << " nright=" << nright << endl;

    if(gType != DisplayGatherType::CommonOffset){
        IdxVal idx1;
        IdxVal idx2;
        IdxVal lftend;
        IdxVal rhtend;

        float* offset = new float [ntr];
        m_sd2d->getOffsetValuesOfGather(gType, gIdx, ntr, offset);

        int nps = tids.count();
        if(tids.at(0) > 0){
            idx1.trIdx = tids.at(0);
            idx1.time = picks[tids.at(0)];
            idx2.trIdx = tids.at(1);
            idx2.time = picks[tids.at(1)];
            float time = calculateTimeUsingTwoOffIdx(idx1, idx2, offset, offset[0]);

            if(time >=0){
                lftend.time = time;
                lftend.trIdx = 0;
                if(nleft == 0){
                    left.append(idx1);
                }
                left.prepend(lftend);
            }
            //cout << " left idx1=" <<left.at(0).trIdx<< " time="<< left.at(0).time << " idx2=" <<left.at(1).trIdx<< " time="<< left.at(1).time << endl;
        }
        //if(nmid >1){
            //cout << " mid idx1=" <<middle.at(0).trIdx<< " time="<< middle.at(0).time << " idx2=" <<middle.at(nmid-1).trIdx<< " time="<< middle.at(nmid-1).time << endl;
        //}
        if(tids.at(nps-1)  < (ntr - 1)){
            idx1.trIdx = tids.at(nps-1);
            idx1.time = picks[tids.at(nps-1)];
            idx2.trIdx = tids.at(nps-2);
            idx2.time = picks[tids.at(nps-2)];
            float time = calculateTimeUsingTwoOffIdx(idx1, idx2, offset, offset[ntr-1]);

            if(time >=0){
                rhtend.time = time;
                rhtend.trIdx = ntr-1;
                if(nmid == 0){
                    left.append(rhtend);
                } else {
                    if(nright == 0){
                        right.append(idx1);
                    } else {
                        if(right.at(0).trIdx > middle.at(nmid-1).trIdx){
                            right.prepend(middle.at(nmid-1));
                        }
                    }
                    right.append(rhtend);
                }
            }
            //int rsize= right.count();
            //if(rsize>1)
            //cout << " right idx1=" <<right.at(rsize-2).trIdx<< " time="<< right.at(rsize-2).time << " idx2=" <<right.at(rsize-1).trIdx<< " time="<< right.at(rsize-1).time << endl;
        }
        delete [] offset;
    }else {
        int npicks = nmid + nleft + nright;
        if(nmid>0) npicks = npicks-2;
        if(npicks > 1 ){
            if(nmid > 0){
                if(nleft > 1 && left.at(0).trIdx > 0) {
                    IdxVal pair;
                    pair.time= left.at(0).time;
                    pair.trIdx= left.at(0).trIdx;
                    left.prepend(pair);
                }
                if(nright > 1 && right.at(nright-1).trIdx < ntr -1) {
                    IdxVal pair;
                    pair.time = right.at(nright-1).time;
                    pair.trIdx = right.at(nright-1).trIdx;
                    right.append(pair);
                }
            } else if(left.count() > 1){
                if(left.at(0).trIdx > 0){
                    IdxVal pair;
                    pair.time= left.at(0).time;
                    pair.trIdx= left.at(0).trIdx;
                    left.prepend(pair);
                }
                if(left.at(nleft-1).trIdx < ntr -1){
                    IdxVal pair;
                    pair.time = right.at(nright-1).time;
                    pair.trIdx = right.at(nright-1).trIdx;
                    left.append(pair);
                }
            }
        }
    }
/*
    if(left.count() == 0){
        cout << "gIdx = "<< gIdx << " No left" << endl;
    } else {
        for(int i=0; i< left.count(); i++){
            cout << "gIdx = "<< gIdx << " Left: i="<< i << " trIdx="<<left.at(i).trIdx << " time="<< left.at(i).time << endl;
        }
    }

    if(middle.count() == 0){
        cout << "gIdx = "<< gIdx << " No middle" << endl;
    } else {
        for(int i=0; i< middle.count(); i++){
            cout << "gIdx = "<< gIdx << " Middle: i="<< i << " trIdx="<<middle.at(i).trIdx << " time="<< middle.at(i).time << endl;
        }
    }

    if(right.count() == 0){
        cout << "gIdx = "<< gIdx << " No right" << endl;
    } else {
        for(int i=0; i< right.count(); i++){
            cout << "gIdx = "<< gIdx << " Right: i="<< i << " trIdx="<<right.at(i).trIdx << " time="<< right.at(i).time << endl;
        }
    }

    cout << endl;
*/
}

void Sdp2dPreStackMutePicks::getPickedMutesOfGather(int gType, int gIdx, QMultiMap<int, int>& picks, QList<IdxVal>& left, QList<IdxVal>& middle, QList<IdxVal>& right, bool inp)
{
    if(picks.count() == 0) return;
    int ntr = m_sd2d->getNumberOfTracesOfGather(gType, gIdx);
    int nps = 0;
    int fstIdx = ntr-1;
    int lstIdx = ntr-1;

    left.clear();
    middle.clear();
    right.clear();

    if(picks.contains(gIdx)){
        QList<int> tids = picks.values(gIdx);  // the key is Seq of traces
        std::sort(tids.begin(), tids.end());

        //for (int i = 0; i < tids.size(); ++i){
        //    cout << "gIdx = " << gIdx << ",  tIdx = " << tids.at(i) << endl;
        //}

        nps = tids.count();
        fstIdx = tids.at(0);
        lstIdx = tids.at(nps-1);

        for (int i = 0; i < nps; i++){
            int trlidx   = tids.at(i);
            int trgidx = m_sd2d->getTraceIndexWithinWholeDataset(gType, gIdx, trlidx+1);
            float time = m_muteTime[trgidx] * m_dt;
            IdxVal pair(trlidx, time);

            middle.append(pair);
        }
    }

    //cout << "m_interpolation = " << m_interpolation << endl;
    if(!m_interpolation && !inp) return;
    //cout << "gIdx=" << gIdx << " ntr=" << ntr << " fstIdx=" << fstIdx << " lstIdx=" << lstIdx << " nps=" << nps << endl;

    if(fstIdx > 0 || nps==0){ //extrapolate the left part of mute picks
        int trgidx = -1;
        if(middle.count() >0){
            int trlidx = middle.at(0).trIdx + 1;
            trgidx = m_sd2d->getTraceIndexWithinWholeDataset(gType, gIdx, trlidx);
        }
        findPicksForMuteExtrapolation(gType, gIdx, left, 0, fstIdx-1, trgidx);
        if(middle.count() >0 && left.count() >0){
            int nleft = left.count() -1;
            if(left.at(nleft).trIdx < middle.at(0).trIdx) left.append(middle.at(0));
        }
        //cout << " size of left = "<< left.count() << endl;
    }
    if(lstIdx < ntr-1){ //extrapolate the right part of mute picks
        int trgidx = -1;
        int nps = middle.count();
        if(nps > 0){
            int trlidx = middle.at(nps-1).trIdx + 1;
            trgidx = m_sd2d->getTraceIndexWithinWholeDataset(gType, gIdx, trlidx);
            //cout << "trgidx = " << trgidx << endl;
        }
        findPicksForMuteExtrapolation(gType, gIdx, right, lstIdx+1, ntr-1, trgidx);
        //cout << " size of right = "<< right.count() << endl;
        if(middle.count() >0 && right.count() >0){
            int nmid = middle.count() -1;
            if(middle.at(nmid).trIdx < right.at(0).trIdx) {
                if(qAbs(middle.at(nmid).trIdx - right.at(0).trIdx) > 2){
                    right.append(middle.at(nmid));
                } else {
                    right[0].trIdx = middle.at(nmid).trIdx;
                    right[0].time = middle.at(nmid).time;
                }
            }
        }
        //cout << " size of right = "<< right.count() << endl;
    }

}

void Sdp2dPreStackMutePicks::getPickedMutesOfOffsetGather(int gIdx, QList<IdxVal>& left, QList<IdxVal>& middle, QList<IdxVal>& right, bool inp)
{
    Q_UNUSED(inp);
    if(m_picksOffIdx.count() == 0) return;

    //int ntr = m_sd2d->getNumberOfTracesOfGather(DisplayGatherType::CommonOffset, gIdx);

    left.clear();
    middle.clear();
    right.clear();

    if(m_picksOffIdx.contains(gIdx)){
        QList<int> tids = m_picksOffIdx.values(gIdx);  // the key is Seq of traces
        std::sort(tids.begin(), tids.end());

        int nps = tids.count();

        for (int i = 0; i < nps; i++){
            int trlidx   = tids.at(i);
            int trgidx = m_sd2d->getTraceIndexWithinWholeDataset(DisplayGatherType::CommonOffset, gIdx, trlidx+1);
            float time = m_muteTime[trgidx] * m_dt;
            IdxVal pair(trlidx, time);
            middle.append(pair);
        }
    }


}

void Sdp2dPreStackMutePicks::findPicksForMuteExtrapolation(int gType, int gIdx, QList<IdxVal>& picks, int fstIdx, int lstIdx, int pickedIdx)
{
    QVector<Idx2Val> allPicks;

    for(int idx = fstIdx; idx <= lstIdx; idx++){
        int trIdx = m_sd2d->getTraceIndexWithinWholeDataset(gType, gIdx, idx+1);
        if(abs(trIdx - pickedIdx) < m_tolPicks)continue;

        int offIdx = m_sd2d->getOIdxOfATrace(trIdx);
        int cdpIdx = m_sd2d->getCIdxOfATrace(trIdx);
        int cdpDiff = 0;
        float time = -1;

        if(m_picksOffIdx.contains(offIdx)){
            //cout << "idx="<< idx << " trIdx=" << trIdx << " cdpIdx=" << cdpIdx << " offIdx="<< offIdx << " has number of picks: "<< m_picksOffIdx.count(offIdx) << endl;
            if(m_picksOffIdx.count(offIdx) == 1){
                int trlidx = m_picksOffIdx.value(offIdx);
                int trgidx = m_sd2d->getTraceIndexWithinWholeDataset(DisplayGatherType::CommonOffset, offIdx, trlidx+1);                
                if(abs(trgidx - pickedIdx) < m_tolPicks)continue;
                int cidx = m_sd2d->getCIdxOfATrace(trgidx);
                cdpDiff = abs(cdpIdx - cidx);
                time = m_muteTime[trgidx] * m_dt;
                //cout << "trlidx=" << trlidx << " trgidx=" << trgidx << " cdpDiff=" << cdpDiff <<  " time=" << time << endl;
            } else if(m_picksOffIdx.count(offIdx) == 2) {
                QList<int> tids = m_picksOffIdx.values(offIdx);
                int trgidx1 = m_sd2d->getTraceIndexWithinWholeDataset(DisplayGatherType::CommonOffset, offIdx, tids.at(0)+1);
                int trgidx2 = m_sd2d->getTraceIndexWithinWholeDataset(DisplayGatherType::CommonOffset, offIdx, tids.at(1)+1);
                if(abs(trgidx1 - pickedIdx) < m_tolPicks || abs(trgidx2 - pickedIdx) < m_tolPicks)continue;
                if(abs(trgidx1 - pickedIdx) < m_tolPicks){
                    int cidx2 = m_sd2d->getCIdxOfATrace(trgidx2);
                    cdpDiff = abs(cdpIdx - cidx2);
                    time = m_muteTime[trgidx2] * m_dt;
                } else if(abs(trgidx2 - pickedIdx) < m_tolPicks) {
                    int cidx1 = m_sd2d->getCIdxOfATrace(trgidx1);
                    cdpDiff = abs(cdpIdx - cidx1);
                    time = m_muteTime[trgidx1] * m_dt;
                } else {
                    int cidx1 = m_sd2d->getCIdxOfATrace(trgidx1);
                    int cidx2 = m_sd2d->getCIdxOfATrace(trgidx2);
                    cdpDiff = min(abs(cdpIdx - cidx1), abs(cdpIdx - cidx2));
                    time = calculateTimeUsingCDPIdx(trgidx1, trgidx2, cdpIdx);
                }
                //cout << "offIdx=" << offIdx << " cdp1=" << tids.at(0) << " cdp2=" << tids.at(1) << " cdpIdx=" << cdpIdx <<  " time=" << time << endl;
            } else {
                QList<int> tids = m_picksOffIdx.values(offIdx);
                int nps = tids.count();
                QVector<int> cdpids;
                QVector<int> cdpdif;
                cdpids.resize(nps);
                cdpdif.resize(nps);
                int mindiff = std::numeric_limits<int>::max();
                int minidx = -1;
                for(int i=0; i< nps; i++){
                    int trgidx = m_sd2d->getTraceIndexWithinWholeDataset(DisplayGatherType::CommonOffset, offIdx, tids.at(i)+1);                    
                    cdpids[i] = m_sd2d->getCIdxOfATrace(trgidx);
                    cdpdif[i] = abs(cdpids[i] - cdpIdx);
                    if(cdpdif[i] < mindiff){
                        mindiff =  cdpdif[i];
                        minidx = i;
                    }
                    //cout << "offIdx=" << offIdx << " i=" << i << " trIdx=" << cdpids[i] << " diff=" << cdpdif[i] << " time=" << m_muteTime[trgidx] << endl;
                }
                int mp1idx = 0;
                if(minidx == 0) {
                    mp1idx =1;
                } else if(minidx == nps-1){
                    mp1idx = nps - 2;
                } else {
                    if(cdpdif[minidx -1] > cdpdif[minidx + 1]) mp1idx = minidx +1;
                    else mp1idx = minidx - 1;
                }

                int trgidx1 = m_sd2d->getTraceIndexWithinWholeDataset(DisplayGatherType::CommonOffset, offIdx, tids.at(minidx)+1);
                int trgidx2 = m_sd2d->getTraceIndexWithinWholeDataset(DisplayGatherType::CommonOffset, offIdx, tids.at(mp1idx)+1);
                cdpDiff = cdpdif.at(minidx);
                time = calculateTimeUsingCDPIdx(trgidx1, trgidx2, cdpIdx);
                //cout << "cdpIdx=" << cdpIdx << " minidx=" << cdpids[minidx] << " mp1idx=" << cdpids[mp1idx] << " time=" << time << endl;
            }

            if(time < 0) continue;
            //cout << " idx=" << idx << " time=" << time << endl;
            Idx2Val pair(idx, cdpDiff, time);
            allPicks.append(pair);
        }
    }

    if(allPicks.count() > 1){
        QVector<Idx2Val> decimPicks;
        decimPicks.append(allPicks.at(0));
        for(int i=1; i< allPicks.count(); i++){
            int last = decimPicks.count() -1;
            if(allPicks.at(i).trIdx - decimPicks.at(last).trIdx < m_tolPredicts){
                //if(allPicks.at(i).intVal < decimPicks.at(last).intVal){
                //    decimPicks[last] = allPicks[i];
                //}
                continue;
            } else {
                decimPicks.append(allPicks.at(i));
            }
        }
        for(int i=0; i< decimPicks.count(); i++){
            picks.append(IdxVal(decimPicks.at(i).trIdx, decimPicks.at(i).time));
        }
        decimPicks.clear();
    }else{
        for(int i=0; i< allPicks.count(); i++){
            picks.append(IdxVal(allPicks.at(i).trIdx, allPicks.at(i).time));
        }
    }
    allPicks.clear();

}

float Sdp2dPreStackMutePicks::calculateTimeUsingCDPIdx(int idx1, int idx2, int cdpIdx)
{
    if(idx1 == idx2) return -1;

    int cidx1 = m_sd2d->getCIdxOfATrace(idx1);
    float time1 = m_muteTime[idx1] * m_dt;

    int cidx2 = m_sd2d->getCIdxOfATrace(idx2);
    float time2 = m_muteTime[idx2] * m_dt;

    if(cidx2 == cidx1 || cdpIdx == cidx2) return time2;
    float time = (time1 - time2)/(cidx1 - cidx2)*(cdpIdx - cidx2) + time2;
    return time;
}

float Sdp2dPreStackMutePicks::calculateTimeUsingTwoOffIdx(IdxVal& idx1, IdxVal& idx2, float* offset, float value)
{
    if( idx1.trIdx == idx2.trIdx) return -1;
    float time1 = idx1.time;
    float time2 = idx2.time;
    float off1 = abs(offset[idx1.trIdx]);
    float off2 = abs(offset[idx2.trIdx]);
    value = abs(value);

    if(abs(off1-off2) < 0.001 || abs(value-off2) < 0.001) return time2;

    float time =-1;

    time = (time1 - time2)/(off1 - off2)*(value - off2) + time2;

    //cout << " gId="<< gIdx << " i1=" << idx1.trIdx << " o1="<< off1 << " t1=" << time1 << " i2="<< idx2.trIdx  << " o2=" << off2 << " t2=" << time2 << " value=" << value << " time=" << time << endl;

    return time;
}

void Sdp2dPreStackMutePicks::calMuteForAllTraces(gathers* gp)
{
    int ntr = m_sd2d->getNumberOfTraces();
    int ns = m_sd2d->getSamplesPerTraces();
    int dtus = m_sd2d->getTimeSampleRateInUs();
    float invdt = 1000000.0/float(dtus);
    float* allOffset = m_sd2d->getOffsetValuePointer();
    memset((void*)m_filledMutes,  0, ntr*sizeof(short));

    int traceIdx = 0;
    QList<IdxVal> left;
    QList<IdxVal> middle;
    QList<IdxVal> right;

    for(int ig = 0; ig < gp->ngroups; ig++){
        int ntr= gp->group[ig].traceIdx.size();

        int gatherIdx = ig + gp->minGroupValue;

        getMuteValuesOfGather(1, gatherIdx, left, middle, right, m_extrapolation, m_interpolation);

        QMap<int, float> picks;
        for(int i=0; i< left.size(); i++) picks.insert(left.at(i).trIdx, left.at(i).time);
        for(int i=0; i< middle.size(); i++) picks.insert(middle.at(i).trIdx, middle.at(i).time);
        for(int i=0; i< right.size(); i++) picks.insert(right.at(i).trIdx, right.at(i).time);

        QList<int> tids = picks.keys();

        //cout << "ig=" << ig << " ng=" << m_pcs->ngroups << " tids.count = " << tids.count() << endl;;

        if(tids.count() < 2)  {
            if(m_muteType == MuteType::BottomMute){
                for(int i=0; i< ntr; i++){
                    traceIdx = gp->group[ig].traceIdx[i];
                    m_filledMutes[traceIdx]  = ns;
                }
            } else {
                for(int i=0; i< ntr; i++){
                    traceIdx = gp->group[ig].traceIdx[i];
                    m_filledMutes[traceIdx]  = 0;
                }
            }

            continue;
        }
        std::sort(tids.begin(), tids.end());

        float* offset = new float [ntr];
        short* muteValues = new short [ntr];

        for(int i=0; i< ntr; i++){
            traceIdx = gp->group[ig].traceIdx[i];
            offset[i] = allOffset[traceIdx];
            muteValues[i] = 0;
        }

        for(int i=0; i<tids.count(); i++){
            muteValues[tids.at(i)] = int(picks[tids.at(i)]*invdt);
        }

        for(int i=1; i<tids.count(); i++){
            int i1 = tids.at(i-1);
            int i2 = tids.at(i);
            float t1 = picks[i1];
            float t2 = picks[i2];
            float off1 = abs(offset[i1]);
            float off2 = abs(offset[i2]);
            for(int j=i1; j <= i2; j++){
                if(muteValues[j] > 0) continue;
                float off = abs(offset[j]);
                if(abs(off1-off2) > 0.001 && abs(off-off2) > 0.001){
                    muteValues[j] = int(((t1 - t2)/(off1 - off2)*(off - off2) + t2)*invdt);
                } else {
                    muteValues[j] = int(t2*invdt);
                }
                //if(ig == 0) cout << "muteType=" << muteType << " j="<< j << " off=" << off << " i1=" << i1 << " i2=" << i2 << " off1=" << off1<< " off2=" << off2  << " t1=" << t1 << " t2=" << t2 <<  " time=" << (t1 - t2)/(off1 - off2)*(off - off2) + t2 << endl;
            }
        }

        if(m_muteType == MuteType::BottomMute){
            for(int i=0; i< ntr; i++){
                traceIdx = gp->group[ig].traceIdx[i];
                if(muteValues[i] <  1 || muteValues[i] > ns) m_filledMutes[traceIdx]  = ns;
                else m_filledMutes[traceIdx]  = muteValues[i];
            }
        } else {
            for(int i=0; i< ntr; i++){
                traceIdx = gp->group[ig].traceIdx[i];
                if(muteValues[i] < 0 ) m_filledMutes[traceIdx]  = 0;
                else if(muteValues[i] > ns) m_filledMutes[traceIdx]  = ns;
                else m_filledMutes[traceIdx]  = muteValues[i];

                //if(ig==0) cout << "muteType=" << muteType<< " ig="<< ig << " it="<< i << " traceIdx="<< traceIdx << " mute=" << allMutes[traceIdx] << endl;
            }
        }


        delete [] offset;
        delete [] muteValues;
    }
    //cout << " calculate mute done" << endl;
}

void Sdp2dPreStackMutePicks::setParametersToDom(Sdp2dQDomDocument* domDoc)
{
    domDoc->setParameterInGroup(QString("TaperLength"), QString::number(m_taperlength));
    if(m_interpolation){
        domDoc->setParameterInGroup(QString("Interpolate"), QString("True"));
    } else {
        domDoc->setParameterInGroup(QString("Interpolate"), QString("False"));
    }
    if(m_extrapolation){
        domDoc->setParameterInGroup(QString("Extrapolate"), QString("True"));
    } else {
        domDoc->setParameterInGroup(QString("Extrapolate"), QString("False"));
    }

}

bool Sdp2dPreStackMutePicks::getParametersFromDom(Sdp2dQDomDocument* domVal)
{
    m_taperlength = domVal->getParameterInGroup(QString("TaperLength")).toInt();
    m_extrapolation = true;
    if(domVal->getParameterInGroup(QString("Extrapolate")).compare("True") != 0){
        m_extrapolation = false;
    }
    m_interpolation = true;
    if(domVal->getParameterInGroup(QString("Interpolate")).compare("True") != 0){
        m_interpolation = false;
    }
    return true;
}
