#include "mytablewidget.h"
#include <QPainter>
#include <cmath>

void MyTableWidget::paintEvent(QPaintEvent *event)
{
    // 1) Draw the normal table
    QTableWidget::paintEvent(event);

    // 2) Overlay each arrow
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(Qt::green, 2);
    painter.setPen(pen);
    painter.setBrush(Qt::green);

    constexpr qreal arrowSize = 10.0;

    for (const Arrow& a : m_arrows) {
        auto itemFrom = item(a.from.first, a.from.second);
        auto itemTo   = item(a.to.first,   a.to.second);
        if (!itemFrom || !itemTo) continue;

        // get center points
        QPointF p1 = visualItemRect(itemFrom).center();
        QPointF p2 = visualItemRect(itemTo  ).center();

        // draw shaft
        painter.drawLine(p1, p2);

        // compute raw deltas
        qreal dx = p2.x() - p1.x();
        qreal dy = p2.y() - p1.y();
        qreal angle = std::atan2(dy, dx);

        // build the two arrowhead wings *backwards* from p2
        QPointF wing1 = p2 - QPointF(std::cos(angle + M_PI/6) * arrowSize,
                                     std::sin(angle + M_PI/6) * arrowSize);
        QPointF wing2 = p2 - QPointF(std::cos(angle - M_PI/6) * arrowSize,
                                     std::sin(angle - M_PI/6) * arrowSize);

        // draw the head
        QPolygonF head;
        head << p2 << wing1 << wing2;
        painter.drawPolygon(head);
    }
}
