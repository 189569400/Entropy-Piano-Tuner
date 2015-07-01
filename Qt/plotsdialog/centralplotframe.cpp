#include "centralplotframe.h"
#include <QGestureEvent>
#include <QPinchGesture>
#include <QApplication>
#include <cmath>
#include "qwt_plot_panner.h"
#include "qwt_plot_magnifier.h"
#include "qwt_plot_zoomer.h"
#include "core/system/log.h"
#include "keyindexscaledraw.h"
#include "keyindexscaleengine.h"

CentralPlotFrame::CentralPlotFrame(int numberOfKeys, int keynumberOfA) :
    mNumberOfKeys(numberOfKeys),
    mKeynumberOfA(keynumberOfA)
{
    Q_UNUSED(keynumberOfA);

    setAxisScaleEngine(xBottom, new KeyIndexScaleEngine(numberOfKeys));

    KeyIndexScaleDraw *xScaleDraw = new KeyIndexScaleDraw;
    setAxisScaleDraw(xBottom, xScaleDraw);
    QObject::connect(this, SIGNAL(keyWidthChanged(double)), xScaleDraw, SLOT(setKeyWidth(double)));

    // panning with the left mouse button
    QwtPlotPanner *panner = new QwtPlotPanner(canvas());
    panner->setMouseButton(Qt::LeftButton);
    QObject::connect(this, SIGNAL(moveCanvas(int,int)), panner, SLOT(moveCanvas(int,int)));

    // zoom in/out with the wheel
    ( void ) new QwtPlotMagnifier( canvas() );

    // rect zoomer and used manually for touch
    mPlotZoomer = new QwtPlotZoomer(canvas());
    mPlotZoomer->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton, Qt::ShiftModifier);


    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);
}

double CentralPlotFrame::currentTickDistanceInPixel() const {
    return transform(QwtPlot::xBottom, 3) - transform(QwtPlot::xBottom, 2);
}

void CentralPlotFrame::applyTouchTransform() {
    bool isLogX = dynamic_cast<QwtLogScaleEngine*>(axisScaleEngine(xBottom)) != nullptr;
    bool isLogY = dynamic_cast<QwtLogScaleEngine*>(axisScaleEngine(yLeft)) != nullptr;


    QStack< QRectF > zoomStack = mPlotZoomer->zoomStack();
    QRectF newRect;

    auto invTransform = [this](const QPointF &p) {
        return QPointF(this->invTransform(xBottom, p.x()),
                       this->invTransform(yLeft, p.y()));
    };

    const QwtInterval xInterval(axisInterval(xBottom));
    const QwtInterval yInterval(axisInterval(yLeft));
    QPointF topLeft(xInterval.minValue(), yInterval.maxValue());
    QPointF botRight(xInterval.maxValue(), yInterval.minValue());

    const QList<QTouchEvent::TouchPoint> &points(mTouchPoints);
    if (points.size() == 1) {
        const QTouchEvent::TouchPoint &p(points.first());
        const QPointF pPrev(invTransform(p.lastPos()));
        const QPointF pCurr(invTransform(p.pos()));
        const QPointF d = pCurr - pPrev;
        newRect = QRectF(topLeft, botRight);
        if (dynamic_cast<QwtLogScaleEngine*>(axisScaleEngine(yLeft))) {
            newRect.translate(-d.x(), 0);
            newRect = newRect.normalized();

            double logFac = pCurr.y() / pPrev.y();
            newRect.setBottom(newRect.bottom() / logFac);
            newRect.setTop(newRect.top() / logFac);

        } else {
            newRect.translate(-d);
        }
    } else if (points.size() == 2) {
        const QTouchEvent::TouchPoint &p1(points.first());
        const QTouchEvent::TouchPoint &p2(points.last());

        const QPointF p1Last = invTransform(p1.lastPos());
        const QPointF p2Last = invTransform(p2.lastPos());
        const QPointF p1Curr = invTransform(p1.pos());
        const QPointF p2Curr = invTransform(p2.pos());

        // auto linearTransform = [](qreal x, qreal m, qreal t) {return m * x + t;};
        auto invLinearTransform = [](qreal x, qreal m, qreal t) {return (x - t) / m;};

        // auto logTransform = [](qreal x, qreal m, qreal t) {return std::pow(x, m) * t;};
        auto invLogTransform = [](qreal x, qreal m, qreal t) {return std::pow(x / t, 1/m);};

        auto linM = [](qreal p1, qreal c1, qreal p2, qreal c2) {return (c1 - c2) / (p1 - p2);};
        auto logM = [](qreal p1, qreal c1, qreal p2, qreal c2) {return log(c1 / c2) / log(p1 / p2);};

        auto linT = [](qreal p1, qreal c1, qreal m) {return c1 - m * p1;};
        auto logT = [](qreal p1, qreal c1, qreal m) {return c1 / std::pow(p1, m);};

        auto computeBounds = [&](int axis, bool log, qreal &top, qreal &bot) {
            auto c = &QPointF::y;
            if (axis == xBottom) {c = &QPointF::x;}

            const qreal p1 = (p1Last.*c)();
            const qreal p2 = (p2Last.*c)();
            const qreal c1 = (p1Curr.*c)();
            const qreal c2 = (p2Curr.*c)();

            if (log) {
                const qreal m = logM(p1, c1, p2, c2);
                const qreal t = logT(p1, c1, m);
                top = invLogTransform(top, m, t);
                bot =invLogTransform(bot, m, t);
            } else {
                const qreal m = linM(p1, c1, p2, c2);
                const qreal t = linT(p1, c1, m);

                top = invLinearTransform(top, m, t);
                bot = invLinearTransform(bot, m, t);
            }
        };

        computeBounds(xBottom, isLogX, topLeft.rx(), botRight.rx());
        computeBounds(yLeft, isLogY, topLeft.ry(), botRight.ry());

        newRect.setTopLeft(topLeft);
        newRect.setBottomRight(botRight);
    }

    if (newRect.isNull() == false) {
        zoomStack << newRect.normalized();
        mPlotZoomer->setZoomStack(zoomStack);
    }

    mTouchPoints.clear();
}

bool CentralPlotFrame::event(QEvent *e) {
    if (dynamic_cast<QTouchEvent*>(e)) {
        return touchEvent(static_cast<QTouchEvent *>(e));
    }
    return QwtPlot::event(e);
}

bool CentralPlotFrame::touchEvent(QTouchEvent *e) {
    QList<QTouchEvent::TouchPoint> points(e->touchPoints());
    for (auto it = points.begin(); it != points.end(); ) {
        if (it->state() == Qt::TouchPointReleased || it->state() == Qt::TouchPointPressed) {
            it = points.erase(it);
        } else {
            it++;
        }
    }
    if (points.size() != mTouchPoints.size()) {
        applyTouchTransform();
        mTouchPoints = points;
    } else {
        for (int i = 0; i < points.size(); ++i) {
            mTouchPoints[i].setPos(points[i].pos());
        }
    }

   return true;
}

void CentralPlotFrame::updateLayout() {
    emit keyWidthChanged(currentTickDistanceInPixel());
    QwtPlot::updateLayout();
}

void CentralPlotFrame::showEvent(QShowEvent *e) {
    emit keyWidthChanged(currentTickDistanceInPixel());
    QwtPlot::showEvent(e);
}

void CentralPlotFrame::resizeEvent(QResizeEvent *e) {
    QwtPlot::resizeEvent(e);
    emit keyWidthChanged(currentTickDistanceInPixel());
}

void CentralPlotFrame::paintEvent(QPaintEvent *e) {
    QwtPlot::paintEvent(e);
}

void CentralPlotFrame::resetView() {
    setAxisAutoScale(QwtPlot::yLeft);
    setAxisScale(QwtPlot::xBottom, 0, mNumberOfKeys, 12);
    replot();

    // set initial zoom rect to current view
    const QwtInterval xInterval(axisInterval(xBottom));
    const QwtInterval yInterval(axisInterval(yLeft));
    QPointF topLeft(xInterval.minValue(), yInterval.maxValue());
    QPointF botRight(xInterval.maxValue(), yInterval.minValue());
    QStack<QRectF> stack;
    QRectF r(topLeft, botRight);
    stack << r.normalized();
    mPlotZoomer->setZoomStack(stack);
}

void CentralPlotFrame::zoomGoFirst() {
    mPlotZoomer->zoom(mPlotZoomer->maxStackDepth());
}

void CentralPlotFrame::zoomGoLast() {
    mPlotZoomer->zoom(-mPlotZoomer->maxStackDepth());
}

void CentralPlotFrame::zoomGoNext() {
    mPlotZoomer->zoom(1);
}

void CentralPlotFrame::zoomGoPrevious() {
    mPlotZoomer->zoom(-1);
}
