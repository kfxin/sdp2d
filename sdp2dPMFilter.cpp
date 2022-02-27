#include "sdp2dPMFilter.h"
#include "seismicdataprocessing2d.h"
#include "sdp2dQDomDocument.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "sdp2dUtils.h"

#include <complex>
#include <fftw3.h>
#include <iostream>

using namespace std;

Sdp2dPMFilter::Sdp2dPMFilter(SeismicDataProcessing2D *parent) : Sdp2dProcessModule(parent)
{
    m_moduleName = QString("Filter");
}

Sdp2dPMFilter::~Sdp2dPMFilter()
{

}

bool Sdp2dPMFilter::setupParameters(Sdp2dQDomDocument* para)
{
    //cout << "setupParameters for : " << para->getModuleName().toStdString().c_str() <<  endl;
    QString filtTypeStr = para->getOptionValue("FilterType");
    m_fileterType = 1;
    if(filtTypeStr.compare("BandPass") != 0) m_fileterType = 2;

    m_flowcut = para->getParameterInOption(QString("flowcut"), filtTypeStr).toFloat();
    m_flowpass = para->getParameterInOption(QString("flowpass"), filtTypeStr).toFloat();
    m_fhighpass = para->getParameterInOption(QString("fhighpass"), filtTypeStr).toFloat();
    m_fhighcut = para->getParameterInOption(QString("fhighcut"), filtTypeStr).toFloat();

    return true;
}

void Sdp2dPMFilter::processCurrentGather(float** indata, float** outdata, int ntr)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    int nt = sd2d->getSamplesPerTraces();
    float dt = float(sd2d->getTimeSampleRateInUs())/1000000.0;
    int nf = int(nt/2)+1;
    float maxfreq = 0.5/dt;
    float df = maxfreq/(nf-1.0);

    float* fftdf  = new float [nt];
    fftwf_complex* fftdc  = fftwf_alloc_complex(nf);
    fftwf_plan fftwfp = fftwf_plan_dft_r2c_1d(nt, fftdf, fftdc, FFTW_DESTROY_INPUT|FFTW_ESTIMATE);
    fftwf_plan fftwbp = fftwf_plan_dft_c2r_1d(nt, fftdc, fftdf, FFTW_DESTROY_INPUT|FFTW_ESTIMATE);

    int f1 = int(m_flowcut   / df + 0.5);
    int f2 = int(m_flowpass  / df + 0.5);
    int f3 = int(m_fhighpass / df + 0.5);
    int f4 = int(m_fhighcut  / df + 0.5);

    if(m_fileterType != 1){
        f1 = int(m_flowpass  / df + 0.5);
        f2 = int(m_flowcut   / df + 0.5);
        f3 = int(m_fhighcut  / df + 0.5);
        f4 = int(m_fhighpass / df + 0.5);
    }
    if(f3 > nf) f3 = nf;
    if(f4 > nf) f4 = nf;

    float flft = f2 - f1;
    float frht = f4 - f3;
    if(f2 != f1) flft = 1.0/flft;
    if(f3 != f4) frht = 1.0/frht;

    float fftwfactor = 1.0/float(nt);

    for(int ix=0; ix< ntr; ix++){
        memcpy((void*) fftdf, (void*)indata[ix], sizeof(float)*nt);
        memset((void*) fftdc, 0, sizeof(fftwf_complex)*nf);

        fftwf_execute(fftwfp);

        if(m_fileterType == 1){
            for(int ifreq=0; ifreq<f1; ifreq++){
                fftdc[ifreq][0] = 0;
                fftdc[ifreq][1] = 0;
            }

            for(int ifreq=f1; ifreq<f2; ifreq++){
                float factor = (ifreq - f1)*flft;
                fftdc[ifreq][0] *= factor;
                fftdc[ifreq][1] *= factor;
            }

            for(int ifreq=f3; ifreq<f4; ifreq++){
                float factor = (f4 - ifreq)*frht;
                fftdc[ifreq][0] *= factor;
                fftdc[ifreq][1] *= factor;
            }

            for(int ifreq=f4; ifreq<nf; ifreq++){
                fftdc[ifreq][0] = 0;
                fftdc[ifreq][1] = 0;
            }
        } else {
            for(int ifreq=f1; ifreq<f2; ifreq++){
                float factor = (f2 - ifreq)*flft;
                fftdc[ifreq][0] *= factor;
                fftdc[ifreq][1] *= factor;
            }

            for(int ifreq=f2; ifreq<f3; ifreq++){
                fftdc[ifreq][0] = 0.0;
                fftdc[ifreq][1] = 0.0;
            }

            for(int ifreq=f3; ifreq<f4; ifreq++){
                float factor = (ifreq - f3)*frht;
                fftdc[ifreq][0] *= factor;
                fftdc[ifreq][1] *= factor;
            }
        }
        fftwf_execute(fftwbp);

        for(int it=0; it<nt; it++) fftdf[it] *= fftwfactor;

        memcpy((void*)outdata[ix], (void*)fftdf, sizeof(float)*nt);
    }

    fftwf_destroy_plan(fftwfp);
    fftwf_destroy_plan(fftwbp);
    fftwf_free(fftdc);
    delete [] fftdf;
}

void Sdp2dPMFilter::processWholeData(void)
{

}
