#pragma once

#include "Annotation/GraphicsItemBase.hpp"
#include <QGraphicsSimpleTextItem>
#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPolygon>
#include <QRectF>
#include <concepts>
#include <type_traits>

namespace annotation {

class LineItem : public GraphicsItemBase {
public:
  LineItem() = default;

  LineItem(const QLineF &line, const QColor &color,
           QGraphicsItem *parent = nullptr)
      : GraphicsItemBase(color, parent), m_line(line) {}

  explicit LineItem(const Annotation &anno) : GraphicsItemBase(anno) {
    updateAnno(anno);
  }

  void updateAnno(const Annotation &anno) override;

  void setLine(const QLineF &line);

  [[nodiscard]] QRectF boundingRect() const override;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

  [[nodiscard]] QPainterPath shape() const override;

private:
  QLineF m_line{};
};

class RectItem : public GraphicsItemBase {
public:
  RectItem() = default;

  RectItem(const QRectF &rect, const QColor &color,
           QGraphicsItem *parent = nullptr)
      : GraphicsItemBase(color, parent), m_rect(rect) {}

  explicit RectItem(const Annotation &anno) : GraphicsItemBase(anno) {
    updateAnno(anno);
  }

  void updateAnno(const Annotation &anno) override;

  void setRect(const QRectF &rect);

  [[nodiscard]] QRectF boundingRect() const override { return m_rect; }

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

private:
  QRectF m_rect{};
};

class FanItem : public GraphicsItemBase {
public:
  FanItem() = default;

  FanItem(const QRectF &rect, Arc arc, const QColor &color,
          QGraphicsItem *parent = nullptr)
      : GraphicsItemBase(color, parent), m_rect(rect), m_arc(arc) {}

  explicit FanItem(const Annotation &anno) : GraphicsItemBase(anno) {
    updateAnno(anno);
  }

  [[nodiscard]] QPainterPath getFanPainterPath() const;

  [[nodiscard]] QPainterPath shape() const override;

  void updateAnno(const Annotation &anno) override;

  [[nodiscard]] QRectF boundingRect() const override;

  [[nodiscard]] auto arc() const { return m_arc; }
  void setArc(Arc arc);

  [[nodiscard]] auto startAngle() const { return m_arc.startAngle; }
  void setStartAngle(int angle);

  [[nodiscard]] auto spanAngle() const { return m_arc.spanAngle; }
  void setSpanAngle(int angle);

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

private:
  // Pair of angles (each 0-360) that denote a fan shape center at the center of
  // the square image. The fan is drawn clockwise

  // Draws the arc defined by the given rectangle, startAngle and spanAngle.
  // The startAngle and spanAngle must be specified in 1/16th of a degree, i.e.
  // a full circle equals 5760 (16 * 360). Positive values for the angles mean
  // counter-clockwise while negative values mean the clockwise direction. Zero
  // degrees is at the 3 o'clock position.

  QRectF m_rect{};
  Arc m_arc{};
};

class PolygonItem : public GraphicsItemBase {
public:
  PolygonItem() = default;
  explicit PolygonItem(const Annotation &anno) : GraphicsItemBase(anno) {
    updateAnno(anno);
  }

  void updateAnno(const Annotation &anno) override;

  void setPolygon(const QPolygonF &polygon);

  [[nodiscard]] QRectF boundingRect() const override {
    return m_polygon.boundingRect();
  }

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
             QWidget *widget) override;

private:
  QPolygonF m_polygon;
};

namespace details {

template <Annotation::Type type> struct AnnotationTypeTraits;
template <> struct AnnotationTypeTraits<Annotation::Type::Line> {
  using type = LineItem;
};
template <> struct AnnotationTypeTraits<Annotation::Type::Rect> {
  using type = RectItem;
};
template <> struct AnnotationTypeTraits<Annotation::Type::Fan> {
  using type = FanItem;
};
template <> struct AnnotationTypeTraits<Annotation::Type::Polygon> {
  using type = PolygonItem;
};

} // namespace details

// Create an instance of the correct graphics item for the given annotation
inline GraphicsItemBase *makeGraphicsItem(const Annotation &annotation) {
  GraphicsItemBase *item{};

  switch (annotation.type()) {
  case Annotation::Line: {
    using ItemType = details::AnnotationTypeTraits<Annotation::Line>::type;
    item = new ItemType;

  } break;
  case Annotation::Rect: {
    using ItemType = details::AnnotationTypeTraits<Annotation::Rect>::type;
    item = new ItemType;

  } break;
  case Annotation::Fan: {
    using ItemType = details::AnnotationTypeTraits<Annotation::Fan>::type;
    item = new ItemType;

  } break;
  case Annotation::Polygon: {
    using ItemType = details::AnnotationTypeTraits<Annotation::Polygon>::type;
    item = new ItemType;
  } break;
  }

  return item;
}

} // namespace annotation