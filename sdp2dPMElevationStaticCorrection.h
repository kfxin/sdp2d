#ifndef SDP2DPMELEVATIONSTATICCORRECTION_H
#define SDP2DPMELEVATIONSTATICCORRECTION_H

#include "sdp2dProcessModule.h"

class Sdp2dPMElevationStaticCorrection : public Sdp2dProcessModule
{
public:
    explicit Sdp2dPMElevationStaticCorrection(SeismicDataProcessing2D *parent);
    ~Sdp2dPMElevationStaticCorrection();

    bool setupParameters(Sdp2dQDomDocument* para) override;
    void processCurrentGather(float** indata, float** outdata, int ntr) override;
    void processWholeData(void) override;

private:
    float m_datum;
    float m_wevel;
    float m_swevel;
    float m_sign;
    bool  m_useHeader;
    float m_btmOfWL;

};

#endif // SDP2DPMELEVATIONSTATICCORRECTION_H
