#include "sdp2dPMSuGain.h"
#include "seismicdataprocessing2d.h"
#include "sdp2dQDomDocument.h"
#include "seismicdata2d.h"
#include "seismicdata2dprestack.h"
#include "sdp2dUtils.h"

#include <iostream>
#include <QMessageBox>
#include <math.h>
#include <float.h>
#include <QtGlobal>

using namespace std;

Sdp2dPMSuGain::Sdp2dPMSuGain(SeismicDataProcessing2D *parent) : Sdp2dProcessModule(parent)
{
    m_moduleName = QString("SuGain");

}

Sdp2dPMSuGain::~Sdp2dPMSuGain()
{

}

bool Sdp2dPMSuGain::setupParameters(Sdp2dQDomDocument* para)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    int nt = sd2d->getSamplesPerTraces();
    float dt = float(sd2d->getTimeSampleRateInUs())/1000000.0;

    //cout << "setupParameters for : " << para->getModuleName().toStdString().c_str() <<  endl;
    m_tpow  = para->getParameterInGroup("tpow").toFloat();
    m_epow  = para->getParameterInGroup("epow").toFloat();
    m_etpow = para->getParameterInGroup("etpow").toFloat();
    m_gpow  = para->getParameterInGroup("gpow").toFloat();

    m_wagc  = para->getParameterInGroup("wagc").toFloat();

    m_trap  = para->getParameterInGroup("trap").toFloat();
    m_clip  = para->getParameterInGroup("clip").toFloat();
    m_pclip = para->getParameterInGroup("pclip").toFloat();
    m_nclip = para->getParameterInGroup("nclip").toFloat();
    m_qclip = para->getParameterInGroup("qclip").toFloat();

    m_scale = para->getParameterInGroup("scale").toFloat();
    m_norm  = para->getParameterInGroup("norm").toFloat();
    m_bias  = para->getParameterInGroup("bias").toFloat();

    m_vred = 0.0;

    m_panel  = false;
    m_agc    = false;
    m_gagc   = false;
    m_qbal   = false;
    m_pbal   = false;
    m_mbal   = false;
    m_maxbal = false;

    if(para->getParameterInGroup("panel").compare("True") == 0)   m_panel  = true;
    if(para->getParameterInGroup("agc").compare("True") == 0)     m_agc    = true;
    if(para->getParameterInGroup("gagc").compare("True") == 0)    m_gagc   = true;
    if(para->getParameterInGroup("qbal").compare("True") == 0)    m_qbal   = true;
    if(para->getParameterInGroup("pbal").compare("True") == 0)    m_pbal   = true;
    if(para->getParameterInGroup("mbal").compare("True") == 0)    m_mbal   = true;
    if(para->getParameterInGroup("maxbal").compare("True") == 0)  m_maxbal = true;

    if (m_trap < 0.0){
        QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("Parameter trap=%1 must be positive!").arg(m_trap), QMessageBox::Ok, NULL);
        message.exec();
        return false;
    }

    if (m_clip < 0.0){
        QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("Parameter clip=%1 must be positive!").arg(m_clip), QMessageBox::Ok, NULL);
        message.exec();
        return false;
    }

    if (m_qclip < 0.0 || m_qclip > 1.0) {
        QMessageBox message(QMessageBox::NoIcon,
                 "Warn", QString("Parameter qclip=%1 must be between 0 and 1!").arg(m_qclip), QMessageBox::Ok, NULL);
        message.exec();
        return false;
    }
    if (m_agc || m_gagc) {
        m_iwagc = NINT(m_wagc/dt);
        if (m_iwagc < 1) {
            QMessageBox message(QMessageBox::NoIcon,
                     "Warn", QString("Parameter wagc=%1 must be positive!").arg(m_wagc), QMessageBox::Ok, NULL);
            message.exec();
            return false;
        }
        if (m_iwagc > nt) {
            QMessageBox message(QMessageBox::NoIcon,
                     "Warn", QString("Parameter wagc=%g too long for trace!").arg(m_wagc), QMessageBox::Ok, NULL);
            message.exec();
            return false;
        }
        m_iwagc >>= 1;  /* windows are symmetric, so work with half */
    }
    if (m_norm)  m_scale /= m_norm;

    return true;
}

void Sdp2dPMSuGain::processCurrentGather(float** indata, float** outdata, int ntr)
{
    SeismicData2D* sd2d = m_mainWindow->getCurrentDataPointer();
    int nt = sd2d->getSamplesPerTraces();
    float dt = float(sd2d->getTimeSampleRateInUs())/1000000.0;
    float tmin = 0.0;

    memcpy((void*)outdata[0],  (void*)indata[0], nt*ntr*sizeof(float));

    int ns = nt;
    if (m_panel) ns = nt*ntr;

    m_tmpData.resize(ns);

    if (m_tpow > 1.0E-5){
        m_tpowfac.resize(ns);
        if (m_panel){
            for(int itr=0; itr<ntr; itr++){
                int idx = itr*nt;
                m_tpowfac[idx] = (tmin < 1.0E-5) ? 0.0 : pow(tmin, m_tpow);
                idx++;
                for (int it = 1; it < nt; it++, idx++){
                    m_tpowfac[idx] = pow(tmin + it*dt, m_tpow);
                }
            }
        } else {
            m_tpowfac[0] = (tmin < 1.0E-5) ? 0.0 : pow(tmin, m_tpow);
            for (int i = 1; i < nt; i++) m_tpowfac[i] = pow(tmin + i*dt, m_tpow);
        }
    }

    if (m_epow > 1.0E-5) {
        m_epowfac.resize(ns);
        m_etpowfac.resize(ns);

        if (m_panel){
            for(int itr=0; itr<ntr; itr++){
                int idx = itr*nt;
                m_etpowfac[idx] = (tmin < 1.0E-5) ? 0.0 : pow(tmin, m_etpow);
                idx++;
                for (int it = 1; it < nt; it++, idx++){
                    m_etpowfac[idx] = pow(tmin + it*dt, m_etpow);
                }
            }
        } else {
            m_etpowfac[0] = (tmin < 1.0E-5) ? 0.0 : pow(tmin, m_etpow);
            for (int i = 1; i < nt; i++)
                m_etpowfac[i] = pow(tmin + i*dt, m_etpow);
        }

        for (int i = 0; i < ns; i++)
            m_epowfac[i] = exp(m_epow * m_etpowfac[i]);
    }

    if (m_gagc){
        /* Allocate and compute Gaussian window weights */
        m_gaussianWin.resize(m_iwagc);
        m_d2.resize(ns);  /* square of input data		 */
        m_s.resize(ns);   /* weighted sum of squares of the data  */
        double u =  3.8090232 / ((double) m_iwagc);
        double usq = u*u;
        for (int i = 1; i < m_iwagc; ++i) {
            m_gaussianWin[i] = exp(-(usq*i*i));
        }
    }

    if (!m_panel) { /* trace by trace */
        for(int itr=0; itr<ntr; itr++){
            gain(outdata[itr], nt);
        }
    } else {        
        gain(outdata[0], nt*ntr);
    }
    m_tmpData.clear();
    m_gaussianWin.clear();
    m_tpowfac.clear();
    m_epowfac.clear();
    m_etpowfac.clear();
    m_d2.clear();
    m_s.clear();
}

void Sdp2dPMSuGain::processWholeData(void)
{

}

/* Multiply by t^tpow */
void Sdp2dPMSuGain::do_tpow(
    float *data,     /* the data			*/
    float tpow,      /* multiply data by t^tpow	*/
    float vred,      /* reducing velocity		*/
    float tmin,      /* first time on record	 */
    float dt,        /* sampling rate in seconds     */
    int nt          /* number of samples	    */
)
{    
    float tred=0;              /* reduced time in seconds	*/

    if ( vred > 0.0 ) {	/* recompute array of tpowfac for each trace */
        //
        //tred = (float)tr.offset / vred;
        //

        if ( tred < 0.0 ) tred *= -1.0;	/* remove sign */
        for (int i = 1; i < nt; ++i)
            m_tpowfac[i] = pow(tmin + tred + i*dt, tpow);
    }

    for (int i = 0; i < nt; ++i)  data[i] *= m_tpowfac[i];
}


/* Exponential deattenuation  with deattenuation factor epow */
/* and with  with deattenuation  power etpow */
void Sdp2dPMSuGain::do_epow(
    float *data,     /* the data			*/    
    int nt           /* number of samples	    */
)
{    
    for (int i = 0; i < nt; ++i)  data[i] *= m_epowfac[i];
}


/* Zero out outliers */
void Sdp2dPMSuGain::do_trap(
    float *data,   /* the data			*/
    float trap,    /* zero if magnitude > trap     */
    int nt	       /* number of samples	    */
)
{
    float *dataptr = data;

    while (nt--) {
        if (abs(*dataptr) > trap) *dataptr = 0.0;
        dataptr++;
    }
}


/* Hard clip outliers */
void Sdp2dPMSuGain::do_clip(
    float *data,   /* the data				*/
    float clip,    /* hard clip if magnitude > clip	*/
    int nt	       /* number of samples		    */
)
{
    float *dataptr = data;
    float mclip = -clip;

    while (nt--) {
        if (*dataptr > clip) {
            *dataptr = clip;
        } else if (*dataptr < mclip) {
            *dataptr = mclip;
        }
        dataptr++;
    }
}



/* Hard clip maxima */
void Sdp2dPMSuGain::do_pclip(
    float *data,    /* the data	  */
    float pclip,    /* hard clip if magnitude > clip  */
    int nt          /* number of samples  */
)
{
    float *dataptr = data;

    while (nt--) {
        if (*dataptr > pclip) {
            *dataptr = pclip;
        }
        dataptr++;
    }
}


/* Hard clip minima */
void Sdp2dPMSuGain::do_nclip(
    float *data,    /* the data				*/
    float nclip,    /* hard clip if magnitude > clip	*/
    int nt	        /* number of samples		    */
)
{
    float *dataptr = data;

    while (nt--) {
        if (*dataptr < nclip) {
            *dataptr = nclip;
        }
        dataptr++;
    }
}


/* Quantile clip on magnitudes of trace values */
void Sdp2dPMSuGain::do_qclip(
    float *data,    /* the data			*/
    float qclip,    /* quantile at which to clip    */
    int nt	        /* number of sample points	*/
)
{
    int i;
    float clip;		    /* ... value of rank[iq]	*/

    int iq = (int) (qclip * nt - 0.5); /* index of qclipth quantile. round, don't truncate */

    /* Clip on value corresponding to qth quantile */
    for (i = 0; i < nt; ++i)  m_tmpData[i] = abs(data[i]);
    clip = quant(m_tmpData.data(), iq, nt);
    do_clip(data, clip, nt);
}


/* Quantile balance */
void Sdp2dPMSuGain::do_qbal(
    float *data,	/* the data			*/
    float qclip,    /* quantile at which to clip    */
    int nt	        /* number of sample points	*/
)
{
    float bal = 0;
    for (int i = 0; i < nt; i++)  {
        if(!qIsFinite(data[i])) {
            cout << " find infinite at " << i << " ";
            data[i] = 0;
        }
        m_tmpData[i] = qAbs(data[i]);
        bal = qMax(bal, m_tmpData[i]);
    }
    //cout << "bal = " << bal << endl;


    //cout << "qclip="<< qclip << " bal=" << (abs(qclip - 1.0) < FLT_EPSILON) << endl;
    if (qAbs(qclip - 1.0) < 0.001) { /* balance by max magnitude on trace */
        if (bal < 1.0E-5) {
            return;
        } else {
            for (int i = 0; i < nt; i++)  data[i] /= bal;
            return;
        }
    }

    int iq = (int) (qclip * nt - 0.5); /* round, don't truncate */

    /* Balance by quantile value (qclip < 1.0) */
    bal = quant(m_tmpData.data(), iq, nt);

    if (abs(bal) < 1.0E-5) {
        return;
    } else {
        for (int i = 0; i < nt; i++)  data[i] /= bal;
        do_clip(data, 1.0, nt);
        return;
    }
}


/* Automatic Gain Control--standard box */
void Sdp2dPMSuGain::do_agc(float *data, int iwagc, int nt)
{
    int i;
    float val;
    float sum;
    int nwin;
    float rms;

    float *agcdata = m_tmpData.data();

    /* compute initial window for first datum */
    sum = 0.0;
    for (i = 0; i < iwagc+1; ++i) {
        val = data[i];
        sum += val*val;
    }
    nwin = 2*iwagc+1;
    rms = sum/nwin;
    agcdata[0] = (rms <= 0.0) ? 0.0 : data[0]/sqrt(rms);

    /* ramping on */
    for (i = 1; i <= iwagc; ++i) {
        val = data[i+iwagc];
        sum += val*val;
        ++nwin;
        rms = sum/nwin;
        agcdata[i] = (rms <= 0.0) ? 0.0 : data[i]/sqrt(rms);
    }

    /* middle range -- full rms window */
    for (i = iwagc + 1; i <= nt-1-iwagc; ++i) {
        val = data[i+iwagc];
        sum += val*val;
        val = data[i-iwagc];
        sum -= val*val; /* rounding could make sum negative! */
        rms = sum/nwin;
        agcdata[i] = (rms <= 0.0) ? 0.0 : data[i]/sqrt(rms);
    }

    /* ramping off */
    for (i = nt - iwagc; i <= nt-1; ++i) {
        val = data[i-iwagc];
        sum -= val*val; /* rounding could make sum negative! */
        --nwin;
        rms = sum/nwin;
        agcdata[i] = (rms <= 0.0) ? 0.0 : data[i]/sqrt(rms);
    }

    /* copy data back into trace */
    memcpy( (void *) data, (const void *) agcdata, nt*sizeof(float));

    return;
}


//#define EPS     3.8090232	/* exp(-EPS*EPS) = 5e-7, "noise" level  */

/* Automatic Gain Control--gaussian taper */
void Sdp2dPMSuGain::do_gagc(float *data, int iwagc, int nt)
{    


    /* Allocate room for agc'd data */
    float* agcdata = m_tmpData.data();
    /* Allocate sum of squares and weighted sum of squares */    
    float *d2 = m_d2.data();
    float *s  = m_s.data();

    /* Agc the trace */
    {

        float val;
        float wtmp;
        float stmp;

        /* Put sum of squares of data in d2 and */
        /* initialize s to d2 to get center point set */
        for (int i = 0; i < nt; ++i) {
            val = data[i];
            d2[i] = val * val;
            s[i] = d2[i];
        }

        /* Compute weighted sum s; use symmetry of Gaussian */
        for (int j = 1; j < iwagc; ++j) {
            wtmp = m_gaussianWin[j];
            for (int i = j; i < nt; ++i)  s[i] += wtmp*d2[i-j];
            int k = nt - j;
            for (int i = 0; i < k; ++i)   s[i] += wtmp*d2[i+j];
        }

        for (int i = 0; i < nt; ++i) {
            stmp = s[i];
            if(abs(stmp) > FLT_EPSILON){
                agcdata[i] = data[i]/sqrt(stmp);
            } else {
                agcdata[i] = 0;
            }
        }

        /* Copy data back into trace */
        memcpy( (void *) data, (const void *) agcdata, nt*sizeof(float));
    }


    return;
}


/*
 * QUANT - find k/n th quantile of a[]
 *
 * Works by reordering a so a[j] < a[k] if j < k.
 *
 * Parameters:
 *    a	 - data
 *    k	 - indicates quantile
 *    n	 - number of points in data
 *
 * This is Hoare's algorithm worked over by SEP (#10, p100) and Brian.
 */

float Sdp2dPMSuGain::quant(float *a, int k, int n)
{
    int i, j;
    int low, hi;
    float ak, aa;

    low = 0; hi = n-1;

    while (low < hi) {
        ak = a[k];
        i = low;
        j = hi;
        do {
            while (a[i] < ak) i++;
            while (a[j] > ak) j--;
            if (i <= j) {
                aa = a[i]; a[i] = a[j]; a[j] = aa;
                i++;
                j--;
            }
        } while (i <= j);

        if (j < k) low = i;

        if (k < i) hi = j;
    }

    return(a[k]);
}


#define CLOSETO(x, y) ((abs((x) - (y)) <= FLT_EPSILON*abs(y))? true:false)
/*
 * GAIN - apply all the various gains
 *
 */
void Sdp2dPMSuGain::gain(float *data, int nt)
{
    float f_two  = 2.0;
    float f_one  = 1.0;
    float f_half = 0.5;    

    if (m_bias) {
        for (int i = 0; i < nt; i++)  data[i] += m_bias ;
    }
    if (m_tpow) {
        //do_tpow(data, m_tpow, m_vred, tmin, dt, nt);
        for (int i = 0; i < nt; ++i)  data[i] *= m_tpowfac[i];
    }
    if (m_epow) {
        for (int i = 0; i < nt; ++i)  data[i] *= m_epowfac[i];
    }
    if (!CLOSETO(m_gpow, f_one)) {
        float val;

        if (CLOSETO(m_gpow, f_half)) {
            for (int i = 0; i < nt; i++) {
                val = data[i];
                data[i] = (val >= 0.0) ? sqrt(val) : -sqrt(-val);
            }
        } else if (CLOSETO(m_gpow, f_two)) {
            for (int i = 0; i < nt; i++) {
                val = data[i];
                data[i] = val * abs(val);
            }
        } else {
            for (int i = 0; i < nt; i++) {
                val = data[i];
                data[i] = (val >= 0.0) ? pow(val, m_gpow) : -pow(-val, m_gpow);
            }
        }
    }
    if (m_agc)		          do_agc(data, m_iwagc, nt);
    if (m_gagc)		          do_gagc(data, m_iwagc, nt);
    if (m_trap > 0.0)	      do_trap(data, m_trap, nt);
    if (m_clip > 0.0)	      do_clip(data, m_clip, nt);
    if (m_pclip > 0.0)        do_pclip(data, m_pclip, nt);
    if (m_nclip < 0.0)        do_nclip(data, m_nclip, nt);
    if (m_qclip < 1.0 && !m_qbal)  do_qclip(data, m_qclip, nt);
    if (m_qbal)		          do_qbal(data, m_qclip, nt);
    if (m_pbal) {
        double val=0;
        double rmsq = 0.0;

        for (int i = 0; i < nt; i++) {
            val = data[i];
            rmsq += val * val;
        }
        rmsq = sqrt(rmsq / nt);

        if (rmsq > 1.0E-5) {
            for (int i = 0; i < nt; i++){
                val = data[i];
                data[i] = val/rmsq;
            }
        }
    }
    if (m_mbal) {
        double mean = 0.0;

        /* mean = SUM (data[i] / nt) */
        for (int i = 0; i < nt; i++) {
            mean += data[i];
        }
        /* compute the mean */
        mean/=nt;

        /* subtract the mean from each sample */
        if (abs(mean) > FLT_EPSILON) {
            for (int i = 0; i < nt; i++)
                data[i] -= mean;
        }
    }

    if (m_maxbal) {
        double max = data[0];

        /* max */
        for (int i = 0; i < nt; i++) {
            if( data[i] > max ) max = data[i];
        }

        /* subtract max */
        for (int i = 0; i < nt; i++) data[i] -= max;
    }


    if (!CLOSETO(m_scale, f_one)) {
        for (int i = 0; i < nt; i++)  data[i] *= m_scale;
    }
}
