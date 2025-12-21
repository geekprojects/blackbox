//
// Created by Ian Parker on 21/12/2025.
//

#include "routemarker.h"

#include <QApplication>
#include <QPainter>

RouteMarker::RouteMarker(RoutePoint& point) : m_point(point)
{
    setFlag(QGV::ItemFlag::Clickable, true);

    QFont font(QApplication::font().family());//"Courier New");
    font.setPointSize(15);
    QFontMetrics metrics(font);

    QString text = QString::fromStdString(m_point.name);
    auto rect = metrics.boundingRect(text);
    rect.setWidth(rect.width() * 2);
   //rect.setHeight(rect.height() * 2);

    printf("RouteMarker: %s -> %d, %d: at %f, %f\n", text.toStdString().c_str(), rect.width(), rect.height(),
        m_point.position.latitude(), m_point.position.longitude());

    printf("RouteMarker:  -> size: %d, %d\n", rect.width(), rect.height());

    QImage image(rect.size(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    {
        QPainter painter(&image);
        painter.setFont(font);
        painter.setPen(Qt::black);

        int h = metrics.height() - 2;
        // Draw a triangle
        painter.drawLine(h / 2, 1, h, h);
        painter.drawLine(1, h, h, h);
        painter.drawLine(1, h, h / 2, 1);

        painter.drawText(
            h + 2,
            metrics.height(),
            text);
        /*
        painter.drawText(
          0,
        metrics.height() * 2,
            "Hello");
            */
        painter.end();
    }
    loadImage(image);

    setGeometry(QGV::GeoPos(point.position.latitude(), point.position.longitude()));
}
