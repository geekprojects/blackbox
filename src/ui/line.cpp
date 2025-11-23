//
// Created by Ian Parker on 13/11/2025.
//

#include "line.h"

#include <QBrush>
#include <QPainter>
#include <QPen>

Line::Line(const QGV::GeoPos& start, const QGV::GeoPos& end, QColor color)
    : mStart(start),
      mEnd(end),
      mColor(color)
{
}

void Line::set(const QGV::GeoPos& start, const QGV::GeoPos& end)
{
    mStart = start;
    mEnd = end;

    // Geo coordinates need to be converted manually again to projection
    onProjection(getMap());

    // Now we can inform QGV about changes for this
    resetBoundary();
    refresh();
}

QGV::GeoRect Line::getRect() const
{
    return QGV::GeoRect(mStart, mEnd);
}

void Line::onProjection(QGVMap* geoMap)
{
    QGVDrawItem::onProjection(geoMap);
    mStartPos = geoMap->getProjection()->geoToProj(mStart);
    mEndPos = geoMap->getProjection()->geoToProj(mEnd);
}

QPainterPath Line::projShape() const
{
    QPainterPath path;
    path.moveTo(mStartPos);
    path.lineTo(mEndPos);
    return path;
}

void Line::projPaint(QPainter* painter)
{
    QPen pen = QPen(QBrush(Qt::red), 5);

    // Custom item highlight indicator
    if (isFlag(QGV::ItemFlag::Highlighted) && isFlag(QGV::ItemFlag::HighlightCustom)) {
        // We will use pen with bigger width
        pen = QPen(QBrush(Qt::black), 5);
    }

    pen.setCosmetic(true);
    painter->setPen(pen);
    painter->setBrush(QBrush(mColor));
    painter->drawLine(mStartPos, mEndPos);

    // Custom item select indicator
    /*
    if (isSelected() && isFlag(QGV::ItemFlag::SelectCustom)) {
        // We will draw additional rect around our item
        painter->drawLine(mProjRect.topLeft(), mProjRect.bottomRight());
        painter->drawLine(mProjRect.topRight(), mProjRect.bottomLeft());
    }
    */
}

QPointF Line::projAnchor() const
{
    // This method is optional (needed flag is QGV::ItemFlag::Transformed).
    // In this case we will use center of item as base

    return QRectF(mStartPos, mEndPos).center();
}

QTransform Line::projTransform() const
{
    // This method is optional (needed flag is QGV::ItemFlag::Transformed).
    // Custom transformation for item.
    // In this case we rotate item by 45 degree.

    return QGV::createTransfromAzimuth(projAnchor(), 45);
}

QString Line::projTooltip(const QPointF& projPos) const
{
    // This method is optional (when empty return then no tooltip).
    // Text for mouse tool tip.

    auto geo = getMap()->getProjection()->projToGeo(projPos);

    return "Line with color " + mColor.name() + "\nPosition " + geo.latToString() + " " + geo.lonToString();
}

void Line::projOnMouseClick(const QPointF& projPos)
{
    // This method is optional (needed flag is QGV::ItemFlag::Clickable).
    // Custom reaction to item single mouse click.
    // To avoid collision with item selection this code applies only if item selection disabled.
    // In this case we change opacity for item.

    if (!isSelectable()) {
        if (getOpacity() <= 0.5)
            setOpacity(1.0);
        else
            setOpacity(0.5);

        qInfo() << "single click" << projPos;
    } else {
        setOpacity(1.0);
    }
}

void Line::projOnMouseDoubleClick(const QPointF& projPos)
{
    // This method is optional (needed flag is QGV::ItemFlag::Clickable).
    // Custom reaction to item double mouse click.
    // In this case we change color for item.

    const QList<QColor> colors = { Qt::red, Qt::blue, Qt::green, Qt::gray, Qt::cyan, Qt::magenta, Qt::yellow };

    const auto iter =
            std::find_if(colors.begin(), colors.end(), [this](const QColor& color) { return color == mColor; });
    mColor = colors[(iter - colors.begin() + 1) % colors.size()];
    repaint();

    setOpacity(1.0);

    qInfo() << "double click" << projPos;
}

void Line::projOnObjectStartMove(const QPointF& projPos)
{
    // This method is optional (needed flag is QGV::ItemFlag::Movable).
    // Custom reaction to item move start.
    // In this case we only log message.

    qInfo() << "object move started at" << projPos;
}

void Line::projOnObjectMovePos(const QPointF& projPos)
{
    // This method is optional (needed flag is QGV::ItemFlag::Movable).
    // Custom reaction to mouse pos change when item move is started.
    // In this case actually changing location of object.
/*
    auto newRect = mProjRect;
    newRect.moveCenter(projPos);

    setRect(getMap()->getProjection()->projToGeo(newRect));

    qInfo() << "object moved" << mGeoRect;
    */
}

void Line::projOnObjectStopMove(const QPointF& projPos)
{
    // This method is optional (needed flag is QGV::ItemFlag::Movable).
    // Custom reaction to item move finished.
    // In this case we only log message.

    qInfo() << "object move stopped" << projPos;
}
