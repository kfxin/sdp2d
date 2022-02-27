#ifndef SDP2DPMSUGAIN_H
#define SDP2DPMSUGAIN_H

#include "sdp2dProcessModule.h"


class Sdp2dPMSuGain : public Sdp2dProcessModule
{
public:
    explicit Sdp2dPMSuGain(SeismicDataProcessing2D *parent);
    ~Sdp2dPMSuGain();

    bool setupParameters(Sdp2dQDomDocument* para) override;
    void processCurrentGather(float** indata, float** outdata, int ntr) override;

    void processWholeData(void) override;
private:
    void gain(float *data, int nt);
    void do_tpow(float *data, float tpow, float vred,   float tmin,  float dt, int nt);
    void do_epow(float *data, int nt);
    void do_trap(float *data,  float trap,  int nt);
    void do_clip(float *data,  float clip,  int nt);
    void do_nclip(float *data,  float nclip,  int nt);
    void do_pclip(float *data,  float pclip,  int nt);
    void do_qclip(float *data, float qclip, int nt);
    void do_qbal(float *data, float qclip, int nt);
    void do_agc(float *data, int iwagc, int nt);
    void do_gagc(float *data, int iwagc, int nt);
    float quant(float *a, int k, int n);

private:
    bool m_panel;    //=1  gain whole data set (vs. trace by trace)
    bool m_agc;      //flag; do automatic gain control
    bool m_gagc;     //flag; do automatic gain control with gaussian taper
    bool m_qbal;     //flag; balance traces by qclip and scale
    bool m_pbal;     //flag; bal traces by dividing by rms value
    bool m_mbal;     //flag; bal traces by subtracting the mean
    bool m_maxbal;   //flag; balance traces by subtracting the max

    float m_tpow;    //multiply data by t^tpow
    float m_epow;    //multiply data by exp(epow*t)
    float m_etpow;   //multiply data by exp(epow*t^etpow)
    float m_gpow;    //take signed gpowth power of scaled data

    float m_wagc;    //agc window in seconds (use if agc=1 or gagc=1)
    float m_trap;    //zero any value whose magnitude exceeds trapval
    float m_clip;    //clip any value whose magnitude exceeds clipval
    float m_pclip;   //clip any value greater than clipval
    float m_nclip;   //clip any value less than  clipval
    float m_qclip;   //clip by quantile on absolute values on trace

    float m_scale;   //multiply data by overall scale factor
    float m_norm;    //divide data by overall scale factor
    float m_bias;    //bias data by adding an overall bias value
    float m_vred;

    int m_iwagc;     // time window length in number of samples

    QVector<float> m_tmpData;
    QVector<float> m_tpowfac;
    QVector<float> m_epowfac;
    QVector<float> m_etpowfac;
    QVector<float> m_gaussianWin;
    QVector<float> m_d2;
    QVector<float> m_s;

};

#endif // SDP2DPMSUGAIN_H
