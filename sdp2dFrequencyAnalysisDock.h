#ifndef SDP2DFREQUENCYANALYSISDOCK_H
#define SDP2DFREQUENCYANALYSISDOCK_H

#include <QDockWidget>
#include <complex>
#include <fftw3.h>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QStatusBar;
class QMenu;
class QAction;
QT_END_NAMESPACE

class SeismicDataProcessing2D;
class SeismicData2D;
class Sdp2dQDomDocument;
class QCustomPlot;

enum FrequencyCurveType{
    Amplitude=1, Phase=2, Real=3, Image=4
};

class Sdp2dFrequencyAnalysisDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit Sdp2dFrequencyAnalysisDock(SeismicData2D* p, bool visible=false, QWidget *parent = nullptr);
    ~Sdp2dFrequencyAnalysisDock();

    void calculateAndShowSpectrum(int ntraces, float** data, QString& label);
    bool isFrequencyAnaDockHide(void) const { return m_isHide; }
    void setFrequencyAnaDockShow() { m_isHide = false;}

    void setFreqComparisonFlag(bool flag) { m_hasComparison = flag; }
    void setGraphyNameWithIdx(QString legendName, int idx);

    void applyBandPassFilter(float** data, int nx, bool applyFilter=true);

    void setFilterType(int ft) { m_filterType = ft; }
    void setFreqLowCut(float val) { m_flowcut = val; }
    void setFreqLowPass(float val) { m_flowpass = val; }
    void setFreqHighCut(float val) { m_fhighcut = val; }
    void setFreqHighPass(float val) { m_fhighpass = val; }

    int getFilterType(void) const { return m_filterType; }
    float getFreqLowCut(void) const { return m_flowcut; }
    float getFreqLowPass(void) const { return m_flowpass; }
    float getFreqHighCut(void) const { return m_fhighcut; }
    float getFreqHighPass(void) const { return m_fhighpass; }

    void setParametersToDom(Sdp2dQDomDocument* domDoc);
    bool getParametersFromDom(Sdp2dQDomDocument* domDoc);

signals:

private:
    void on_freqAna_customContextMenuRequested(const QPoint &pos);
    void setNormalizeAmplitude(bool checked);
    void setShowAmplitude(bool checked);
    void setShowPhase(bool checked);
    void setShowReal(bool checked);
    void setShowImage(bool checked);
    void showFiltedFrequencyCurve(void);

    void showFrequencyCurve(QString& label);
    void showFrequencyCurve();

    void onMouseMove(QMouseEvent *event);
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);

private:
    SeismicDataProcessing2D* m_mainWindow;
    SeismicData2D* m_sd2d;
    QStatusBar* m_statusbar;
    QCustomPlot *m_customPlot;
    QMenu* m_myMenu;
    QAction* m_mitemNormalize;
    QAction* m_mitemShowAm;
    QAction* m_mitemShowPh;
    QAction* m_mitemShowRe;
    QAction* m_mitemShowIm;

    fftwf_plan m_fftwfp;
    fftwf_plan m_fftwbp;
    float* m_fftdf;
    fftwf_complex* m_fftdc;
    int m_nt;
    int m_nf;
    float m_dt;
    float m_df;
    float m_maxfreq;
    int m_curveIdx;
    QVector<double> m_fftam;
    QVector<double> m_fftan;
    QVector<double> m_fftph;
    QVector<double> m_fftre;
    QVector<double> m_fftim;
    QVector<double> m_freqVal;

    QVector<double> m_fftam2;
    QVector<double> m_fftan2;
    QVector<double> m_fftph2;
    QVector<double> m_fftre2;
    QVector<double> m_fftim2;

    bool m_isHide;
    bool m_hasComparison;

    bool m_normalizeAmp;
    int m_noCurveType;
    int m_curveType;

    float m_ammin;
    float m_phmin;
    float m_remin;
    float m_immin;

    float m_ammax;
    float m_phmax;
    float m_remax;
    float m_immax;

    int m_filterType;
    float m_flowcut;
    float m_flowpass;
    float m_fhighpass;
    float m_fhighcut;
};

#endif // SDP2DFREQUENCYANALYSISDOCK_H
