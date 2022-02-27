#ifndef SD2DCHARTVIEW_H
#define SD2DCHARTVIEW_H

#include <QWidget>

#include <QChart>
#include <QChartView>

QT_BEGIN_NAMESPACE
class QPointF;
class QDialog;
class QStatusBar;
class QWidget;
class QEvent;
class QMouseEvent;
class QKeyEvent;
class QPointF;
class QStatusBar;
QT_END_NAMESPACE

using namespace QtCharts;

class Sdp2dChartView : public QChartView
{
public:
    Sdp2dChartView(QChart *chart, QWidget *mainWindow, QWidget *parent = 0);

protected:
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    bool m_isTouching;
    QPointF m_startP;
    QPointF m_endP;
    QStatusBar* m_statusbar;
    QChart *m_chart;

    //SrcRecCdpDistributionDialog *m_pdlg;
};

#endif // SD2DCHARTVIEW_H
