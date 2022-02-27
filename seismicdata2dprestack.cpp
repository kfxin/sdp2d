#include "seismicdata2dprestack.h"
#include "seismicdataprocessing2d.h"
#include "sdp2dAmplitudeDisplayDock.h"
#include "sdp2dFrequencyAnalysisDock.h"
#include "sdp2dMainGatherDisplayArea.h"
#include "sdp2dPreStackMutePicks.h"
#include "sdp2dQDomDocument.h"
#include "sdp2dStackVelocityAnalysis.h"
#include "sdp2dVelSemblanceDisplayArea.h"
#include "sdp2dProcessedGatherDisplayArea.h"
#include "sdp2dUtils.h"
#include "sdp2dSegy.h"

#include <iostream>
#include <cstring>
#include <cfloat>
#include <cmath>

#include <QtWidgets>
#include <QStringList>
#include <QString>
#include <QPointF>


//NOTE: 2020-09-03
//      The current version can only apply on land/OBS/OBN seismic data.
//      Namely, the receivers are at fixed position.

using namespace std;


SeismicData2DPreStack::SeismicData2DPreStack(const string& segy_file_name, SeismicDataProcessing2D* mainwin, bool loadSEGYHeader)
    : SeismicData2D(segy_file_name, mainwin, loadSEGYHeader)
{
    m_datatype = SeismicDataType::PreStack;

    m_ampDock = nullptr;
    m_velSemb  = nullptr;

    m_pcs  = new gathers(1);
    m_pco  = new gathers(3);
    m_pcr  = new gathers(4);

    m_srcid = new int [m_ntr];
    m_offid = new int [m_ntr];
    m_recid = new int [m_ntr];

    m_srcx  = new float [m_ntr];
    m_srcy  = new float [m_ntr];
    m_recx  = new float [m_ntr];
    m_recy  = new float [m_ntr];

    m_offset= new float [m_ntr];
    m_sdepth= new float [m_ntr];
    m_selev = new float [m_ntr];
    m_relev = new float [m_ntr];

    m_tMute = new Sdp2dPreStackMutePicks(this, m_topMute, MuteType::TopMute);
    m_bMute = new Sdp2dPreStackMutePicks(this, m_btmMute, MuteType::BottomMute);
    m_currentMute = m_tMute;

    loadIndexFile();

    //collectGathersInfo();

}

SeismicData2DPreStack::~SeismicData2DPreStack()
{
    if (m_pcs != nullptr) delete m_pcs;
    if (m_pco != nullptr) delete m_pco;
    if (m_pcr != nullptr) delete m_pcr;

    if (m_srcid != nullptr) delete [] m_srcid;
    if (m_offid != nullptr) delete [] m_offid;
    if (m_recid != nullptr) delete [] m_recid;
    if (m_offset!= nullptr) delete [] m_offset;
    if (m_srcx  != nullptr) delete [] m_srcx;
    if (m_srcy  != nullptr) delete [] m_srcy;
    if (m_recx  != nullptr) delete [] m_recx;
    if (m_recy  != nullptr) delete [] m_recy;
    if (m_sdepth!= nullptr) delete [] m_sdepth;
    if (m_selev != nullptr) delete [] m_selev;
    if (m_relev != nullptr) delete [] m_relev;
}

void SeismicData2DPreStack::removeAdjunctiveDisplays(void)
{
    //cout << "removeAdjunctiveDisplays start  widgetList.count() = " << widgetList.count() << endl;
    if(m_outGthDisplay != nullptr) {
        widgetList.takeAt(widgetList.indexOf(m_outGthDisplay));
        m_outGthDisplay->close();
        m_outGthDisplay = nullptr;
    }

    if(m_velSemb != nullptr){
        widgetList.takeAt(widgetList.indexOf(m_velSemb));
        m_velSemb->close();
        m_velSemb = nullptr;
    }
    //cout << "removeAdjunctiveDisplays end  widgetList.count() = " << widgetList.count() << endl;
}

void SeismicData2DPreStack::collectGathersInfo(void)
{
    collectTraceHeaderInfo();
    calculateSlopeOf2DLine();
    estimate2DAcquisitionGeometry();
    findLineEndPoints();    
}

void SeismicData2DPreStack::setupDataIndex()
{
    if(!isIndexFileExist()){
        collectGathersInfo();
        cout << " start to build CDP" << endl;
        cout << "m_dcdp="<< m_dcdp<< endl;
        cout << "m_doff="<< m_doff<< endl;
    } else {
        collectTraceHeaderInfo();
    }

    if(m_doff > FLT_EPSILON)  buildOffsetIndex();
    if(m_dcdp > FLT_EPSILON)  buildCDPIndex();

    buildIndexUsingOneCoordinate(m_recx, m_recid, m_pcr);
    buildIndexUsingOneCoordinate(m_srcx, m_srcid, m_pcs);

    //cout << "m_firstXY.x=" << m_firstXY.x() << " m_firstXY.y=" << m_firstXY.y() << endl;
    //cout << "isIndexFileExist = " << isIndexFileExist() << endl;
    if(!isIndexFileExist()){

        fillIndexStructure();
        createDataSummaryInfo(true);
        saveIndexFile();

        //int ncdp = getNumberOfCDPs();
        //for(int i=0; i< ncdp; i++){
        //    cout << "1. Fold of cdp "<< i+1 << " is " << getFoldOfOneCDP(i+1) << endl;
        //}
    }
    m_tMute->addPicks();
    m_bMute->addPicks();

/*
    string segy_file_name = m_sgy->getSEGYFileName();
    size_t loc = segy_file_name.rfind('.', segy_file_name.length());
    string fn = segy_file_name.substr(0, loc)+"_csg.segy";
    cout << "output file name: "<< fn.c_str() << endl;
    outputSegyWithLocalFormatInShotOrder(fn);
    fn = segy_file_name.substr(0, loc)+"_cdp.segy";
    outputSegyWithLocalFormatInCDPOrder(fn);
    fn = segy_file_name.substr(0, loc)+"_crg.segy";
    outputSegyWithLocalFormatInRecvOrder(fn);
    fn = segy_file_name.substr(0, loc)+"_cog.segy";
    outputSegyWithLocalFormatInOffsetOrder(fn);
   */
    cout << "m_ntr: " << m_ntr << endl;
    cout << "m_dtus: " << m_dtus << endl;
    cout << "m_ns: " << m_ns << endl;

}

void SeismicData2DPreStack::fillIndexStructure(void)
{
    int sid, rid, cid, oid;

    cout << "m_pcs->ngroups="<< m_pcs->ngroups << " m_pcdp->ngroups=" << m_pcdp->ngroups << endl;

    m_pcs->group.resize(m_pcs->ngroups);
    m_pcr->group.resize(m_pcr->ngroups);
    m_pco->group.resize(m_pco->ngroups);
    m_pcdp->group.resize(m_pcdp->ngroups);

    cout << "m_pcs->ngroups="<< m_pcs->ngroups << " m_pcdp->ngroups=" << m_pcdp->ngroups << endl;
    float* cdpVal = new float [m_ntr];
    for(int i=0; i< m_ntr; i++){
        cdpVal[i] = m_cdpid[i];
        //if(!m_trflag[i]) continue;
        sid = m_srcid[i] - m_pcs->minGroupValue;
        rid = m_recid[i] - m_pcr->minGroupValue;
        oid = m_offid[i] - m_pco->minGroupValue;
        cid = m_cdpid[i] - m_pcdp->minGroupValue;
        if(oid < 1) cout <<"oid=" << oid << " i="<< i << " m_offid=" << m_offid[i] << " offset minGroupValue=" << m_pco->minGroupValue << endl;

        m_pcs->group[sid].ntraces += 1;
        m_pcs->group[sid].groupValue1 += m_srcx[i];
        m_pcs->group[sid].groupValue2 += m_srcy[i];
        m_pcs->group[sid].groupValue3 += m_selev[i];
        m_pcs->group[sid].groupValue4 += m_sdepth[i];
        m_pcs->group[sid].traceIdx.push_back(i);
        //cout << "i=" << i << " sid=" << sid << " sx=" << m_pcs->group[sid].groupValue1/m_pcs->group[sid].ntraces <<" sy="<< m_pcs->group[sid].groupValue2/m_pcs->group[sid].ntraces << endl;

        m_pcr->group[rid].ntraces += 1;
        m_pcr->group[rid].groupValue1 += m_recx[i];
        m_pcr->group[rid].groupValue2 += m_recy[i];
        m_pcr->group[rid].groupValue3 += m_relev[i];        
        m_pcr->group[rid].traceIdx.push_back(i);

        m_pcdp->group[cid].ntraces += 1;
        m_pcdp->group[cid].groupValue1 += m_cdpx[i];
        m_pcdp->group[cid].groupValue2 += m_cdpy[i];        
        m_pcdp->group[cid].traceIdx.push_back(i);

        m_pco->group[oid].ntraces += 1;
        m_pco->group[oid].groupValue1 += m_offset[i];        
        m_pco->group[oid].traceIdx.push_back(i);
    }
    for(int ig = 0; ig < m_pcs->ngroups; ig++){
        int ntr = m_pcs->group[ig].ntraces;
        if(ntr == 0) continue;
        m_pcs->group[ig].groupValue1 = m_pcs->group[ig].groupValue1/ntr;
        m_pcs->group[ig].groupValue2 = m_pcs->group[ig].groupValue2/ntr;
        m_pcs->group[ig].groupValue3 = m_pcs->group[ig].groupValue3/ntr;
        m_pcs->group[ig].groupValue4 = m_pcs->group[ig].groupValue4/ntr;
    }
    for(int ig = 0; ig < m_pcr->ngroups; ig++){
        int ntr = m_pcr->group[ig].ntraces;
        if(ntr == 0) continue;
        m_pcr->group[ig].groupValue1 = m_pcr->group[ig].groupValue1/ntr;
        m_pcr->group[ig].groupValue2 = m_pcr->group[ig].groupValue2/ntr;
        m_pcr->group[ig].groupValue3 = m_pcr->group[ig].groupValue3/ntr;
    }
    for(int ig = 0; ig < m_pcdp->ngroups; ig++){
        int ntr = m_pcdp->group[ig].ntraces;
        if(ntr == 0) continue;
        m_pcdp->group[ig].groupValue1 = m_pcdp->group[ig].groupValue1/ntr;
        m_pcdp->group[ig].groupValue2 = m_pcdp->group[ig].groupValue2/ntr;
    }
    for(int ig = 0; ig < m_pco->ngroups; ig++){
        int ntr = m_pco->group[ig].ntraces;
        if(ntr == 0) continue;
        m_pco->group[ig].groupValue1 = m_pco->group[ig].groupValue1/ntr;
    }
    sortSubsetOfGathers(m_pcs, m_offset);
    sortSubsetOfGathers(m_pcr, m_offset);
    sortSubsetOfGathers(m_pcdp, m_offset);
    sortSubsetOfGathers(m_pco, cdpVal);

    delete [] cdpVal;
    cout << "fillIndexStructure done" << endl;
}

void SeismicData2DPreStack::estimate2DAcquisitionGeometry(void)
{
    estimateSpacing(m_recx, m_recy, m_pcr, 100);
    estimateSpacing(m_srcx, m_srcy, m_pcs, 100);
    estimateSpacing(m_cdpx, m_cdpy, m_pcdp, m_pcr->maxGroupSpace);
    m_pco->aveGroupSpace = m_pcr->aveGroupSpace;
    m_pco->minGroupSpace = m_pcr->minGroupSpace;
    m_pco->maxGroupSpace = m_pcr->maxGroupSpace;

    cout << "spacing of receivers(min, max, ave): (" << m_pcr->minGroupSpace << ", " << m_pcr->maxGroupSpace << ", "<< m_pcr->aveGroupSpace<<")"<<endl;
    cout << "spacing of shots(min, max, ave): (" << m_pcs->minGroupSpace << ", " << m_pcs->maxGroupSpace << ", "<< m_pcs->aveGroupSpace<<")"<<endl;
    cout << "spacing of CDPs(min, max, ave): (" << m_pcdp->minGroupSpace << ", " << m_pcdp->maxGroupSpace << ", "<< m_pcdp->aveGroupSpace<<")"<<endl;
    cout << "First Shot at : (" << m_pcs->firstGroupXY.x() << ", " << m_pcs->firstGroupXY.y() <<")" <<endl;
    cout << "Last  Shot at : (" << m_pcs->lastGroupXY.x()  << ", " << m_pcs->lastGroupXY.y() <<")" <<endl;
    cout << "First Recv at : (" << m_pcr->firstGroupXY.x() << ", " << m_pcr->firstGroupXY.y() <<")" <<endl;
    cout << "Last  Recv at : (" << m_pcr->lastGroupXY.x()  << ", " << m_pcr->lastGroupXY.y() <<")" <<endl;
    cout << "First CDP at  : (" << m_pcdp->firstGroupXY.x() << ", " << m_pcdp->firstGroupXY.y() <<")" <<endl;
    cout << "Last  CDP at  : (" << m_pcdp->lastGroupXY.x()  << ", " << m_pcdp->lastGroupXY.y() <<")" <<endl;
    cout << "Negative m_offset range : (" << m_pco->firstGroupXY.x() << ", " << m_pco->firstGroupXY.y() <<")" <<endl;
    cout << "Positive m_offset range : (" << m_pco->lastGroupXY.x()  << ", " << m_pco->lastGroupXY.y() <<")" <<endl;

}

// knowOffInfo=true has not implemented yet. Now the offset information is recalculated using srcXY and recXY
// the offset values in the trace header will be used when set knowOffInfo=true.
void SeismicData2DPreStack::collectTraceHeaderInfo(bool knowOffInfo)
{
    //cout << " in SeismicData2DPreStack" << endl;
    TraceHeaders** tHeader = m_sgy->getAllTracesHeader();

    float xyscale = tHeader[0]->scalar_for_coordinates;
    if(xyscale < 0) xyscale = 1./fabs(xyscale);
    if(fabs(xyscale) < 0.0000001) xyscale=1;

    float elscale = tHeader[0]->scalar_for_elevations_and_depths;
    if(elscale < 0) elscale = 1./fabs(elscale);
    if(fabs(elscale) < 0.000001) elscale=1;
    float sx, sy, rx, ry, disx, disy;
    float minpoff = 999999;
    float maxpoff = 0;
    float minnoff = 0;
    float maxnoff = -999999;

    for(int i=0; i < m_ntr; i++ ){
        sx = tHeader[i]->x_source_coordinate * xyscale;
        sy = tHeader[i]->y_source_coordinate * xyscale;
        rx = tHeader[i]->x_receiver_group_coordinate * xyscale;
        ry = tHeader[i]->y_receiver_group_coordinate * xyscale;

        m_srcx[i] = sx;
        m_srcy[i] = sy;
        m_recx[i] = rx;
        m_recy[i] = ry;
        m_cdpx[i] = (sx+rx)/2.0;
        m_cdpy[i] = (sy+ry)/2.0;

        if(!knowOffInfo){
            disx = sx-rx;
            disy = sy-ry;
            m_offset[i] = sqrtf(disx*disx + disy*disy);
            if(rx  < sx ) {
                m_offset[i] *= -1.0;
                if(m_offset[i] < minnoff) minnoff = m_offset[i];
                if(m_offset[i] > maxnoff) maxnoff = m_offset[i];
            } else {
                if(m_offset[i] < minpoff) minpoff = m_offset[i];
                if(m_offset[i] > maxpoff) maxpoff = m_offset[i];
            }
        } else {
            m_offset[i] = tHeader[i]->distance_from_source_point_to_receiver_group;
            if(m_offset[i] < minnoff) minnoff = m_offset[i];
            if(m_offset[i] > maxnoff) maxnoff = m_offset[i];
        }

        m_sdepth[i] = tHeader[i]->source_depth_below_surface * elscale;
        m_selev[i]  = tHeader[i]->surface_elevation_at_source* elscale;
        m_relev[i]  = tHeader[i]->receiver_group_elevation * elscale;

        //m_srcid[i] = tHeader[i]->energy_source_point_number;
        m_cdpid[i] = tHeader[i]->cdp_ensemble_number;
        if(tHeader[i]->data_use > 0) m_trflag[i] = tHeader[i]->data_use;
        m_topMute[i] = tHeader[i]->mute_time_start;
        m_btmMute[i] = tHeader[i]->mute_time_end;

        //m_topMute[i] = 0;
        //cout <<"i="<<i<< ", sx="<<sx<<", sy="<<sy<<", rx="<<rx<<", ry="<<ry<<", m_cdpx="<<m_cdpx[i]<<", m_cdpy="<<m_cdpy[i]<<", m_offset="<<m_offset[i]<<endl;
    }

    m_pco->firstGroupXY = QPointF(m_minoff, m_maxoff); // user specified values
    m_pco->lastGroupXY  = QPointF(minpoff, maxpoff);   // from acquisition
}

void SeismicData2DPreStack::findLineEndPoints(void)
{
    if(m_pcs->firstGroupXY.x() < m_pcr->firstGroupXY.x()){
        m_firstXY = m_pcs->firstGroupXY;
    }else{
        m_firstXY = m_pcr->firstGroupXY;
    }

    if(m_pcs->lastGroupXY.x() > m_pcr->lastGroupXY.x()){
        m_lastXY = m_pcs->lastGroupXY;
    }else{
        m_lastXY = m_pcr->lastGroupXY;
    }

}

void SeismicData2DPreStack::buildCDPIndex()
{
    float x = m_pcdp->firstGroupXY.x();
    float y = m_pcdp->firstGroupXY.y();
    float x0 = x*cos(m_angle) + y*sin(m_angle);
    float rx;
    for(int i=0; i< m_ntr; i++){
        rx = m_cdpx[i]*cos(m_angle) + m_cdpy[i]*sin(m_angle);
        m_cdpid[i] = roundf((rx-x0)/m_dcdp)+1;
    }
    int mincdp = INT_MAX;
    int maxcdp = 0;
    for(int i=0; i< m_ntr; i++){
        if(m_trflag[i] == SeismicTraceFlag::DeadTrace || m_trflag[i] == SeismicTraceFlag::UnusedTrace) continue;
        if(m_cdpid[i] < mincdp) mincdp = m_cdpid[i];
        if(m_cdpid[i] > maxcdp) maxcdp = m_cdpid[i];
    }
    m_pcdp->minGroupValue = mincdp;
    m_pcdp->maxGroupValue = maxcdp;
    m_pcdp->ngroups = maxcdp-mincdp+1;
    cout << "Min CDPIdx="<<mincdp<<", Max CDPIdx="<<maxcdp<<", Number of traces="<< m_ntr << endl;

}

void SeismicData2DPreStack::buildOffsetIndex()
{
    int maxid = roundf((m_maxoff-m_minoff)/m_doff) + 1;
    cout << "maxid = " << maxid << endl;
    for(int i=0; i< m_ntr; i++){
        // recalculate the offsetID using user specified offset spaceing and min/max offset.
        // The loaded value from SEGY file is discarded.
        // offsetID start from 1
        m_offid[i] = roundf((m_offset[i] - m_minoff)/m_doff) + 1;
        if(m_offid[i] > maxid) m_offid[i] = -1;
        if(m_offid[i] < 0) m_trflag[i] = SeismicTraceFlag::UnusedTrace;
    }    

    int minoid = INT_MAX;
    int maxoid = 0;
    for(int i=0; i< m_ntr; i++){
        if(m_trflag[i] == SeismicTraceFlag::DeadTrace || m_trflag[i] == SeismicTraceFlag::UnusedTrace) continue;
        if(m_offid[i] < minoid) minoid = m_offid[i];
        if(m_offid[i] > maxoid) maxoid = m_offid[i];
    }
    m_pco->minGroupValue = minoid;
    m_pco->maxGroupValue = maxoid;
    m_pco->ngroups = maxoid-minoid+1;
    cout << "Build Offset Index: ngroups="<< m_pco->ngroups << " Min OffsetIdx=" << m_pco->minGroupValue << " Max OffsetIdx=" << m_pco->maxGroupValue << endl;
}

void SeismicData2DPreStack::saveIndexFile(void)
{
    int swapbytes = m_sgy->getTraceDataSwapbytes();

    string& segy_file = m_sgy->getSEGYFileName();
    unsigned int StringLength = segy_file.size();
    ofstream outfile(m_idxfile, ofstream::binary);

    outfile.write((char*)&m_datatype, sizeof(SeismicDataType));
    outfile.write((char*)&StringLength, sizeof(unsigned int));
    outfile.write(segy_file.c_str(), StringLength);

    outfile.write((char*)&swapbytes, sizeof(int));

    outfile.write((char*)&m_dcdp, sizeof(float));
    outfile.write((char*)&m_doff, sizeof(float));
    outfile.write((char*)&m_minoff, sizeof(float));
    outfile.write((char*)&m_maxoff, sizeof(float));
    outfile.write((char*)&m_firstXY, sizeof(QPointF));
    outfile.write((char*)&m_lastXY, sizeof(QPointF));
    outfile.write((char*)&m_slope, sizeof(float));
    outfile.write((char*)&m_intercept, sizeof(float));
    outfile.write((char*)&m_angle, sizeof(float));

    saveGathersIndex(outfile, m_pcs);
    saveGathersIndex(outfile, m_pcdp);
    saveGathersIndex(outfile, m_pco);
    saveGathersIndex(outfile, m_pcr);

    m_idxPos = outfile.tellp();
    //cout << "m_idxPos = " << m_idxPos << endl;

    for(int i=0; i< m_ntr; i++){
        TraceHeaders* tmp_thdr = m_sgy->getTraceHeader(i);
        outfile.write(reinterpret_cast<char*>(tmp_thdr), TRACE_HEADER_SIZE);
    }
    outfile.close();
    cout << "saveIndexFile done" << endl;
}

void SeismicData2DPreStack::loadIndexFile(void)
{
    if(!isIndexFileExist()) return;
    //cout << "###### loadIndexFile #### " << endl;
    unsigned int StringLength;
    string segyFN_from_index;
    ifstream infile(m_idxfile, ifstream::binary);
    infile.read((char*)&m_datatype, sizeof(SeismicDataType));
    infile.read((char*)&StringLength, sizeof(unsigned int));

    char* temp = new char[StringLength+1];
    infile.read(temp, StringLength);
    temp[StringLength] = '\0';
    segyFN_from_index = temp;
    delete [] temp;

    string segyFN_from_GUI = m_sgy->getSEGYFileName();

    cout << "SEGY file name in Index file: " << segyFN_from_index.c_str() << endl;
    cout << "SEGY file name from GUI: " << segyFN_from_GUI.c_str() << endl;

    //for(int i=0; i< m_ntr; i++) m_trflag[i] = 1;

    int swapbytes;
    infile.read((char*)&swapbytes, sizeof(float));
    m_sgy->setTraceDataSwapbytes(swapbytes);

    infile.read((char*)&m_dcdp, sizeof(float));
    infile.read((char*)&m_doff, sizeof(float));
    infile.read((char*)&m_minoff, sizeof(float));
    infile.read((char*)&m_maxoff, sizeof(float));
    infile.read((char*)&m_firstXY, sizeof(QPointF));
    infile.read((char*)&m_lastXY, sizeof(QPointF));
    infile.read((char*)&m_slope, sizeof(float));
    infile.read((char*)&m_intercept, sizeof(float));
    infile.read((char*)&m_angle, sizeof(float));

    //cout << "angle = "<< m_angle << endl;

    loadGathersIndex(infile, m_pcs);
    loadGathersIndex(infile, m_pcdp);
    loadGathersIndex(infile, m_pco);
    loadGathersIndex(infile, m_pcr);
    m_idxPos = infile.tellg();
    for(int i=0; i< m_ntr; i++){
        TraceHeaders* tmp_thdr = new TraceHeaders;
        infile.read(reinterpret_cast<char*>(tmp_thdr), TRACE_HEADER_SIZE);
        m_sgy->keepSEGYTraceHeader(i, tmp_thdr);
    }

    infile.close();
    SeismicData2DPreStack::createDataSummaryInfo(true);
}

bool SeismicData2DPreStack::isIdxFileHasNMOVelocities()
{
    if(!isIndexFileExist()) return false;
    QFile idxf(QString::fromUtf8(m_idxfile.data(), m_idxfile.size()));
    qint64 fsize = idxf.size();
    qint64 velIdx = m_idxPos + m_ntr*240;
    cout << "fsize=" << fsize << " m_idxPos=" << m_idxPos << " velIdx=" << velIdx << endl;
    if(velIdx < fsize) return true;
    return false;
}

void SeismicData2DPreStack::saveGathersIndex(ofstream& outfile, gathers *p)
{
    outfile.write((char*)&(p->gather_type), sizeof(int));
    outfile.write((char*)&(p->ngroups), sizeof(int));
    outfile.write((char*)&(p->idxType), sizeof(int));
    outfile.write((char*)&(p->minGroupValue), sizeof(int));
    outfile.write((char*)&(p->maxGroupValue), sizeof(int));
    outfile.write((char*)&(p->minGroupSpace), sizeof(float));
    outfile.write((char*)&(p->maxGroupSpace), sizeof(float));
    outfile.write((char*)&(p->aveGroupSpace), sizeof(float));
    outfile.write((char*)&(p->useGroupSpace), sizeof(float));
    outfile.write((char*)&(p->firstGroupXY), sizeof(QPointF));
    outfile.write((char*)&(p->lastGroupXY), sizeof(QPointF));

    //cout << "ngroups = " << p->ngroups << endl;

    for(int ig = 0; ig < p->ngroups; ig++){        
        outfile.write((char*)&(p->group[ig].ntraces), sizeof(int));
        outfile.write((char*)&(p->group[ig].groupValue1), sizeof(float));
        outfile.write((char*)&(p->group[ig].groupValue2), sizeof(float));
        outfile.write((char*)&(p->group[ig].groupValue3), sizeof(float));
        outfile.write((char*)&(p->group[ig].groupValue4), sizeof(float));
        //cout << p->gather_type<< " groupx "<< p->group[ig].groupValue1 << " groupy "<< p->group[ig].groupValue2 <<" has traces: " << p->group[ig].ntraces << endl;
        int ntraces= p->group[ig].traceIdx.size();
        outfile.write((char*)p->group[ig].traceIdx.data(), sizeof(float)*ntraces);
    }
}

void SeismicData2DPreStack::loadGathersIndex(ifstream& infile, gathers *p)
{
    int trIdx = 0;

    infile.read((char*)&(p->gather_type), sizeof(int));
    infile.read((char*)&(p->ngroups), sizeof(int));
    infile.read((char*)&(p->idxType), sizeof(int));
    infile.read((char*)&(p->minGroupValue), sizeof(int));
    infile.read((char*)&(p->maxGroupValue), sizeof(int));
    infile.read((char*)&(p->minGroupSpace), sizeof(float));
    infile.read((char*)&(p->maxGroupSpace), sizeof(float));
    infile.read((char*)&(p->aveGroupSpace), sizeof(float));
    infile.read((char*)&(p->useGroupSpace), sizeof(float));
    infile.read((char*)&(p->firstGroupXY), sizeof(QPointF));
    infile.read((char*)&(p->lastGroupXY), sizeof(QPointF));

    //cout << "ngroups = " << p->ngroups << endl;

    p->group.resize(p->ngroups);
    int gtype = p->gather_type;
    int mingv = p->minGroupValue;

    for(int ig = 0; ig < p->ngroups; ig++){

        infile.read((char*)&(p->group[ig].ntraces), sizeof(int));
        infile.read((char*)&(p->group[ig].groupValue1), sizeof(float));
        infile.read((char*)&(p->group[ig].groupValue2), sizeof(float));
        infile.read((char*)&(p->group[ig].groupValue3), sizeof(float));
        infile.read((char*)&(p->group[ig].groupValue4), sizeof(float));

        //cout << p->gather_type<< " group "<< p->group[ig].groupValue1 << " has traces: " << p->group[ig].ntraces << endl;

        for(int is = 0; is < p->group[ig].ntraces; is++){
            infile.read((char*)&(trIdx), sizeof(int));
            p->group[ig].traceIdx.push_back(trIdx);
            switch(gtype){
                case 1:
                    m_srcid[trIdx] = ig + mingv;
                    break;
                case 2:
                    m_cdpid[trIdx] = ig + mingv;
                    break;
                case 3:
                    m_offid[trIdx] = ig + mingv;
                    break;
                case 4:
                    m_recid[trIdx] = ig + mingv;
                    break;
            }
            //cout << "     is="<<is<<"    subset: "<< p->group[ig].subset[is].subsetValue << ", traceIdx="<<p->group[ig].subset[is].traceIdx<<endl;
        }
    }

}

void SeismicData2DPreStack::saveTraceHeaderToIndexFile(int trIdx)
{
    QString idxFileName = QString::fromStdString(m_idxfile);
    QFile* idxFile = new QFile(idxFileName);
    idxFile->open(QIODevice::ReadWrite | QIODevice::ExistingOnly);
    qint64 pos = m_idxPos + trIdx*240;
    //cout << "m_idxPos = " << m_idxPos << " pos=" << pos << endl;
    TraceHeaders** tHeader = m_sgy->getAllTracesHeader();
    idxFile->seek(pos);
    idxFile->write((char*)tHeader[trIdx], 240);
    idxFile->close();
    //cout << "m_idxPos = " << m_idxPos << " size=" << idxFile->size() << endl;
}

gathers* SeismicData2DPreStack::getShotGatherIndex(void)
{
    return m_pcs;
}

gathers* SeismicData2DPreStack::getReceiverGatherIndex(void)
{
    return m_pcr;
}

gathers* SeismicData2DPreStack::getOffsetGatherIndex(void)
{
    return m_pco;
}

int SeismicData2DPreStack::getFoldOfCDPs(QList<QPointF>& cdps)
{
    int maxFold=0;
    for(int i=m_pcdp->minGroupValue; i<=m_pcdp->maxGroupValue; i++){
        int cdpidx = i - m_pcdp->minGroupValue;
        float fold = m_pcdp->group[cdpidx].ntraces;
        float x = m_pcdp->group[cdpidx].groupValue1;
        cdps.append(QPointF(x, fold));
        if(fold > maxFold) maxFold=fold;
    }
    return maxFold;
}

int SeismicData2DPreStack::getFoldOfOneCDP(int cdp)
{
    if(cdp < m_pcdp->minGroupValue || cdp > m_pcdp->maxGroupValue) return 0;
    int cdpidx = cdp - m_pcdp->minGroupValue;
    return m_pcdp->group[cdpidx].ntraces;
}

int SeismicData2DPreStack::getNumberOfTracesInOneGather(int seq, gathers *p)
{
    if(seq < p->minGroupValue || seq > p->maxGroupValue) return 0;
    int idx = seq - p->minGroupValue;
    return p->group[idx].ntraces;
}

int SeismicData2DPreStack::getNumberOfCDPs(void)
{
    return m_pcdp->ngroups;
}

int SeismicData2DPreStack::getNumberOfShots(void)
{
    return m_pcs->ngroups;
}

int SeismicData2DPreStack::getNumberOfReceivers(void)
{
    return m_pcr->ngroups;
}

int SeismicData2DPreStack::getNumberOfOffsets(void)
{
    return m_pco->ngroups;
}

float** SeismicData2DPreStack::getSeismicDataOfGather(int gatherType, int gIdx, int& ntr)
{
    gathers* p = getGatherIndexPointer(gatherType);

    ntr = getNumberOfTracesInOneGather(gIdx, p);
    if(ntr <= 0) return nullptr;
    int ns = m_ns;
    float** data = Sdp2dUtils::alloc2float(ns, ntr);
    int idx = gIdx - p->minGroupValue;

    for(int i=0; i< ntr; i++){
        int traceIdx = p->group[idx].traceIdx[i];
         m_sgy->getTraceDataFromIntermFile(traceIdx, data[i]);
    }

    return data;
}

void SeismicData2DPreStack::getOffsetAndElevOfGather(int gatherType, int gIdx, int ntr, double* offset, double* elev)
{
    gathers* p = getGatherIndexPointer(gatherType);

    int idx = gIdx - p->minGroupValue;
    for(int i=0; i< ntr; i++){
        int tIdx = p->group[idx].traceIdx[i];
        offset[i] = m_offset[tIdx];

        if(gatherType == DisplayGatherType::CommonReceiver){
            elev[i] = m_selev[tIdx];
        } else {
            elev[i] = m_relev[tIdx];
        }
    }
}

void SeismicData2DPreStack::getOffsetValuesOfGather(int gatherType, int gIdx, int ntr, float* offset)
{
    gathers* p = getGatherIndexPointer(gatherType);

    int idx = gIdx - p->minGroupValue;
    for(int i=0; i< ntr; i++){
        int tIdx = p->group[idx].traceIdx[i];
        offset[i] = m_offset[tIdx];
    }
}

void SeismicData2DPreStack::outputSegyWithLocalFormat()
{
    cout << "In SeismicData2DPreStack::outputSegyWithLocalFormat" << endl;
    ofstream outfile(m_outSegyName.toStdString(), ofstream::binary);

    EbcdicHeader* ehdr = m_sgy->get3200TextHeader();
    BinaryHeader* bhdr = m_sgy->get400BinaryHeader();

    EbcdicHeader* tmp_ehdr = new EbcdicHeader;
    memcpy((char*)tmp_ehdr, (char*)ehdr, TEXT_HEADER_SIZE);
    Sdp2dUtils::ascii2Ebcdic(tmp_ehdr);

    BinaryHeader* tmp_bhdr = new BinaryHeader;
    memcpy((char*)tmp_bhdr, (char*)bhdr, BINARY_HEADER_SIZE);

    tmp_bhdr->samples_per_trace = m_ns;
    tmp_bhdr->data_sample_format_code = 5;

    outfile.write((char*)tmp_ehdr, TEXT_HEADER_SIZE);
    outfile.write((char*)tmp_bhdr, BINARY_HEADER_SIZE);

    QString type;

    switch(m_outputOrder+1){
        case 1:
            type = "Common Shot";
            break;
        case 2:
            type = "CDP";
            break;
        case 3:
            type = "Common Offset";
            break;
        case 4:
            type = "Common Receiver";
            break;
    }

    //cout << "m_outputOrder="<< m_outputOrder << " type=" << type.toStdString().c_str() << endl;
    gathers* p = getGatherIndexPointer(m_outputOrder+1);

    QProgressDialog dialog;
    dialog.setLabelText(QString("Write "+type+" Gathers to segy file..."));
    dialog.setMaximum(p->ngroups-1);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAutoClose(true);

    float* data = new float [m_ns];
    size_t dlen = m_ns * sizeof(float);

    short* topMute = m_tMute->getFilledMutes();
    short* btmMute = m_bMute->getFilledMutes();
    float* weight = nullptr;

    if(m_outputApplyTopMute || m_outputApplyBtmMute){
        weight = new float [m_ns];
        for(int i=0; i< m_ns; i++) weight[i] = 1.;
    }

    int topTaperLength = m_tMute->getMuteTaperLength();
    int btmTaperLength = m_bMute->getMuteTaperLength();

    for(int ig = 0; ig < p->ngroups; ig++){
        //cout << "group "<< ig <<" is writing to disk"<< endl;
        dialog.setValue(ig);
        if (dialog.wasCanceled()) break;

        int ntraces= p->group[ig].traceIdx.size();

        for(int itr=0; itr < ntraces; itr++){
            int tid = p->group[ig].traceIdx[itr];
            if(m_discardBadTraces && m_trflag[tid] == SeismicTraceFlag::PickedBadTrace) continue;

            TraceHeaders* thdr = m_sgy->getTraceHeader(tid);
            m_sgy->getTraceDataFromIntermFile(tid, data);

            if(m_outputApplyTopMute || m_outputApplyBtmMute){
                int top = 0;
                if(m_outputApplyTopMute) top = topMute[tid];

                int btm = m_ns;
                if(m_outputApplyBtmMute) btm = btmMute[tid];
                if(btm < top) btm = top;
                //if(ig==0) cout << "ig="<< ig << " it="<< i << " top="<< top << " btm=" << btm << endl;
                calculateTraceWeightValues(top, btm, weight, topTaperLength, btmTaperLength);
                for(int i=0; i< m_ns; i++) data[i] *= weight[i];
            }

            outfile.write((char*)thdr, TRACE_HEADER_SIZE);
            outfile.write((char*)data, dlen);
            //cout << " tid:" << tid << ", group=" << p->group[ig].groupValue1  << " ig=" << ig << endl;
            //cout << "shot station: "<< thdr->energy_source_point_number << ", receiver station: "<< thdr->cdp_ensemble_number << endl;
        }
    }

    outfile.close();

    delete tmp_bhdr;    
    delete tmp_ehdr;    
    delete [] data;    


    if(weight != nullptr)    delete [] weight;

    m_sbar->showMessage(QString("Output to file %1 is done!").arg(m_outSegyName));

}


void SeismicData2DPreStack::processAndOutputSegyWithLocalFormat(QString moduleName)
{
    cout << "In SeismicData2DPreStack::processAndOutputSegyWithLocalFormat" << endl;
    ofstream outfile(m_outSegyName.toStdString(), ofstream::binary);

    EbcdicHeader* ehdr = m_sgy->get3200TextHeader();
    BinaryHeader* bhdr = m_sgy->get400BinaryHeader();

    EbcdicHeader* tmp_ehdr = new EbcdicHeader;
    memcpy((char*)tmp_ehdr, (char*)ehdr, TEXT_HEADER_SIZE);
    Sdp2dUtils::ascii2Ebcdic(tmp_ehdr);

    BinaryHeader* tmp_bhdr = new BinaryHeader;
    memcpy((char*)tmp_bhdr, (char*)bhdr, BINARY_HEADER_SIZE);

    tmp_bhdr->samples_per_trace = m_ns;
    tmp_bhdr->data_sample_format_code = 5;

    outfile.write((char*)tmp_ehdr, TEXT_HEADER_SIZE);
    outfile.write((char*)tmp_bhdr, BINARY_HEADER_SIZE);

    QString type;

    switch(m_outputOrder+1){
        case 1:
            type = "Common Shot";
            break;
        case 2:
            type = "CDP";
            break;
        case 3:
            type = "Common Offset";
            break;
        case 4:
            type = "Common Receiver";
            break;
    }

    //cout << "m_outputOrder="<< m_outputOrder << " type=" << type.toStdString().c_str() << endl;
    gathers* p = getGatherIndexPointer(m_outputOrder+1);

    QProgressDialog dialog;
    dialog.setLabelText(QString("Write "+type+" Gathers to segy file..."));
    dialog.setMaximum(p->ngroups-1);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAutoClose(true);

    size_t dlen = m_ns * sizeof(float);

    short* topMute = m_tMute->getFilledMutes();
    short* btmMute = m_bMute->getFilledMutes();
    float* weight = nullptr;

    if(m_outputApplyTopMute || m_outputApplyBtmMute){
        weight = new float [m_ns];
        for(int i=0; i< m_ns; i++) weight[i] = 1.;
    }

    int topTaperLength = m_tMute->getMuteTaperLength();
    int btmTaperLength = m_bMute->getMuteTaperLength();

    for(int ig = 0; ig < p->ngroups; ig++){
        //cout << "group "<< ig <<" is writing to disk"<< endl;
        dialog.setValue(ig);
        if (dialog.wasCanceled()) break;

        int ntraces= p->group[ig].traceIdx.size();
        float** oudata = Sdp2dUtils::alloc2float(m_ns, ntraces);
        float** indata = Sdp2dUtils::alloc2float(m_ns, ntraces);
        memset((void*)indata[0], 0, ntraces*dlen);

        for(int itr=0; itr < ntraces; itr++){
            int tid = p->group[ig].traceIdx[itr];
            if(m_discardBadTraces && m_trflag[tid] == SeismicTraceFlag::PickedBadTrace) continue;

            m_sgy->getTraceDataFromIntermFile(tid, indata[itr]);

            if(m_outputApplyTopMute || m_outputApplyBtmMute){
                int top = 0;
                if(m_outputApplyTopMute) top = topMute[tid];

                int btm = m_ns;
                if(m_outputApplyBtmMute) btm = btmMute[tid];
                if(btm < top) btm = top;

                calculateTraceWeightValues(top, btm, weight, topTaperLength, btmTaperLength);
                for(int i=0; i< m_ns; i++) indata[itr][i] *= weight[i];
            }
        }

        m_mainWindow->processCurrentGather(moduleName, indata, oudata, ntraces);

        for(int itr=0; itr < ntraces; itr++){
            int tid = p->group[ig].traceIdx[itr];
            if(m_discardBadTraces && m_trflag[tid] == SeismicTraceFlag::PickedBadTrace) continue;
            TraceHeaders* thdr = m_sgy->getTraceHeader(tid);
            outfile.write((char*)thdr, TRACE_HEADER_SIZE);
            outfile.write((char*)oudata[itr], dlen);
        }

        Sdp2dUtils::free2float(indata);
        Sdp2dUtils::free2float(oudata);
    }

    outfile.close();

    delete tmp_bhdr;
    delete tmp_ehdr;


    if(weight != nullptr)    delete [] weight;

    m_sbar->showMessage(QString("Output to file %1 is done!").arg(m_outSegyName));

}

void SeismicData2DPreStack::outputSegyWithLocalFormat(const string& segy_fname, gathers *p)
{
    ofstream outfile(segy_fname, ofstream::binary);

    EbcdicHeader* ehdr = m_sgy->get3200TextHeader();
    BinaryHeader* bhdr = m_sgy->get400BinaryHeader();

    EbcdicHeader* tmp_ehdr = new EbcdicHeader;
    memcpy((char*)tmp_ehdr, (char*)ehdr, TEXT_HEADER_SIZE);
    Sdp2dUtils::ascii2Ebcdic(tmp_ehdr);

    BinaryHeader* tmp_bhdr = new BinaryHeader;
    memcpy((char*)tmp_bhdr, (char*)bhdr, BINARY_HEADER_SIZE);

    tmp_bhdr->samples_per_trace = m_ns;
    tmp_bhdr->data_sample_format_code = 5;

    outfile.write((char*)tmp_ehdr, TEXT_HEADER_SIZE);
    outfile.write((char*)tmp_bhdr, BINARY_HEADER_SIZE);

    QString type;
    switch(p->gather_type){
        case 1:
            type = "Common Shot";
            break;
        case 2:
            type = "CDP";
            break;
        case 3:
            type = "Common Offset";
            break;
        case 4:
            type = "Common Receiver";
            break;
    }

    QProgressDialog dialog;
    dialog.setLabelText(QString("Write "+type+" Gathers to segy file..."));
    dialog.setMaximum(p->ngroups-1);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAutoClose(true);

    for(int ig = 0; ig < p->ngroups; ig++){
        //cout << "group "<< ig <<" is writing to disk"<< endl;
        dialog.setValue(ig);
        if (dialog.wasCanceled()) break;
        writeOneGroup2Disk(outfile,  p, ig);
    }

    outfile.close();
    delete tmp_bhdr;
    delete tmp_ehdr;
}

void SeismicData2DPreStack::outputSegyWithLocalFormatInShotOrder(const string& segy_fname)
{
    outputSegyWithLocalFormat(segy_fname, m_pcs);
}

void SeismicData2DPreStack::outputSegyWithLocalFormatInCDPOrder(const string& segy_fname)
{
    outputSegyWithLocalFormat(segy_fname, m_pcdp);
}

void SeismicData2DPreStack::outputSegyWithLocalFormatInOffsetOrder(const string& segy_fname)
{
    outputSegyWithLocalFormat(segy_fname, m_pco);
}

void SeismicData2DPreStack::outputSegyWithLocalFormatInRecvOrder(const string& segy_fname)
{
    outputSegyWithLocalFormat(segy_fname, m_pcr);
}

void SeismicData2DPreStack::createDataSummaryInfo(bool moreflag)
{
    m_datasum.clear();
    //cout << " in SeismicData2DPreStack::createDataSummaryInfo" << endl;
    std::string& tmp = getSEGYFileName();
    //cout << "DataSummary fileName: " << tmp.c_str() << endl;
    const QString namewithpath = QString::fromUtf8( tmp.data(), tmp.size() );
    QString dataType;
    switch(m_datatype){
    case SeismicDataType::Stack:
        dataType = "Stack";
        break;
    case SeismicDataType::PreStack:
        dataType = "PreStack";
        break;
    case SeismicDataType::Attribure:
        dataType = "Attribure";
        break;
    }
    m_datasum << QString("File name: ") + namewithpath;
    m_datasum << QString("Data type: ") + dataType;
    m_datasum << QString("Number of traces: %1").arg(m_ntr);
    m_datasum << QString("Number of samples per trace: %1").arg(m_ns);
    m_datasum << QString("Sample rate in Micrometer: %1").arg(m_dtus);
    m_datasum << QString("Number of shots: %1").arg(m_pcs->ngroups);
    m_datasum << QString("Number of receivers: %1").arg(m_pcr->ngroups);
    m_datasum << QString("Number of CDPs: %1").arg(m_pcdp->ngroups);
    m_datasum << QString("Number of offsets: %1").arg(m_pco->ngroups);

    m_datasum << QString("Spacing of receivers(min, max, ave): (%1, %2, %3)").arg(m_pcr->minGroupSpace).arg(m_pcr->maxGroupSpace).arg(m_pcr->aveGroupSpace);
    m_datasum << QString("Spacing of shots    (min, max, ave): (%1, %2, %3)").arg(m_pcs->minGroupSpace).arg(m_pcs->maxGroupSpace).arg(m_pcs->aveGroupSpace);
    m_datasum << QString("Spacing of CDPs     (min, max, ave): (%1, %2, %3)").arg(m_pcdp->minGroupSpace).arg(m_pcdp->maxGroupSpace).arg(m_pcdp->aveGroupSpace);
    m_datasum << QString("First Shot at : (%1, %2)").arg(m_pcs->firstGroupXY.x()).arg(m_pcs->firstGroupXY.y());
    m_datasum << QString("Last  Shot at : (%1, %2)").arg(m_pcs->lastGroupXY.x()).arg(m_pcs->lastGroupXY.y());
    m_datasum << QString("Last  Recv at : (%1, %2)").arg(m_pcr->firstGroupXY.x()).arg(m_pcr->firstGroupXY.y());
    m_datasum << QString("Last  Recv at : (%1, %2)").arg(m_pcr->lastGroupXY.x()).arg(m_pcr->lastGroupXY.y());
    m_datasum << QString("Last  CDP  at : (%1, %2)").arg(m_pcdp->firstGroupXY.x()).arg(m_pcdp->firstGroupXY.y());
    m_datasum << QString("Last  CDP  at : (%1, %2)").arg(m_pcdp->lastGroupXY.x()).arg(m_pcdp->lastGroupXY.y());

    m_datasum << QString("Negative offset range : (%1 ~%2)").arg(m_pco->firstGroupXY.x()).arg(m_pco->firstGroupXY.y());
    m_datasum << QString("Positive offset range : (%1 ~%2)").arg(m_pco->lastGroupXY.x()).arg(m_pco->lastGroupXY.y());

    if(!moreflag) return;

    m_datasum << QString("Assigned CDP spacing : %1").arg(m_dcdp);
    m_datasum << QString("Assigned Offset spacing: %1").arg(m_doff);
    m_datasum << QString("Assigned minimum offset: %1").arg(m_minoff);
    m_datasum << QString("Assigned maximum offset: %1").arg(m_maxoff);
    m_datasum << QString("Estimated first XY of the 2D line : (%1, %2)").arg(m_firstXY.x()).arg(m_firstXY.y());
    m_datasum << QString("Estimated Last  XY of the 2D line : (%1, %2)").arg(m_lastXY.x()).arg(m_lastXY.y());
    m_datasum << QString("Estimated azimuth angle of the 2D line: %1").arg(m_angle);

}

void SeismicData2DPreStack::getAllShotXY(QList<QPointF>& data)
{
    cout << "Number of shot gathers: " << m_pcs->ngroups << endl;
    for(int ig = 0; ig < m_pcs->ngroups; ig++){
        const QPointF value(m_pcs->group[ig].groupValue1, m_pcs->group[ig].groupValue2);
        data.append(value);
    }
}

void SeismicData2DPreStack::getAveCDPXY(QList<QPointF>& data)
{
    cout << "Number of cdp gathers: " << m_pcdp->ngroups << endl;
    for(int ig = 0; ig < m_pcdp->ngroups; ig++){
        const QPointF value(m_pcdp->group[ig].groupValue1, m_pcdp->group[ig].groupValue2);
        data.append(value);
    }
}

void SeismicData2DPreStack::getAllReceiverXY(QList<QPointF>& data)
{
    cout << "Number of receiver gathers: " << m_pcr->ngroups << endl;
    for(int ig = 0; ig < m_pcr->ngroups; ig++){
        const QPointF value(m_pcr->group[ig].groupValue1, m_pcr->group[ig].groupValue2);
        data.append(value);
    }
}

int SeismicData2DPreStack::getSubsetXYUsingGroupXY(const QPointF& groupXY, QList<QPointF>& subsetXY, int gatherType)
{
    subsetXY.clear();

    if(gatherType == DisplayGatherType::CommonOffset) return 0;

    gathers* p = getGatherIndexPointer(gatherType);

    for(int ig = 0; ig < p->ngroups; ig++){
        const QPointF value(p->group[ig].groupValue1, p->group[ig].groupValue2);
        if(groupXY == value){
            //cout << " find the group at index " << ig+p->minGroupValue << " has traces " << p->group[ig].ntraces << endl;

            for(int i=0; i< p->group[ig].ntraces; i++){
                int traceidx = p->group[ig].traceIdx[i];
                if(gatherType == DisplayGatherType::CommonReceiver){
                    subsetXY.append(QPointF(m_srcx[traceidx], m_srcy[traceidx]));
                } else {
                    subsetXY.append(QPointF(m_recx[traceidx], m_recy[traceidx]));
                }
            }            
            return ig+p->minGroupValue;
        }
    }
    return 0;
}

QPointF SeismicData2DPreStack::getSubsetXYUsingGroupIdx(int groupIdx, QList<QPointF>& subsetXY, int gatherType)
{
    subsetXY.clear();

    if(gatherType == DisplayGatherType::CommonOffset) return QPointF(0,0);

    gathers* p = getGatherIndexPointer(gatherType);

    int ig = groupIdx - p->minGroupValue;
    QPointF value(p->group[ig].groupValue1, p->group[ig].groupValue2);
    for(int i=0; i< p->group[ig].ntraces; i++){
        int traceidx = p->group[ig].traceIdx[i];
        if(gatherType == DisplayGatherType::CommonReceiver){
            subsetXY.append(QPointF(m_srcx[traceidx], m_srcy[traceidx]));
        } else {
            subsetXY.append(QPointF(m_recx[traceidx], m_recy[traceidx]));
        }
    }
    return value;
}

int SeismicData2DPreStack::getNumberOfTracesOfGather(int gType, int gIdx)
{
    gathers* p = getGatherIndexPointer(gType);
    int ntrs = getNumberOfTracesInOneGather(gIdx, p);
    return ntrs;
}

int SeismicData2DPreStack::getTracesIndexInData(int gType, int gIdx, int tIdx)
{
    gathers* p = getGatherIndexPointer(gType);

    int ig = gIdx - p->minGroupValue;

    if(tIdx <1 || tIdx >p->group[ig].ntraces){
        return 0;
    }else{
        return p->group[ig].traceIdx[tIdx-1];
    }
}

void SeismicData2DPreStack::getOneTraceWithTypeAndIndex(int gType, int gIdx, int tIdx, float *trace)
{
    gathers* p = getGatherIndexPointer(gType);

    int ig = gIdx - p->minGroupValue;
    int traceIdx = p->group[ig].traceIdx[tIdx-1];
    m_sgy->getTraceDataFromIntermFile(traceIdx, trace);
}

int SeismicData2DPreStack::getTraceIndexWithinWholeDataset(int gType, int gIdx, int tIdx)
{
    gathers* p = getGatherIndexPointer(gType);

    int ig = gIdx - p->minGroupValue;
    int traceIdx = p->group[ig].traceIdx[tIdx-1];
    return traceIdx;
}

void SeismicData2DPreStack::getTraceFlags(int gatherType, int gatherIdx, bool* trFlags)
{
    if(trFlags == nullptr) return;
    gathers* p = getGatherIndexPointer(gatherType);

    int ig = gatherIdx - p->minGroupValue;

    int traceIdx = 0;
    for(int i=0; i< p->group[ig].ntraces; i++){
        traceIdx = p->group[ig].traceIdx[i];
        if(m_trflag[traceIdx] == SeismicTraceFlag::PickedBadTrace){
            trFlags[i] = true;
        } else {
            trFlags[i] = false;
        }
        //if(!trFlags[i]){
        //    cout << "trace = " << i << " Index=" << traceIdx << endl;
        //}
    }
}

void SeismicData2DPreStack::setTraceFlags(int gatherType, int gatherIdx, bool* trFlags)
{
    if(trFlags == nullptr) return;
    gathers* p = getGatherIndexPointer(gatherType);
    TraceHeaders** tHeader = m_sgy->getAllTracesHeader();

    int ig = gatherIdx - p->minGroupValue;

    int traceIdx = 0;
    for(int i=0; i< p->group[ig].ntraces; i++){
        traceIdx = p->group[ig].traceIdx[i];
        if(trFlags[i]) {
            m_trflag[traceIdx]=SeismicTraceFlag::PickedBadTrace;
        }else {
            if(m_trflag[traceIdx]==SeismicTraceFlag::PickedBadTrace)
                m_trflag[traceIdx]=SeismicTraceFlag::LiveTrace;
        }
        tHeader[traceIdx]->data_use = m_trflag[traceIdx];
        saveTraceHeaderToIndexFile(traceIdx);
    }
    //cout << "in setTraceFlags traceIdx=" << traceIdx << endl;

}

void SeismicData2DPreStack::getTracesMute(int gatherType, int gatherIdx, short* muteValues, int forceFlag)
{
    if(muteValues == nullptr ) return;

    gathers* p = getGatherIndexPointer(gatherType);
    int ig = gatherIdx - p->minGroupValue;
    int ntr = p->group[ig].ntraces;
    int traceIdx = 0;

    if(forceFlag == 2 || ( forceFlag == 0 && m_muteType == MuteType::BottomMute )) {
        short* mutes = m_bMute->getFilledMutes();
        for(int i=0; i< ntr; i++){
            traceIdx = p->group[ig].traceIdx[i];
            muteValues[i] = mutes[traceIdx];
        }
    } else {
        short* mutes = m_tMute->getFilledMutes();
        for(int i=0; i< ntr; i++){
            traceIdx = p->group[ig].traceIdx[i];
            muteValues[i] = mutes[traceIdx];
        }
    }
}

/*
int SeismicData2DPreStack::getTracesMute(int gatherType, int gatherIdx, short* muteValues, int forceFlag)
{
    if(muteValues == nullptr ) return 0;

    gathers* p = getGatherIndexPointer(gatherType);
    int ig = gatherIdx - p->minGroupValue;
    int ntr = p->group[ig].ntraces;
    float* offset = new float [ntr];
    int traceIdx = 0;
    for(int i=0; i< ntr; i++){
        traceIdx = p->group[ig].traceIdx[i];
        offset[i] = m_offset[traceIdx];
    }

    QList<IdxVal> left;
    QList<IdxVal> middle;
    QList<IdxVal> right;

    if(forceFlag == 2 || ( forceFlag == 0 && m_muteType == MuteType::BottomMute )) {
        m_bMute->getMuteValuesOfGather(gatherType, gatherIdx, left, middle, right);
    } else {        
        m_tMute->getMuteValuesOfGather(gatherType, gatherIdx, left, middle, right);
    }

    QMap<int, float> picks;
    for(int i=0; i< left.size(); i++) picks.insert(left.at(i).trIdx, left.at(i).time);
    for(int i=0; i< middle.size(); i++) picks.insert(middle.at(i).trIdx, middle.at(i).time);
    for(int i=0; i< right.size(); i++) picks.insert(right.at(i).trIdx, right.at(i).time);

    QList<int> tids = picks.keys();
    //cout << "tids.count = " << tids.count() << endl;;

    if(tids.count() >= 2)  {
        std::sort(tids.begin(), tids.end());

        for(int i=0; i<tids.count(); i++){
            muteValues[tids.at(i)] = int(picks[tids.at(i)]*1000000./m_dtus);
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
                    muteValues[j] = int(((t1 - t2)/(off1 - off2)*(off - off2) + t2)*1000000/m_dtus);
                } else {
                    muteValues[j] = int(t2*1000000/m_dtus);
                }
                //cout << "j="<< j << " off=" << off << " i1=" << i1 << " i2=" << i2 << " off1=" << off1<< " off2=" << off2  << " t1=" << t1 << " t2=" << t2 << endl;
            }
        }
    }
    delete [] offset;

    int count = 0;
    for(int i=0; i< ntr; i++){
        if(muteValues[i] > 0) {
            count++;
        } else {
            if(forceFlag == 2 || ( forceFlag == 0 && m_muteType == MuteType::BottomMute)){
                muteValues[i] = m_ns;
            } else {
                muteValues[i] = 0;
            }
        }
        //cout << "i=" << i << "   mute = " << muteValues[i] << endl;
    }
    return count;
}
*/
void SeismicData2DPreStack::setTracesMute(int gatherType, int gatherIdx, short* topMute, short* btmMute)
{
    if(topMute == nullptr || btmMute == nullptr) return;
    gathers* p = getGatherIndexPointer(gatherType);
    TraceHeaders** tHeader = m_sgy->getAllTracesHeader();

    int ig = gatherIdx - p->minGroupValue;

    int traceIdx = 0;
    for(int i=0; i< p->group[ig].ntraces; i++){
        traceIdx = p->group[ig].traceIdx[i];
        m_topMute[traceIdx] = topMute[i];
        m_btmMute[traceIdx] = btmMute[i];
        tHeader[traceIdx]->mute_time_start = topMute[i];
        tHeader[traceIdx]->mute_time_end = btmMute[i];
    }
    //cout << "in setTracesMute1 traceIdx=" << traceIdx << endl;
    saveTraceHeaderToIndexFile(traceIdx);
}

void SeismicData2DPreStack::setTraceMute(int traceIdx, short value, int writeHeader)
{
    TraceHeaders** tHeader = m_sgy->getAllTracesHeader();

    if(m_muteType == MuteType::TopMute){
        tHeader[traceIdx]->mute_time_start = value;        
    } else {
        tHeader[traceIdx]->mute_time_end = value;
    }

    if(writeHeader>1) m_currentMute->calMuteForAllTraces(m_pcs);
    //cout << "in setTracesMute traceIdx=" << traceIdx << " value=" << value << " type=" << m_muteType << " writeHeader=" << writeHeader << endl;
    if(writeHeader) saveTraceHeaderToIndexFile(traceIdx);
}

void SeismicData2DPreStack::calMuteForAllTraces(short* allMutes, int muteType)
{
    if(allMutes == nullptr ) return;

    memset((void*)allMutes,  0, m_ntr*sizeof(short));

    int traceIdx = 0;
    QList<IdxVal> left;
    QList<IdxVal> middle;
    QList<IdxVal> right;

    for(int ig = 0; ig < m_pcs->ngroups; ig++){
        int ntr= m_pcs->group[ig].traceIdx.size();

        int gatherIdx = ig + m_pcs->minGroupValue;

        if(muteType == MuteType::BottomMute){
            bool exp = m_bMute->getExtrapolateMute();
            bool inp = m_bMute->getInterpolateMute();
            m_bMute->getMuteValuesOfGather(1, gatherIdx, left, middle, right, exp, inp);
        } else {
            bool exp = m_tMute->getExtrapolateMute();
            bool inp = m_tMute->getInterpolateMute();
            m_tMute->getMuteValuesOfGather(1, gatherIdx, left, middle, right, exp, inp);
        }

        QMap<int, float> picks;
        for(int i=0; i< left.size(); i++) picks.insert(left.at(i).trIdx, left.at(i).time);
        for(int i=0; i< middle.size(); i++) picks.insert(middle.at(i).trIdx, middle.at(i).time);
        for(int i=0; i< right.size(); i++) picks.insert(right.at(i).trIdx, right.at(i).time);

        QList<int> tids = picks.keys();

        //cout << "ig=" << ig << " ng=" << m_pcs->ngroups << " tids.count = " << tids.count() << endl;;

        if(tids.count() < 2)  {
            if(muteType == MuteType::BottomMute){
                for(int i=0; i< ntr; i++){
                    traceIdx = m_pcs->group[ig].traceIdx[i];
                    allMutes[traceIdx]  = m_ns;
                }
            } else {
                for(int i=0; i< ntr; i++){
                    traceIdx = m_pcs->group[ig].traceIdx[i];
                    allMutes[traceIdx]  = 0;
                }
            }

            continue;
        }
        std::sort(tids.begin(), tids.end());

        float* offset = new float [ntr];
        short* muteValues = new short [ntr];

        for(int i=0; i< ntr; i++){
            traceIdx = m_pcs->group[ig].traceIdx[i];
            offset[i] = m_offset[traceIdx];
            muteValues[i] = 0;
        }

        for(int i=0; i<tids.count(); i++){
            muteValues[tids.at(i)] = int(picks[tids.at(i)]*1000000./m_dtus);
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
                    muteValues[j] = int(((t1 - t2)/(off1 - off2)*(off - off2) + t2)*1000000/m_dtus);
                } else {
                    muteValues[j] = int(t2*1000000/m_dtus);
                }
                //if(ig == 0) cout << "muteType=" << muteType << " j="<< j << " off=" << off << " i1=" << i1 << " i2=" << i2 << " off1=" << off1<< " off2=" << off2  << " t1=" << t1 << " t2=" << t2 <<  " time=" << (t1 - t2)/(off1 - off2)*(off - off2) + t2 << endl;
            }
        }

        if(muteType == MuteType::BottomMute){
            for(int i=0; i< ntr; i++){
                traceIdx = m_pcs->group[ig].traceIdx[i];
                if(muteValues[i] <  1 || muteValues[i] > m_ns) allMutes[traceIdx]  = m_ns;
                else allMutes[traceIdx]  = muteValues[i];
            }
        } else {
            for(int i=0; i< ntr; i++){
                traceIdx = m_pcs->group[ig].traceIdx[i];
                if(muteValues[i] < 0 ) allMutes[traceIdx]  = 0;
                else if(muteValues[i] > m_ns) allMutes[traceIdx]  = m_ns;
                else allMutes[traceIdx]  = muteValues[i];

                //if(ig==0) cout << "muteType=" << muteType<< " ig="<< ig << " it="<< i << " traceIdx="<< traceIdx << " mute=" << allMutes[traceIdx] << endl;
            }
        }


        delete [] offset;
        delete [] muteValues;
    }
    //cout << " calculate mute done" << endl;
}

int SeismicData2DPreStack::findTraceIndexWithShotIdxAndOffset(int gIdx, float offset, int& trSeq, float tol)
{    
    return findTraceIndexWithGroupIdxAndOffset(m_pcs, gIdx, offset, trSeq, tol);
}

int SeismicData2DPreStack::findTraceIndexWithRecvIdxAndOffset(int gIdx, float offset, int& trSeq, float tol)
{
    return findTraceIndexWithGroupIdxAndOffset(m_pcr, gIdx, offset, trSeq, tol);
}

int SeismicData2DPreStack::findTraceIndexWithCdpIdxAndOffset(int gIdx, float offset, int& trSeq, float tol)
{
    return findTraceIndexWithGroupIdxAndOffset(m_pcdp, gIdx, offset, trSeq, tol);
}

int SeismicData2DPreStack::findTraceIndexWithGroupIdxAndOffset(gathers* gp, int gIdx, float offset, int& trSeq, float tol)
{
    int trIdx = -1;
    trSeq = -1;

    if(gIdx < gp->minGroupValue || gIdx > gp->maxGroupValue) return trIdx;

    int ig = gIdx - gp->minGroupValue;

    int i=0;
    for(i=0; i< gp->group[ig].ntraces; i++){
        trIdx = gp->group[ig].traceIdx[i];
        if(abs(m_offset[trIdx] - offset) < tol) {
            trSeq = i;
            return trIdx;
        }
    }
    return trIdx;
}

int SeismicData2DPreStack::findTraceSequenceWithinGather(int gatherType, int gIdx, int trIdx)
{
    gathers* gp = getGatherIndexPointer(gatherType);
    if(gIdx < gp->minGroupValue || gIdx > gp->maxGroupValue) return 0;
    int ig = gIdx - gp->minGroupValue;

    for(int i=0; i< gp->group[ig].ntraces; i++){
        int tid = gp->group[ig].traceIdx[i];
        if(tid == trIdx) {
            return i+1;
        }
    }
    return 0;
}

void SeismicData2DPreStack::showAllDisplayWidgets()
{
    for(int i=0; i< widgetList.count(); i++){        
        if(widgetList[i] == m_fanaDock){
            if(m_fanaDock->isFrequencyAnaDockHide()) continue;
        }
        if(widgetList[i] == m_ampDock){
            if(m_ampDock->isAmplitudeDockHide()) continue;
        }
        widgetList[i]->show();
    }
    m_focusedDisplay = m_inGthDisplay;

    if(m_outGthDisplay != nullptr && m_interactiveFunction == InteractiveFunctions::StackVelAnalysis) {
        QString plotTitle = m_inGthDisplay->getPlotTitle() + QString(" : NMO") ;
        m_outGthDisplay->setPlotTitle(plotTitle);
    }
}

void SeismicData2DPreStack::hideTraceLocationPointOnAmpDock(void)
{
    if(m_ampDock != nullptr)    m_ampDock->hideTraceLocationPoint();
}

gathers* SeismicData2DPreStack::getGatherIndexPointer(int gatherType)
{
    gathers* gp = nullptr;;
    switch(gatherType){
    case DisplayGatherType::CommonShot:
        gp = m_pcs;
        break;
    case DisplayGatherType::CommonDepthPoint:
        gp = m_pcdp;
        break;
    case DisplayGatherType::CommonOffset:
        gp = m_pco;
        break;
    case DisplayGatherType::CommonReceiver:
        gp = m_pcr;
        break;
    }
    return gp;
}

gathers* SeismicData2DPreStack::getGatherIndexPointer()
{
    if(m_focusedDisplay == nullptr) return nullptr;
    return getGatherIndexPointer(m_focusedDisplay->getGatherType());
}

int SeismicData2DPreStack::getGatherIndex(int gatherType, int tidx)
{
    int id = -1;
    switch(gatherType){
    case DisplayGatherType::CommonShot:
        //cout << " m_srcid[" << tidx << "]=" << m_srcid[tidx] << endl;
        id = m_srcid[tidx];
        break;
    case DisplayGatherType::CommonDepthPoint:
        //cout << " m_cdpid[" << tidx << "]=" << m_cdpid[tidx] << endl;
        id =  m_cdpid[tidx];
        break;
    case DisplayGatherType::CommonOffset:
        //cout << " m_offid[" << tidx << "]=" << m_offid[tidx] << endl;
        id =  m_offid[tidx];
        break;
    case DisplayGatherType::CommonReceiver:
        //cout << " m_recid[" << tidx << "]=" << m_recid[tidx] << endl;
        id =  m_recid[tidx];
        break;
    }
    return id;
}

void SeismicData2DPreStack::addOneMutePickPoint(int trIdx, float time)
{
    int plotGathereType = m_focusedDisplay->getGatherType();
    int plotGroupIndex  = m_focusedDisplay->getGatherIndex();
    m_currentMute->addPick(plotGathereType, plotGroupIndex, trIdx, time);
    updateMuteDisplayOnAttributeDock(trIdx);
    //cout << "trIdx="<< trIdx << " time="<<time << endl;
}

void SeismicData2DPreStack::removeOneMutePickPoint(int trIdx)
{
    int plotGathereType = m_focusedDisplay->getGatherType();
    int plotGroupIndex  = m_focusedDisplay->getGatherIndex();
    int ntr = m_focusedDisplay->getNumTracesOfCurrentGather();
    int start = trIdx - 2;
    int end = trIdx + 2;
    if(start < 1) start = 1;
    if(end > ntr) end = ntr;
    for(int itr = start; itr <= end; itr++)
        m_currentMute->removePick(plotGathereType, plotGroupIndex, itr);
    updateMuteDisplayOnAttributeDock(trIdx);
}

void SeismicData2DPreStack::updateMuteDisplayOnAttributeDock(int trIdx)
{
    if(m_ampDock == nullptr) return;
    int plotGathereType = m_focusedDisplay->getGatherType();
    int plotGroupIndex  = m_focusedDisplay->getGatherIndex();
    int traceIdx = getTraceIndexWithinWholeDataset(plotGathereType, plotGroupIndex, trIdx);
    int attType = m_ampDock->getDisplayAttType();
    if(attType==3 && m_currentMute->getMuteType()==MuteType::TopMute){
        m_ampDock->generateMuteScatterValues(MuteType::TopMute);
    } else if(attType==4 && m_currentMute->getMuteType()==MuteType::BottomMute){
        m_ampDock->generateMuteScatterValues(MuteType::BottomMute);
    }
    m_ampDock->setDataForCurrentTracePoint(traceIdx);
}

void SeismicData2DPreStack::setMuteValuesOfCurrentGather(bool visible)
{
    //cout << "setMuteValuesOfCurrentGather visible=" << visible << endl;
    if(visible){
        QList<IdxVal> left;
        QList<IdxVal> middle;
        QList<IdxVal> right;
        int gtype = m_inGthDisplay->getGatherType();
        int gidx = m_inGthDisplay->getGatherIndex();
        m_currentMute->getMuteValuesOfGather(gtype, gidx, left, middle, right);
        m_currentMute->calMuteForAllTraces(m_pcs);
        m_inGthDisplay->displayMutePicks(left, middle, right);
        left.clear();
        middle.clear();
        right.clear();
        bool applyTopMute = m_focusedDisplay->isApplyTopMute();
        bool applybtmMute = m_focusedDisplay->isApplyBtmMute();
        if(applyTopMute || applybtmMute) {
            m_focusedDisplay->updataGatherDisplay();
        }
    }
    m_inGthDisplay->setMutePicksVisible(visible);
}

short* SeismicData2DPreStack::getTopMuteForAllTraces(void)
{
    return m_tMute->getFilledMutes();
}
short* SeismicData2DPreStack::getBtmMuteForAllTraces(void)
{
    return m_bMute->getFilledMutes();
}

void SeismicData2DPreStack::setupStackVelocityAnalysis(Sdp2dQDomDocument* para, bool processWholeData)
{
    QString moduleName = para->getModuleName();
    if(moduleName.compare("iVelAna") != 0) return;
    if(m_velSemb == nullptr){
        m_velSemb = new Sdp2dVelSemblanceDisplayArea(m_mainWindow);
        m_mainWindow->addWidgetToTheCentralSplitView(m_velSemb);
    }

    if(processWholeData){
        m_velSemb->processWholeData(para);
    } else {
        Sdp2dStackVelocityAnalysis* velana = m_velSemb->setupSembDisplay(para);
        if(velana == nullptr) {
            cout << "velana IS nullptr" << endl;
            return;
        }

        if(m_outGthDisplay == nullptr) {
            m_outGthDisplay = new Sdp2dProcessedGatherDisplayArea(dynamic_cast<SeismicData2D*>(this), m_mainWindow);
            m_mainWindow->addWidgetToTheCentralSplitView(m_outGthDisplay);
            m_velSemb->setNMOGatherDisplayPointer(m_outGthDisplay);
        }
        prepareStackVelocityAnalysis();
    }
}

void SeismicData2DPreStack::prepareStackVelocityAnalysis()
{
    if(m_outGthDisplay == nullptr) {
        cout << "m_outGthDisplay IS nullptr. WRONG!!!" << endl;
        return;
    }

    Sdp2dStackVelocityAnalysis* velana = m_velSemb->getStackVelAnaWorker();
    if(velana == nullptr) {
        cout << "velana IS nullptr" << endl;
        return;
    }

    m_velSemb->processCurrentGather();
}

void SeismicData2DPreStack::loadPickedNMOVelocities()
{
    //cout << "in SeismicData2DPreStack::loadPickedNMOVelocities" << endl;
    if(isIdxFileHasNMOVelocities()){
        m_velPicks.clear();

        QFile idxf(QString::fromUtf8(m_idxfile.data(), m_idxfile.size()));
        idxf.open(QIODevice::ReadOnly | QIODevice::ExistingOnly);
        qint64 pos = m_idxPos + m_ntr*240;
        idxf.seek(pos);

        // load NMO velocity picks HERE
        int ncdps = 0;
        int cdpIdx = 0;
        int npicks = 0;
        QVector<value_idx> tvs;

        idxf.read((char*)&ncdps, 4);
        for(int i=0; i < ncdps; i++){
            idxf.read((char*)&cdpIdx, 4);
            idxf.read((char*)&npicks, 4);
            tvs.resize(npicks);
            idxf.read((char*)tvs.data(), npicks*sizeof(value_idx));
            for(int j=0; j< npicks; j++){
                m_velPicks.insert(cdpIdx, tvs.at(j));
            }
        }
        idxf.close();
    }
}

void SeismicData2DPreStack::unloadPickedNMOVelocities()
{
    //cout << "in SeismicData2DPreStack::unloadPickedNMOVelocities" << endl;

    QFile idxf(QString::fromUtf8(m_idxfile.data(), m_idxfile.size()));
    idxf.open(QIODevice::ReadWrite | QIODevice::ExistingOnly);
    qint64 pos = m_idxPos + m_ntr*240;
    idxf.seek(pos);

    // write NMO velocity picks HERE
    QList<int> cdps = m_velPicks.uniqueKeys();
    int ncdps = cdps.count();
    idxf.write((const char*)&ncdps, 4);
    for(int i=0; i < ncdps; i++){
        int cdpIdx = cdps.at(i);
        QList<value_idx> tvPairs = m_velPicks.values(cdpIdx);
        int npicks = tvPairs.count();
        idxf.write((const char*)&cdpIdx, 4);
        idxf.write((const char*)&npicks, 4);
        QVector<value_idx> tvs = tvPairs.toVector();
        idxf.write((const char*)tvs.data(), npicks*sizeof(value_idx));
    }

    idxf.close();
    m_velPicks.clear();
}


float** SeismicData2DPreStack::getinputGatherUsingOutputOption(int gatherIdx)
{
    // Using the display gather type
    int gatherType = getGatherType();

    gathers* p = getGatherIndexPointer(gatherType);
    int ntr = getNumberOfTracesInOneGather(gatherIdx, p);
    if(ntr <= 0) return nullptr;
    int ns = m_ns;
    float** indata = Sdp2dUtils::alloc2float(ns, ntr);
    int idx = gatherIdx - p->minGroupValue;

    for(int i=0; i< ntr; i++){
        int traceIdx = p->group[idx].traceIdx[i];
         m_sgy->getTraceDataFromIntermFile(traceIdx, indata[i]);
    }

    short* topMuteValues = nullptr;
    short* btmMuteValues = nullptr;

    bool*  badTraces  = new bool [ntr];
    for(int i=0; i< ntr; i++) badTraces[i]  = false;

    float* weight =  new float [m_ns];
    for(int i=0; i< m_ns; i++) weight[i] =1;

    int taperLength = getMuteTaperLength();

    if(m_outputApplyTopMute){
        topMuteValues = new short [ntr];
        memset((void*)topMuteValues, 0, ntr*sizeof(short));
        getTracesMute(gatherType, gatherIdx, topMuteValues, 1);
    }
    if(m_outputApplyBtmMute){
        btmMuteValues = new short [ntr];
        memset((void*)btmMuteValues, 0, ntr*sizeof(short));
        getTracesMute(gatherType, gatherIdx, btmMuteValues, 2);
    }

    if(m_discardBadTraces){
        getTraceFlags(gatherType, gatherIdx, badTraces);
    }

    float datavalue = 0;

    for (int xIndex=0; xIndex<ntr; xIndex++){

        if(m_outputApplyTopMute || m_outputApplyBtmMute) {
            int top = 0;
            int btm = m_ns;
            if(m_outputApplyTopMute) top = topMuteValues[xIndex];
            if(m_outputApplyBtmMute) btm = btmMuteValues[xIndex];
            //cout << " xIndex=" << xIndex << " topmute=" << top << " btmMute="<< btm << endl;
            calculateTraceWeightValues(top, btm, weight, taperLength);
        }

        for (int yIndex=0; yIndex<m_ns; yIndex++){
            datavalue = indata[xIndex][yIndex] * weight[yIndex];
            if(badTraces[xIndex]) datavalue = 0;
            indata[xIndex][yIndex] = datavalue;
        }
    }

    if(topMuteValues != nullptr) delete [] topMuteValues;
    if(btmMuteValues != nullptr) delete [] btmMuteValues;
    delete [] weight;
    delete [] badTraces;

    return indata;
}


