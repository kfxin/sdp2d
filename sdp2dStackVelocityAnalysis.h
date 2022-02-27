#ifndef SDP2DPMSTACKVELOCITYANALYSIS_H
#define SDP2DPMSTACKVELOCITYANALYSIS_H

#include <QObject>


class SeismicData2DPreStack;
class Sdp2dQDomDocument;

/* Credits:
 *	CWP, Colorado School of Mines:
 *           Dave Hale (everything except ...)
 *           Bjoern Rommel (... the quartic term)
 *           Valmore Celis, NCCS/NSEL/UCCS/USEL
 *      SINTEF, IKU Petroleumsforskning
 *           Bjoern Rommel (... the power-of-semblance function)
 *
 * Trace header fields accessed:  ns, dt, delrt, offset, cdp
 * Trace header fields modified:  ns, dt, offset, cdp
 */

class Sdp2dStackVelocityAnalysis :public QObject
{    
    Q_OBJECT

public:
    explicit Sdp2dStackVelocityAnalysis(SeismicData2DPreStack* sd2d);
    ~Sdp2dStackVelocityAnalysis();

    bool setupParameters(Sdp2dQDomDocument* para) ;
    float** processCurrentGather(float** indata, int ntr) ;
    void processWholeData(void) ;

    int getNumberOfVelocities(void) const { return m_nv; }
    float getVelocityInterval(void) const { return m_dv;}
    float getFirstVelocity(void) const { return m_fv;}
    float getAnisotropicFactorOne(void) const { return m_anis1; }
    float getAnisotropicFactorTwo(void) const { return m_anis2; }
    float getNMOStretchMuteFactor(void) const { return m_smute; }
    float getSemblancePowerFactor(void) const { return m_pwr; }
    float getSemblanceThreshold(void) const { return m_tau; }

    int getNumberOfDtRatio(void) const { return m_dtratio; }
    int getLengthOfSmooth(void) const { return m_nsmooth; }

    int getTimeSamplesOfSembelance(void) const { return m_ntout; }
    float getTimeSampleRateOfSembelance(void) const { return m_dtout; }

    float getLastVelocity(void) const { return m_fv + m_dv*(m_nv-1); }
    float** getSemblancePointer(void) const { return  m_semblance; }

    bool isInterpolateVelocity(void) const {return m_interp; }

    void setParametersToDom(Sdp2dQDomDocument* domDoc);
    bool getParametersFromDom(Sdp2dQDomDocument* domDoc);

    void applyNmoOnCurrentGather(float** indata, float** outdata, float* vnmo, int ntr);

private:
    void velannConventional(float** indata,  int ntr);
    void velannNCCS(float** indata, int ntr);
    void velannNSEL(float** indata, int ntr);
    void velannUCCS(float** indata, int ntr);
    void velannUSEL(float** indata, int ntr);

private:
    SeismicData2DPreStack* m_sd2d;
    int m_algorithm;
    int m_nv;         //number of velocities
    float m_dv;       //velocity sampling interval
    float m_fv;       //first velocity
    float m_anis1;    //quartic term, numerator of an extended quartic term
    float m_anis2;    //in denominator of an extended quartic term
    float m_smute;    //samples with NMO stretch exceeding smute are zeroed
    float m_pwr;      //semblance value to the power
    float m_tau;
    int m_dtratio;    //ratio of output to input time sampling intervals
    int m_nsmooth;    //length of semblance num and den smoothing window

    int m_lmute;      //length (in samples) of linear ramp for stretch mute
    bool m_sscale;    //=1 to divide output samples by NMO stretch factor
    bool m_invert;    //=1 to perform (approximate) inverse NMO
    bool m_upward;    //=1 to scan upward to find first sample to kill
    bool m_interp;    //=1 to interpolation velocity for NMO

    int m_ntin;
    int m_ntout;
    float m_dtin;
    float m_dtout;

    float m_ft; //time of the first sample

    double* m_offset;

    float** m_semblance;

};

#endif // SDP2DPMSTACKVELOCITYANALYSIS_H
