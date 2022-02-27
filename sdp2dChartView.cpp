#include "sdp2dChartView.h"
#include "sdp2dMapDiaplayDock.h"
#include "seismicdataprocessing2d.h"

#include <algorithm>
#include <cfloat>
#include <climits>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>


#include <QEvent>
#include <QMouseEvent>
#include <QStatusBar>

using namespace std;

Sdp2dChartView::Sdp2dChartView(QChart *chart, QWidget *mainWindow, QWidget *parent) :
    QChartView(chart, parent),
    m_isTouching(false)
{
    m_chart = chart;
    SeismicDataProcessing2D* p = dynamic_cast<SeismicDataProcessing2D*>(mainWindow);
    m_statusbar = p->getStatusbarPointer();
    setRubberBand(QChartView::RectangleRubberBand);
}

bool Sdp2dChartView::viewportEvent(QEvent *event)
{

    if (event->type() == QEvent::TouchBegin) {
        // By default touch events are converted to mouse events. So
        // after this event we will get a mouse event also but we want
        // to handle touch events as gestures only. So we need this safeguard
        // to block mouse events that are actually generated from touch.
        m_isTouching = true;

        // Turn off animations when handling gestures they
        // will only slow us down.
        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void Sdp2dChartView::mousePressEvent(QMouseEvent *event)
{
    if (m_isTouching)  return;
    m_startP = event->localPos();
    //QChartView::mousePressEvent(event);
}

void Sdp2dChartView::mouseMoveEvent(QMouseEvent *event)
{

    const QPointF pos = event->localPos();
    int x1 = this->chart()->plotArea().topLeft().x();
    int x2 = this->chart()->plotArea().bottomRight().x();
    int y1 = this->chart()->plotArea().topLeft().y();
    int y2 = this->chart()->plotArea().bottomRight().y();
    if(pos.x() < x1 || pos.x() > x2 || pos.y() < y1 || pos.y() > y2 ) {
        m_statusbar->clearMessage();
        return;
    }
    QPointF val = m_chart->mapToValue(pos);
    m_statusbar->showMessage(QString("X: %1, Y: %2").arg(val.x()).arg(val.y()));
    /*
    cout << "pos.x = " << pos.x() << "pos.y = " << pos.y()
         << " width=" << this->size().width() << " height=" << this->size().height()
         << " rwidth=" << this->chart()->plotArea().width() << " rheight=" << this->chart()->plotArea().height()
         << " px=" <<this->chart()->plotArea().x() << " py=" << this->chart()->plotArea().y()
         << endl;
    */
    if (m_isTouching)  return;
    QChartView::mouseMoveEvent(event);
}

void Sdp2dChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isTouching)  m_isTouching = false;
    m_endP = event->localPos();

    double move_x = m_endP.x() - m_startP.x();
    double move_y = m_endP.y() - m_startP.y();
    chart()->scroll(-move_x, move_y);

    //double width  = this->chart()->plotArea().width();
    //double height = this->chart()->plotArea().height();


    //m_pdlg
    // Because we disabled animations when touch event was detected
    // we must put them back on.
    //chart()->setAnimationOptions(QChart::SeriesAnimations);

    //QChartView::mouseReleaseEvent(event);

}

//![1]
void Sdp2dChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        //cout << "Zoom in" << endl;
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        //cout << "Zoom out" << endl;
        chart()->zoomOut();
        break;
//![1]
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}
