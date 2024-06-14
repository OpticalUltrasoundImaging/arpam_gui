#pragma once

#include "Canvas.hpp"
#include "CanvasAnnotationModel.hpp"
#include "CanvasAnnotationView.hpp"
#include <QLabel>
#include <QTableView>
#include <QWidget>

class CoregDisplay : public QWidget {
  Q_OBJECT
public:
  explicit CoregDisplay(QWidget *parent = nullptr);

  auto annotationView() { return m_annoView; }

signals:
  void message(const QString &msg);
  void mouseMoved(QPoint pos, double depth_mm);

public slots:
  void setCursorMode(Canvas::CursorMode mode) {
    m_canvasLeft->setCursorMode(mode);
    m_canvasRight->setCursorMode(mode);
  }

  void undo() {
    m_canvasLeft->undo();
    m_canvasRight->undo();
  }

  void imshow(const QImage &img1, const QImage &img2, double pix2m);

  void resetZoom() { actResetZoom->trigger(); }
  void resetZoomOnNextImshow() {
    m_canvasLeft->resetZoomOnNextImshow();
    m_canvasRight->resetZoomOnNextImshow();
  }

  void setSequenceName(const QString &sequence) {
    m_canvasLeft->overlay()->setSequence(sequence);
    m_canvasRight->overlay()->setSequence(sequence);
  }

  void setMaxIdx(int maxIdx) {
    m_canvasLeft->overlay()->setMaxIdx(maxIdx);
    m_canvasRight->overlay()->setMaxIdx(maxIdx);
  }

  void setIdx(int idx) {
    m_canvasLeft->overlay()->setIdx(idx);
    m_canvasRight->overlay()->setIdx(idx);
  }

private:
  Canvas *m_canvasLeft;
  Canvas *m_canvasRight;

  AnnotationModel *m_model;

  AnnotationView *m_annoView;

  // Reset image zoom
  QAction *actResetZoom;

  // Action to use default cursor
  QAction *actCursorDefault;
  // Action to use cursor to pan
  QAction *actCursorPan;
  // Action to undo the last cursor action in the canvas
  QAction *actCursorUndo;
  // Action to set the cursor mode to line measure
  QAction *actCursorLine;
  // Action to set the cursor mode to label rect
  QAction *actCursorLabelRect;
  // Action to set the cursor mode to label fan
  QAction *actCursorLabelFan;
};
