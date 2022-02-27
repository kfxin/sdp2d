#include "sdp2dStackVelocityAnalysis.h"
#include "sdp2dGatherDisplayArea.h"
#include "sdp2dMainGatherDisplayArea.h"
#include "sdp2dQDomDocument.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "sdp2dUtils.h"

#include <QMessageBox>
#include <QObject>

#include <iostream>
using namespace std;

Sdp2dStackVelocityAnalysis::Sdp2dStackVelocityAnalysis(SeismicData2DPreStack* sd2d)
    : QObject(sd2d)
{

    m_sd2d = sd2d;

    if(m_sd2d->getDataType() != SeismicDataType::PreStack){
        QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("The input seismic data must be prestack data!"), QMessageBox::Ok, NULL);
        message.exec();
    }

    m_semblance = nullptr;
    m_algorithm = 1;
    m_nv = 50;        //number of velocities
    m_dv = 50;        //velocity sampling interval
    m_fv = 1500;      //first velocity
    m_anis1 = 0.0;    //quartic term, numerator of an extended quartic term
    m_anis2 = 0.0;    //in denominator of an extended quartic term
    m_smute = 1.5;    //samples with NMO stretch exceeding smute are zeroed
    m_pwr = 1.0;      //semblance value to the power
    m_tau = 1.5;
    m_dtratio = 5;    //ratio of output to input time sampling intervals
    m_nsmooth = 11;

    m_lmute = 25;
    m_sscale = true;
    m_invert = false;
    m_upward = false;
    m_interp = true;

    m_ntin = m_sd2d->getSamplesPerTraces();
    m_dtin = float(m_sd2d->getTimeSampleRateInUs())/1000000.0;    

    m_ntout = 1+ (m_ntin-1)/m_dtratio;
    m_dtout = m_dtin*m_dtratio;
}

Sdp2dStackVelocityAnalysis::~Sdp2dStackVelocityAnalysis()
{
    if(m_semblance != nullptr) Sdp2dUtils::free2float(m_semblance);
}

bool Sdp2dStackVelocityAnalysis::getParametersFromDom(Sdp2dQDomDocument* para)
{

    QString algorithm = para->getParameterInGroup("Algorithm", "VelocitySemblance");
    //cout << "algorithm =  " << algorithm.toStdString().c_str() << endl;
    m_algorithm  = 1;
    if(algorithm.compare("NCCS") == 0){
        m_algorithm  = 2;
    } else if(algorithm.compare("NSEL") == 0){
        m_algorithm  = 3;
    } else if(algorithm.compare("UCCS") == 0){
        m_algorithm  = 4;
    } else if(algorithm.compare("USEL") == 0){
        m_algorithm  = 5;
    }

    m_nv = para->getParameterInGroup("NumVelocities", "VelocitySemblance").toInt();
    m_dv = para->getParameterInGroup("VelInterval", "VelocitySemblance").toFloat();
    m_fv = para->getParameterInGroup("FirstVelocity", "VelocitySemblance").toFloat();
    m_anis1 = para->getParameterInGroup("AnisoFactor1", "VelocitySemblance").toFloat();
    m_anis2 = para->getParameterInGroup("AnisoFactor2", "VelocitySemblance").toFloat();
    m_smute = para->getParameterInGroup("StretchMute", "VelocitySemblance").toFloat();
    m_pwr   = para->getParameterInGroup("Power", "VelocitySemblance").toFloat();
    m_tau   = para->getParameterInGroup("Tau", "VelocitySemblance").toFloat();
    m_dtratio = para->getParameterInGroup("DtRatio", "VelocitySemblance").toInt();
    m_nsmooth = para->getParameterInGroup("SmoothLength", "VelocitySemblance").toInt();


    m_lmute = para->getParameterInGroup("LinearRampLength", "NMO").toInt();
    m_sscale = true;
    if(para->getParameterInGroup("StretchScale", "NMO").compare("False") == 0){
        m_sscale = false;
    }

    m_invert = true;
    if(para->getParameterInGroup("InvertNMO", "NMO").compare("False") == 0){
        m_invert = false;
    }

    m_upward = true;
    if(para->getParameterInGroup("UpwardScan", "NMO").compare("False") == 0){
        m_upward = false;
    }

    m_interp = true;
    if(para->getParameterInGroup("InterpolateVel", "NMO").compare("False") == 0){
        m_interp = false;
    }

    m_ft = 0;

    if (m_smute<=1.0) {
        QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("smute must be greater than 1.0!"), QMessageBox::Ok, NULL);
        message.exec();
        return false;
    }

    if (m_pwr<=0.0) {
        QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("m_pwr must be greater than 0.0! \npwr < 0, we are not looking for noise. \npwr = 0, we are creating an all-white semblance. "), QMessageBox::Ok, NULL);
        message.exec();
        return false;
    }

    m_ntout = 1+ (m_ntin-1)/m_dtratio;
    m_dtout = m_dtin*m_dtratio;

    if(m_semblance != nullptr) Sdp2dUtils::free2float(m_semblance);
    m_semblance = Sdp2dUtils::alloc2float(m_ntout, m_nv);

    return true;
}

void Sdp2dStackVelocityAnalysis::setParametersToDom(Sdp2dQDomDocument* para)
{
    QString algorithm = QString("Conventional");
    if(m_algorithm == 2){
        algorithm = QString("NCCS");
    } else if(m_algorithm == 3){
        algorithm = QString("NSEL");
    } else if(m_algorithm == 4){
        algorithm = QString("UCCS");
    } else if(m_algorithm == 5){
        algorithm = QString("USEL");
    }

    para->setParameterInGroup("Algorithm", algorithm, "VelocitySemblance");

    para->setParameterInGroup("NumVelocities", QString::number(m_nv), "VelocitySemblance");
    para->setParameterInGroup("VelInterval", QString::number(m_dv), "VelocitySemblance");
    para->setParameterInGroup("FirstVelocity", QString::number(m_fv), "VelocitySemblance");
    para->setParameterInGroup("AnisoFactor1", QString::number(m_anis1), "VelocitySemblance");
    para->setParameterInGroup("AnisoFactor2", QString::number(m_anis2), "VelocitySemblance");
    para->setParameterInGroup("StretchMute", QString::number(m_smute), "VelocitySemblance");
    para->setParameterInGroup("Power", QString::number(m_pwr), "VelocitySemblance");
    para->setParameterInGroup("Tau", QString::number(m_tau), "VelocitySemblance");
    para->setParameterInGroup("DtRatio", QString::number(m_dtratio), "VelocitySemblance");
    para->setParameterInGroup("SmoothLength", QString::number(m_nsmooth), "VelocitySemblance");

    para->setParameterInGroup("LinearRampLength", QString::number(m_lmute), "NMO");

    if(m_sscale){
        para->setParameterInGroup("StretchScale", "True", "NMO");
    } else {
        para->setParameterInGroup("StretchScale", "False", "NMO");
    }

    if(m_upward){
        para->setParameterInGroup("UpwardScan", "True", "NMO");
    } else {
        para->setParameterInGroup("UpwardScan", "False", "NMO");
    }

    if(m_invert){
        para->setParameterInGroup("InvertNMO", "True", "NMO");
    } else {
        para->setParameterInGroup("InvertNMO", "False", "NMO");
    }

    if(m_interp){
        para->setParameterInGroup("InterpolateVel", "True", "NMO");
    } else {
        para->setParameterInGroup("InterpolateVel", "False", "NMO");
    }

}

float** Sdp2dStackVelocityAnalysis::processCurrentGather(float** indata, int ntr)
{
    if(m_sd2d->getDataType() != SeismicDataType::PreStack){
        QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("The input seismic data must be prestack data!"), QMessageBox::Ok, NULL);
        message.exec();
        return nullptr;
    }

    m_offset = m_sd2d->getInputGatherDisplayPointer()->getOffsetOfTheGather().data();

    //cout << "processCurrentGather: m_algorithm = " << m_algorithm << endl;
    switch (m_algorithm) {
    case 1:
        velannConventional(indata, ntr);
        break;
    case 2:
        velannNCCS(indata, ntr);
        break;
    case 3:
        velannNSEL(indata, ntr);
        break;
    case 4:
        velannUCCS(indata, ntr);
        break;
    case 5:
        velannUSEL(indata, ntr);
        break;
    default:
        cout << "should not reach here" << endl;
        break;
    }
    return m_semblance;
}

void Sdp2dStackVelocityAnalysis::processWholeData(void)
{

}

void Sdp2dStackVelocityAnalysis::velannConventional(float** indata, int ntr)
{
    float** num = Sdp2dUtils::alloc2float(m_ntin, m_nv);
    float** den = Sdp2dUtils::alloc2float(m_ntin, m_nv);
    float** nnz = Sdp2dUtils::alloc2float(m_ntin, m_nv);
    memset((void*)num[0], 0, m_ntin*m_nv*sizeof(float));
    memset((void*)den[0], 0, m_ntin*m_nv*sizeof(float));
    memset((void*)nnz[0], 0, m_ntin*m_nv*sizeof(float));

    memset((void*)m_semblance[0], 0, m_ntout*m_nv*sizeof(float));

    for(int itr=0; itr< ntr; itr++){
        float v = m_fv;
        float offset = m_offset[itr];
        float factor = 1.0 + offset*offset*m_anis2;
        float offan = offset*offset*offset*offset*m_anis1;
        if(factor > 1.0E-5){
            offan = offan/factor;
        }

        for (int iv=0; iv<m_nv; iv++) {
            /* compute offset/velocity squared */
            float offovs = (offset*offset)/(v*v) + offan;

            /* decrease of traveltime with distance due to highly
               increasing velocity cannot be handled yet
            */
            if (offovs < -1.0E-5){
                QMessageBox message(QMessageBox::NoIcon,
                         "Warn", QString("no moveout; check anis1 and anis2"), QMessageBox::Ok, NULL);
                message.exec();
                offovs = 0;
            }

            /* determine mute time after nmo */
            float tnmute = sqrt(offovs/(m_smute*m_smute-1.0));
            int itmute = 0;
            if (tnmute > m_ft) itmute = (tnmute-m_ft)/m_dtin;

            /* do nmo via quick and dirty linear interpolation
               (accurate enough for velocity analysis) and
               accumulate semblance numerator and denominator
            */
            float tn = m_ft + itmute * m_dtin;
            for (int it=itmute; it < m_ntin; it++){
                float ti = (sqrt(tn*tn+offovs)-m_ft)/m_dtin;
                int iti = ti;
                if (iti < m_ntin - 1) {
                    float frac = ti-iti;
                    float temp = (1.0-frac)*indata[itr][iti]+ frac*indata[itr][iti+1];
                    if(qAbs(temp) > 1.0E-5){
                        num[iv][it] += temp;
                        den[iv][it] += temp*temp;
                        nnz[iv][it] += 1.0;
                    }
                }
                tn += m_dtin;
            }
            v += m_dv;
        }
    }

    /* loop over velocities */
    for (int iv=0; iv < m_nv; ++iv) {
        /* compute semblance quotients */
        for (int itout=0; itout<m_ntout; ++itout) {
             int it = itout*m_dtratio;
             int ismin = it-m_nsmooth/2;
             int ismax = it+m_nsmooth/2;
             if (ismin<0) ismin = 0;
             if (ismax>m_ntin-1) ismax = m_ntin-1;
             float nsum = 0.0;
             float dsum = 0.0;
             for (int is=ismin; is<ismax; ++is) {
                  nsum += num[iv][is] * num[iv][is];
                  dsum += nnz[iv][is] * den[iv][is];
             }
             if(dsum > 0) m_semblance[iv][itout] = (dsum!=0.0?nsum/dsum:0.0);
         }

        /* powering the semblance */
         if (qAbs(m_pwr - 1.0) > 1.0E-5) {
            for (int itout=0; itout<m_ntout; ++itout)
            m_semblance[iv][itout] = pow (m_semblance[iv][itout], m_pwr);
        }
    }

    Sdp2dUtils::free2float(num);
    Sdp2dUtils::free2float(den);
    Sdp2dUtils::free2float(nnz);
}

void Sdp2dStackVelocityAnalysis::velannNCCS(float** indata, int ntr)
{
    float** datn = Sdp2dUtils::alloc2float(m_ntin, ntr);
    float* norm = new float [ntr];
    memset((void*)datn[0], 0, m_ntin*ntr*sizeof(float));

    int inc = ntr*(ntr-1)/2;

    float v = m_fv;
    for (int iv=0; iv<m_nv; ++iv) {
        for (int itr=0; itr < ntr; ++itr) {
            float offovs = (m_offset[itr]*m_offset[itr])/(v*v);
            float tnmute = sqrt(offovs/(m_smute*m_smute-1.0));
            int itmute = 0;
            if (tnmute > m_ft) itmute = (tnmute-m_ft)/m_dtin;
            float tn = m_ft + itmute * m_dtin;
            for (int it=itmute; it < m_ntin; it++){
                float ti = (sqrt(tn*tn+offovs)-m_ft)/m_dtin;
                int iti = ti;
                if (iti < m_ntin - 1) {
                    float frac = ti-iti;
                    datn[itr][it] = (1.0-frac)*indata[itr][iti]+ frac*indata[itr][iti+1];
                }
                tn += m_dtin;
            }
        }

        for (int itout=0; itout<m_ntout; ++itout){
            int it = itout*m_dtratio;
            int ismin = it-m_nsmooth/2;
            int ismax = it+m_nsmooth/2;
            if (ismin<0) ismin = 0;
            if (ismax>m_ntin-1) ismax = m_ntin-1;

            float temp1=0.0;
            for (int ix=0; ix<ntr; ++ix){
                norm[ix]=0.0;
                for (int is=ismin; is<ismax; ++is){
                    temp1+=(datn[ix][is]*datn[ix][is]);
                }
                norm[ix] = sqrt(temp1)+1;
                temp1=0.0;
            }

            float ff = 0.0;
            float nsum = 0.0;
            float dsum = 0.0;
            float fac = 0.0;
            for (int is=ismin; is<ismax; ++is){
                dsum = 0.0;
                fac = 0.0;
                for (int ix=0; ix<ntr; ++ix) {
                    nsum = datn[ix][is]/norm[ix];
                    dsum = dsum + nsum;
                    fac  = fac + nsum*dsum;
                }
                ff  = ff + fac;
            }
            m_semblance[iv][itout] = ff/inc;
        }
        v += m_dv;
    }

    Sdp2dUtils::free2float(datn);
    delete [] norm;
}


void Sdp2dStackVelocityAnalysis::velannNSEL(float** indata, int ntr)
{    
    float dx = m_sd2d->getOffsetSpacing();

    float** datn = Sdp2dUtils::alloc2float(m_ntin, ntr);
    float* norm = new float [ntr];
    memset((void*)datn[0], 0, m_ntin*ntr*sizeof(float));

    float ol = floor(qAbs(m_offset[0]));
    float m = 0.0;
    for (int ix=1;ix<ntr; ++ix) {
         float oaux = floor(qAbs(m_offset[ix]));
         m = (oaux < ol) ? oaux : ol;
    }
    ol  = m/dx;
    int j0t = floor(sqrt(m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol)+ol*ol)-ol)+2;
    int beta = 0;
    for (int ix=j0t; ix <= ntr; ++ix) {
        beta += floor(sqrt((ix+ol-1)*(ix+ol-1)-m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol))-ol);
    }
    //int tncc = (ntr-1)*ntr/2;
    //float pcc = (float) beta*100/tncc;

    float v = m_fv;
    for (int iv=0; iv<m_nv; ++iv) {
        for (int itr=0; itr < ntr; ++itr) {
            float offovs = (m_offset[itr]*m_offset[itr])/(v*v);
            float tnmute = sqrt(offovs/(m_smute*m_smute-1.0));
            int itmute = 0;
            if (tnmute > m_ft) itmute = (tnmute-m_ft)/m_dtin;
            float tn = m_ft + itmute * m_dtin;
            for (int it=itmute; it < m_ntin; it++){
                float ti = (sqrt(tn*tn+offovs)-m_ft)/m_dtin;
                int iti = ti;
                if (iti < m_ntin - 1) {
                    float frac = ti-iti;
                    datn[itr][it] = (1.0-frac)*indata[itr][iti]+ frac*indata[itr][iti+1];
                }
                tn += m_dtin;
            }
        }

        for (int itout=0; itout<m_ntout; ++itout){
            int it = itout*m_dtratio;
            int ismin = it-m_nsmooth/2;
            int ismax = it+m_nsmooth/2;
            if (ismin<0) ismin = 0;
            if (ismax>m_ntin-1) ismax = m_ntin-1;

            float temp1=0.0;
            for (int itr=0; itr<ntr; ++itr){
                norm[itr]=0.0;
                for (int is=ismin; is<ismax; ++is){
                    temp1+=(datn[itr][is]*datn[itr][is]);
                }
                norm[itr] = sqrt(temp1)+1;
                temp1=0.0;
            }

            float ff = 0.0;
            float nsum = 0.0;
            float dsum = 0.0;
            float fac = 0.0;
            for (int is=ismin; is<ismax; ++is){
                dsum = 0.0;
                int ktj = floor(sqrt((j0t+ol-1)*(j0t+ol-1)- m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol))-ol);

                for (int ik=0; ik<ktj; ++ik) {
                    dsum +=datn[ik][is]/norm[ik];
                }
                fac = datn[j0t][is]/norm[j0t]*dsum;

                for (int ix=j0t+1; ix < ntr; ++ix) {
                    int ktj1 = floor(sqrt(((ix-1)+ol-1)*((ix-1)+ol-1)- m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol))-ol)+1;
                    int ktj2 = floor(sqrt((ix+ol-1)*(ix+ol-1)- m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol))-ol);
                    int ik = ktj1;
                    float temp = 0;
                    while (ik<=ktj2){
                        temp += datn[ik][is]/norm[ik];
                        ++ik;
                    }
                    dsum = dsum + temp;
                    nsum = datn[ix][is]/norm[ix];
                    fac  = fac + nsum*dsum;
                }
                ff  = ff + fac;
            }
            if (ff < 0)  ff = 0;
            m_semblance[iv][itout] = ff/beta;
        }
        v += m_dv;
    }

    Sdp2dUtils::free2float(datn);
    delete [] norm;
}


void Sdp2dStackVelocityAnalysis::velannUCCS(float** indata, int ntr)
{
    float** datn = Sdp2dUtils::alloc2float(m_ntin, ntr);
    memset((void*)datn[0], 0, m_ntin*ntr*sizeof(float));

    float v = m_fv;
    for (int iv=0; iv<m_nv; ++iv) {
        for (int ix=0; ix < ntr; ++ix) {
            float offovs = (m_offset[ix]*m_offset[ix])/(v*v);
            float tnmute = sqrt(offovs/(m_smute*m_smute-1.0));
            int itmute = 0;
            if (tnmute > m_ft) itmute = (tnmute-m_ft)/m_dtin;
            float tn = m_ft + itmute * m_dtin;
            for (int it=itmute; it < m_ntin; it++){
                float ti = (sqrt(tn*tn+offovs)-m_ft)/m_dtin;
                int iti = ti;
                if (iti < m_ntin - 1) {
                    float frac = ti-iti;
                    datn[ix][it] = (1.0-frac)*indata[ix][iti]+ frac*indata[ix][iti+1];
                }
                tn += m_dtin;
            }
        }

        for (int itout=0; itout<m_ntout; ++itout){
            int it = itout*m_dtratio;
            int ismin = it-m_nsmooth/2;
            int ismax = it+m_nsmooth/2;
            if (ismin<0) ismin = 0;
            if (ismax>m_ntin-1) ismax = m_ntin-1;

            float ff=0.0;
            for (int is=ismin; is<ismax; ++is){
                float nsum = 0.0;
                float dsum = 0.0;
                float fac = 0.0;
                for (int ix=0; ix<ntr; ++ix) {
                    nsum = datn[ix][is];
                    dsum = dsum + nsum;
                    fac  = fac + nsum*dsum;
                }
                ff  = ff + fac;
            }
            m_semblance[iv][itout] = ff;
        }
        v += m_dv;
    }

    Sdp2dUtils::free2float(datn);
}


void Sdp2dStackVelocityAnalysis::velannUSEL(float** indata, int ntr)
{
    float dx = m_sd2d->getOffsetSpacing();

    float** datn = Sdp2dUtils::alloc2float(m_ntin, ntr);
    memset((void*)datn[0], 0, m_ntin*ntr*sizeof(float));

    float ol = floor(qAbs(m_offset[0]));
    float m = 0.0;
    for (int ix=1;ix<ntr; ++ix) {
         float oaux = floor(qAbs(m_offset[ix]));
         m = (oaux < ol) ? oaux : ol;
    }
    ol  = m/dx;
    int j0t = floor(sqrt(m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol)+ol*ol)-ol)+2;
    //int beta = 0;
    //for (int ix=j0t; ix <= ntr; ++ix) {
    //    beta += floor(sqrt((ix+ol-1)*(ix+ol-1)-m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol))-ol);
    //}
    //int tncc = (ntr-1)*ntr/2;
    //float pcc = (float) beta*100/tncc;

    float v = m_fv;
    for (int iv=0; iv<m_nv; ++iv) {
        for (int ix=0; ix < ntr; ++ix) {
            float offovs = (m_offset[ix]*m_offset[ix])/(v*v);
            float tnmute = sqrt(offovs/(m_smute*m_smute-1.0));
            int itmute = 0;
            if (tnmute > m_ft) itmute = (tnmute-m_ft)/m_dtin;
            float tn = m_ft + itmute * m_dtin;
            for (int it=itmute; it < m_ntin; it++){
                float ti = (sqrt(tn*tn+offovs)-m_ft)/m_dtin;
                int iti = ti;
                if (iti < m_ntin - 1) {
                    float frac = ti-iti;
                    datn[ix][it] = (1.0-frac)*indata[ix][iti]+ frac*indata[ix][iti+1];
                }
                tn += m_dtin;
            }
        }

        for (int itout=0; itout<m_ntout; ++itout){
            int it = itout*m_dtratio;
            int ismin = it-m_nsmooth/2;
            int ismax = it+m_nsmooth/2;
            if (ismin<0) ismin = 0;
            if (ismax>m_ntin-1) ismax = m_ntin-1;

            float ff = 0.0;
            float nsum = 0.0;
            float dsum = 0.0;
            float fac = 0.0;
            for (int is=ismin; is<ismax; ++is){
                dsum = 0.0;
                int ktj = floor(sqrt((j0t+ol-1)*(j0t+ol-1)- m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol))-ol);

                for (int ik=0; ik<ktj; ++ik) {
                    dsum +=datn[ik][is];
                }
                fac = datn[j0t][is]*dsum;

                for (int ix=j0t+1; ix < ntr; ++ix) {
                    int ktj1 = floor(sqrt(((ix-1)+ol-1)*((ix-1)+ol-1)- m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol))-ol)+1;
                    int ktj2 = floor(sqrt((ix+ol-1)*(ix+ol-1)- m_tau*((ntr+ol-1)*(ntr+ol-1)-ol*ol))-ol);
                    int ik = ktj1;
                    float temp = 0;
                    while (ik<=ktj2){
                        temp += datn[ik][is];
                        ++ik;
                    }
                    dsum = dsum + temp;
                    nsum = datn[ix][is];
                    fac  = fac + nsum*dsum;
                }
                ff  = ff + fac;
            }
            if (ff < 0)  ff = 0;
            m_semblance[iv][itout] = ff;
        }
        v += m_dv;
    }

    Sdp2dUtils::free2float(datn);
}

void Sdp2dStackVelocityAnalysis::applyNmoOnCurrentGather(float** indata, float** outdata, float* ovv, int ntr)
{
    int it = 0;
    float tn = 0;
    float ft = 0;
    float dt = m_dtin;
    int nt = m_ntin;
    int itmute = 0;

    float tsq = 0;
    float osmute = 0;
    float* ttn = new float [m_ntin];
    float* atn = new float [m_ntin];
    float* tnt = new float [m_ntin];
    float* at = new float [m_ntin];
    float* qtn = new float [m_ntin];
    float* qt = new float [m_ntin];

    for (int ix=0; ix<ntr; ++ix) {
         float offset = m_offset[ix];

         /* compute time t(tn) (normalized) */
         float temp = ((float) offset*offset)/(dt*dt);
         for (it=0,tn=ft/dt; it<nt; ++it,tn+=1.0) {
             tsq = temp*ovv[it];
             ttn[it] = sqrt (tn*tn + tsq);
         }

         /* compute inverse of stretch factor a(tn) */
         atn[0] = ttn[1]-ttn[0];
         for (it=1; it<nt; ++it)  atn[it] = ttn[it]-ttn[it-1];

         /* determine index of first sample to survive mute */
         osmute = 1.0/m_smute;
         if(!m_upward) {
             for (it=0; it<nt-1 && atn[it]<osmute; ++it);
         } else {
             /* scan samples from bottom to top */
             for (it=nt-1; it>0 && atn[it]>=osmute; --it);
         }
         itmute = it;

         /* if inverse NMO will be performed */
         if (m_invert) {
             /* compute tn(t) from t(tn) */
             Sdp2dUtils::yxtoxy(nt-itmute, 1.0, ft/dt+itmute, &ttn[itmute],
                 nt-itmute, 1.0, ft/dt+itmute,   ft/dt-nt, ft/dt+nt, &tnt[itmute]);

             /* adjust mute time */
             itmute = 1.0+ttn[itmute]-ft/dt;
             itmute = qMin(nt-2,itmute);

             /* compute a(t) */
             if (m_sscale) {
                 for (it=itmute+1; it<nt; ++it) at[it] = tnt[it]-tnt[it-1];
                 at[itmute] = at[itmute+1];
             }
         }

         /* if forward (not inverse) nmo */
         if (!m_invert) {

             /* do nmo via 8-point sinc interpolation */
             Sdp2dUtils::ints8r(nt, 1.0, ft/dt, indata[ix], 0.0, 0.0, nt-itmute, &ttn[itmute], &qtn[itmute]);

             /* apply mute */
             for (it=0; it<itmute; ++it)  qtn[it] = 0.0;

             /* apply linear ramp */
             for (it=itmute; it < (itmute+ m_lmute) && it < nt; ++it)
                 qtn[it] *= (float)(it-itmute+1)/(float)m_lmute;

             /* if specified, scale by the NMO stretch factor */
             if (m_sscale)
                 for (it=itmute; it<nt; ++it)
                     qtn[it] *= atn[it];

             /* copy NMO corrected trace to output trace */
             memcpy( (void *) outdata[ix],  (const void *) qtn, nt*sizeof(float));

         /* else inverse nmo */
         } else {

             /* do inverse nmo via 8-point sinc interpolation */
             Sdp2dUtils::ints8r(nt, 1.0, ft/dt, indata[ix], 0.0, 0.0, nt-itmute, &tnt[itmute], &qt[itmute]);

             /* apply mute */
             for (it=0; it<itmute; ++it) qt[it] = 0.0;

             /* if specified, undo NMO stretch factor scaling */
             if (m_sscale)
                 for (it=itmute; it<nt; ++it) qt[it] *= at[it];

             /* copy inverse NMO corrected trace to output trace */
             memcpy( (void *) outdata[ix], (const void *) qt, nt*sizeof(float));
         }
    }

    delete [] ttn;
    delete [] atn;
    delete [] tnt;
    delete [] at;
    delete [] qtn;
    delete [] qt;
}
