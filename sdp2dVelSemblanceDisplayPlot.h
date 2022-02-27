#ifndef SDP2DVELSEMBLANCEDISPLAYPLOT_H
#define SDP2DVELSEMBLANCEDISPLAYPLOT_H

#include "qcustomplot.h"


QT_BEGIN_NAMESPACE
class QString;
class QMouseEvent;
class QStatusBar;
class QMenu;
QT_END_NAMESPACE

class Sdp2dVelSemblanceDisplayArea;

class Sdp2dVelSemblanceDisplayPlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit Sdp2dVelSemblanceDisplayPlot(Sdp2dVelSemblanceDisplayArea *parent = nullptr);
    ~Sdp2dVelSemblanceDisplayPlot();

signals:

private:
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    Q_SLOT void onMenuRequested(const QPoint &pos);

    Sdp2dVelSemblanceDisplayArea* m_parent;
    QStatusBar* m_statusBar;
    QMenu* m_myMenu;

};

#endif // SDP2DVELSEMBLANCEDISPLAYPLOT_H
