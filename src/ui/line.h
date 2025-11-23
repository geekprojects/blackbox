//
// Created by Ian Parker on 13/11/2025.
//

#ifndef BLACKBOX_LINE_H
#define BLACKBOX_LINE_H

#include <QGeoView/QGVDrawItem.h>

#include <QBrush>
#include <QPen>

class Line :  public QGVDrawItem
{

private:
    void onProjection(QGVMap* geoMap) override;
    QPainterPath projShape() const override;
    void projPaint(QPainter* painter) override;
    QPointF projAnchor() const override;
    QTransform projTransform() const override;
    QString projTooltip(const QPointF& projPos) const override;
    void projOnMouseClick(const QPointF& projPos) override;
    void projOnMouseDoubleClick(const QPointF& projPos) override;
    void projOnObjectStartMove(const QPointF& projPos) override;
    void projOnObjectMovePos(const QPointF& projPos) override;
    void projOnObjectStopMove(const QPointF& projPos) override;

private:
    QGV::GeoPos mStart;
    QGV::GeoPos mEnd;
    QPointF mStartPos;
    QPointF mEndPos;
    QColor mColor;

public:
    explicit Line(const QGV::GeoPos& start, const QGV::GeoPos& end, QColor color);

    void set(const QGV::GeoPos& start, const QGV::GeoPos& end);

    QGV::GeoRect getRect() const;
};


#endif //BLACKBOX_LINE_H
