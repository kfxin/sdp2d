#include "sdp2dPMElevationStaticCorrection.h"
#include "seismicdataprocessing2d.h"
#include "sdp2dQDomDocument.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "sdp2dUtils.h"

#include <iostream>
#include <QMessageBox>

using namespace std;

Sdp2dPMElevationStaticCorrection::Sdp2dPMElevationStaticCorrection(SeismicDataProcessing2D *parent) : Sdp2dProcessModule(parent)
{
    m_moduleName = QString("ElevStatic");
}

Sdp2dPMElevationStaticCorrection::~Sdp2dPMElevationStaticCorrection()
{

}

bool Sdp2dPMElevationStaticCorrection::setupParameters(Sdp2dQDomDocument* para)
{
    if(m_mainWindow->getCurrentDataPointer()->getDataType() != SeismicDataType::PreStack) {
        QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("Processing Modeul %1 is only applyablr for pre-stack seismic data!").arg(m_moduleName), QMessageBox::Ok, NULL);
        message.exec();
        return false;
    }
    //cout << "setupParameters for : " << para->getModuleName().toStdString().c_str() <<  endl;

    m_datum = para->getParameterInGroup("Datum").toFloat();
    m_wevel = para->getParameterInGroup("WeatheringVelocity").toFloat();
    m_swevel = para->getParameterInGroup("SubWeatheringVelocity").toFloat();
    m_btmOfWL = para->getParameterInGroup("BottomOfWeatheringLayer").toFloat();
    m_sign = 1.0;
    if(para->getParameterInGroup("ApplyType").compare("Remove") == 0) {
        m_sign = -1.0;
    }

    m_useHeader = false;
    if(para->getParameterInGroup("StaticsFromTraceHeader").compare("True") == 0) {
        m_useHeader = true;
    }
    return true;
}

void Sdp2dPMElevationStaticCorrection::processCurrentGather(float** indata, float** outdata, int ntr)
{
    SeismicData2DPreStack* sd2d = dynamic_cast<SeismicData2DPreStack*>(m_mainWindow->getCurrentDataPointer());
    int gType = sd2d->getGatherType() ;
    int gIdx  = sd2d->getPlotGroupIndex();

    int ns = sd2d->getSamplesPerTraces();
    float dt = float(sd2d->getTimeSampleRateInUs())/1000000.0;

    float* time = new float [ns];
    //cout << " m_datum=" << m_datum << " m_wevel="<< m_wevel << " m_swevel=" << m_swevel << " m_sign="<< m_sign << endl;
    for(int i=0; i< ntr; i++){
        int tidx = sd2d->getTracesIndexInData(gType, gIdx, i+1);
        float sdep = sd2d->getSDepthOfATrace(tidx);
        float sele = sd2d->getSElevOfATrace(tidx);
        float rele = sd2d->getRelevOfATrace(tidx);

        // the datum is the bottom of the weathering layer
        float tse = (m_datum - sele + sdep)/m_swevel;
        float tre = (m_datum - rele)/m_swevel;
        //float tstat = tsd + trd + (sele - rele)/m_wevel;
        float tsw = (sele - sdep - m_btmOfWL)*(1.0/m_wevel - 1.0/m_swevel);
        float trw = (rele - m_btmOfWL)*(1.0/m_wevel - 1.0/m_swevel);
        //tsw = (m_btmOfWL)*(1.0/m_wevel - 1.0/m_swevel);
        //trw = (m_btmOfWL)*(1.0/m_wevel - 1.0/m_swevel);
        float tstat = tse + tre - tsw - trw;

        //cout << "tr="<< i+1 << " sele=" << sele << " sdep=" << sdep << " rele=" <<  rele << " tse=" << tse << " tre=" << tre << " tstat=" << tstat << endl;
        for (int itime=0; itime<ns; ++itime) {
            time[itime] = itime*dt - m_sign*tstat;

        }
        Sdp2dUtils::ints8r(ns, dt, 0, indata[i], 0.0, 0.0, ns, time, outdata[i]);
    }

    delete [] time;
}

void Sdp2dPMElevationStaticCorrection::processWholeData(void)
{

}
