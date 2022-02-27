#ifndef SD2DMAPDIAPLAYDOCKWIDGET_H
#define SD2DMAPDIAPLAYDOCKWIDGET_H

#include <QDockWidget>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QStatusBar;
QT_END_NAMESPACE

class SeismicData2D;
class SeismicDataProcessing2D;


#include "sdp2dChartView.h"
#include <QScatterSeries>

class Sdp2dMapDiaplayDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit Sdp2dMapDiaplayDockWidget(SeismicData2D* p, QWidget *parent = nullptr);
    ~Sdp2dMapDiaplayDockWidget();

    void displayPreStackMap(int gatherType, int groupIdx);
    void showLocsOfSelectedGroup(int groupIdx);
    QStatusBar* getStatusBarPointer(void) const { return m_statusbar; }

    bool isShow(void) const { return m_isShow; }

private:
    SeismicData2D* m_sd2d;
    SeismicDataProcessing2D* m_mainWindow;
    QStatusBar* m_statusbar;
    QMenu *m_myMenu;

    int m_gatherType;

    Sdp2dChartView *m_chartview;
    QChart* m_chart;

    int m_groupIdx;
    int m_selectedGroupIdx;
    QPointF m_groupXY;
    QPointF m_selectedGroupXY;

    QList<QPointF> m_shotXY;
    QList<QPointF> m_cdpXY;
    QList<QPointF> m_recvXY;

    QList<QPointF> m_subsetXY;

    QScatterSeries* m_groupSeries;
    QScatterSeries* m_subsetSeries;

    QScatterSeries* m_recvSeries;
    QScatterSeries* m_shotSeries;
    QScatterSeries* m_cdpSeries;

    double m_fx;
    double m_lx;
    double m_fy;
    double m_ly;

    bool m_isShow;

    void displayPreStackSrcRecMap(void);
    void displayPreStackRecSrcMap(void);
    void displayPreStackCDPMap(void);
    void loadPreStackMapData(void);
    void displayPreStackGatherLocation(const QPointF &point, bool state);
    void doubleClickedScatterSeries(void);
    void showLocsOfSelectedGroup(const QPointF &point);
    void restoreDisplay(void);
    void onActiveMyMenu(const QPoint &pos);

    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    //void redrawFigure(bool shown);
    //void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event) override;
};

#endif // SD2DMAPDIAPLAYDOCKWIDGET_H
