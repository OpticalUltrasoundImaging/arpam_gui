#pragma once

#include <QImage>
#include <QLabel>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QThread>
#include <QVBoxLayout>

#include "CoregDisplay.hpp"
#include "DataProcWorker.hpp"
#include "ImshowCanvas.hpp"

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow() = default;

public slots:
  // Log error message to a text box on screen
  void logError(const QString &message);

  void switchMode();

  // Opens a file dialog to select a bin file and then launches a
  // worker thread to process images and display
  void openBinFile();
  void abortCurrentWorkInThread();

  void handleNewImages(QImage img1, QImage img2);

signals:
  void setProcWorkerBinfile(QString binfile);

private:
  void closeEvent(QCloseEvent *event) override;

  QPlainTextEdit *textEdit;
  // CoregDisplay *coregDisplay;
  ImshowCanvas *canvasLeft;
  ImshowCanvas *canvasRight;

  QThread workerThread;
  DataProcWorker *worker;
};