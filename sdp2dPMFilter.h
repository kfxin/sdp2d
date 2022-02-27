#ifndef SDP2DPMFILTER_H
#define SDP2DPMFILTER_H

#include "sdp2dProcessModule.h"


class Sdp2dPMFilter : public Sdp2dProcessModule
{
public:
    explicit Sdp2dPMFilter(SeismicDataProcessing2D *parent);
    ~Sdp2dPMFilter();

    bool setupParameters(Sdp2dQDomDocument* para) override;
    void processCurrentGather(float** indata, float** outdata, int ntr) override;
    void processWholeData(void) override;

private:
    int m_fileterType;
    float m_flowcut;
    float m_flowpass;
    float m_fhighpass;
    float m_fhighcut;
};

#endif // SDP2DPMFILTER_H
