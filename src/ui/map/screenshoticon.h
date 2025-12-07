//
// Created by Ian Parker on 06/12/2025.
//

#ifndef BLACKBOX_SCREENSHOTICON_H
#define BLACKBOX_SCREENSHOTICON_H
#include <QGeoView/Raster/QGVIcon.h>
#include <ufc/geoutils.h>


class ScreenshotIcon : public QGVIcon
{
    QImage* m_image;
    std::string m_path;
    UFC::Coordinate m_coordinate;

    bool m_thumbnail = false;

 public:
    ScreenshotIcon(std::string path, UFC::Coordinate coordinate);

    void projOnMouseDoubleClick(const QPointF &projPos) override;

    void scaleChanged();
};


#endif //BLACKBOX_SCREENSHOTICON_H
