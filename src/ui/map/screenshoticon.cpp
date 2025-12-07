//
// Created by Ian Parker on 06/12/2025.
//

#include "screenshoticon.h"

#include <QDesktopServices>

#include "routemap.h"

ScreenshotIcon::ScreenshotIcon(std::string path, UFC::Coordinate coordinate) :
    m_path(path),
    m_coordinate(coordinate)
{
    setFlag(QGV::ItemFlag::Clickable, true);
    QString qpath = QString::fromStdString(path);
    m_image = new QImage(qpath);
    loadImage(*m_image);

    QSizeF size(20, 20);
    setGeometry(QGV::GeoPos(coordinate.latitude, coordinate.longitude), size);
    m_thumbnail = false;
}

void ScreenshotIcon::projOnMouseDoubleClick(const QPointF &projPos)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(m_path)));
}

void ScreenshotIcon::scaleChanged()
{
    auto scale = getMap()->getCamera().scale();
    if (scale < 0.01)
    {
        if (m_thumbnail)
        {
            loadImage(*((RouteMap*)getMap())->getScreenshotIcon());
            QSizeF size(20, 20);
            setGeometry(QGV::GeoPos(m_coordinate.latitude, m_coordinate.longitude), size);
            m_thumbnail = false;
        }
    }
    else if (!m_thumbnail)
    {
        loadImage(*m_image);
        QSizeF size = m_image->size();
        size.scale(100, 100, Qt::AspectRatioMode::KeepAspectRatio);
        setGeometry(QGV::GeoPos(m_coordinate.latitude, m_coordinate.longitude), size);
        m_thumbnail = true;
    }
}
