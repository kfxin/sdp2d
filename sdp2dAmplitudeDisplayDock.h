#ifndef SDP2DAMPLITUDEDISPLAYDIALOG_H
#define SDP2DAMPLITUDEDISPLAYDIALOG_H

#include "qcustomplot.h"
#include <QDockWidget>
#include <QVector>

class SeismicData2DPreStack;
class SeismicDataProcessing2D;

class Sdp2dAmpDisplayQCPGraph : public QCPGraph
{
    Q_OBJECT
public:
    explicit Sdp2dAmpDisplayQCPGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);
    virtual ~Sdp2dAmpDisplayQCPGraph();

    void setColorScaleDataForScatters(double* strength) { mScatterScale = strength; }
    void setTraceFlagForScatters(short* flag) { mTraceFlag = flag; }

protected:
    void drawScatter(QCPPainter *painter, const QVector<QPointF> &scatters);
    void draw(QCPPainter *painter);

private:
    double* mScatterScale;
    short* mTraceFlag;

};

class Sdp2dAmplitudeDisplayDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit Sdp2dAmplitudeDisplayDock(SeismicData2DPreStack* p, QString sd2dName, QWidget *mainWindow = nullptr);
    ~Sdp2dAmplitudeDisplayDock();

    bool isAmplitudeDockHide(void) const { return m_isHide; }
    void setAmplitudeDockShow() { m_isHide = false;}

    void setDataForCurrentTracePoint(int traceIdx);
    void hideTraceLocationPoint();
    int getDisplayAttType(void) const { return m_attType; }
    void generateMuteScatterValues(int muteType);
    double* getMuteValuesPointer(void) { return m_mutetime.data();}
    void replot(void);

signals:

private:
    void createLayout(void);
    void attTypeChanged(QString text);
    void xAxisTypeChanged(QString text);
    void loadAttributeInformation(void);
    void createPlotArea(QLayout *p);
    void drawGraphy(void);
    void zoomFitBtnClicked();
    void amplitudeColormapChanged(QString text);

    void displayCurrentPointLocation(void);

    void findDataRangeWithClipPercentage(QVector<double> amp,  QCPRange& range);
    void amplitudeClipPercentageChanged(QString text);
    void amplitudeMarkerScaleChanged(QString text);

    void onMouseMove(QMouseEvent *event);
    void onMouseDoubleClick(QMouseEvent *event);
    void onCustomContextMenuRequested(const QPoint &pos);
    void showTraceInShotGather(void);
    void showTraceInCDPsGather(void);
    void showTraceInRecvGather(void);
    void showTraceInOffsetGather(void);
    void offsetSearchTolerenceChanged(const QCPRange &newRange);

    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);


    SeismicDataProcessing2D* m_mainWindow;
    SeismicData2DPreStack* m_sd2d;
    QMenu* m_myMenu;
    QComboBox* m_attTypeCBox;
    QComboBox* m_xAxisCBox;

    Sdp2dAmpDisplayQCPGraph* m_attScatters;
    QCPGraph* m_currentPoint;

    QCustomPlot* m_customPlot;
    QCPColorScale* m_colorScale;
    QStatusBar* m_statusbar;

    QCPRange m_aveAmpRange;
    QCPRange m_absAmpRange;
    QCPRange m_rmsAmpRange;
    QCPRange m_muteRange;

    bool m_isHide;
    float m_clipPercentage;
    double m_markerSize;

    int m_attType;
    int m_xaxisType;

    int m_trIdx;
    int m_trSeq;

    QVector<double> m_aveamp;
    QVector<double> m_absamp;
    QVector<double> m_rmsamp;
    QVector<double> m_shotidx;
    QVector<double> m_cdpsidx;
    QVector<double> m_recvidx;
    QVector<double> m_offsval;
    QVector<double> m_mutetime;

    QVector<double> m_pointKey;
    QVector<double> m_pointVal;

};

#endif // SDP2DAMPLITUDEDISPLAYDIALOG_H
