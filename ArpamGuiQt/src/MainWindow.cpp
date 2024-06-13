#include "MainWindow.hpp"
#include "About.hpp"
#include "CanvasAnnotationModel.hpp"
#include "CoregDisplay.hpp"
#include "DataProcWorker.hpp"
#include "FrameController.hpp"
#include "ReconParamsController.hpp"
#include "strConvUtils.hpp"
#include <QDockWidget>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMimeData>
#include <QScrollArea>
#include <QSlider>
#include <QToolBar>
#include <QVBoxLayout>
#include <QtDebug>
#include <QtLogging>
#include <filesystem>
#include <format>
#include <opencv2/opencv.hpp>
#include <qaction.h>
#include <qdockwidget.h>
#include <qnamespace.h>
#include <qwidget.h>
#include <uspam/defer.h>
#include <utility>

namespace {
void setGlobalStyle(QLayout *layout) {
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);
}
} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), textEdit(new QPlainTextEdit(this)),
      m_coregDisplay(new CoregDisplay), worker(new DataProcWorker) {

  // Enable QStatusBar at the bottom of the MainWindow
  statusBar();

  // Enable drop (bin files)
  setAcceptDrops(true);

  /**
   * Setup worker thread
   */
  {
    worker->moveToThread(&workerThread);

    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);

    connect(worker, &DataProcWorker::resultReady, m_coregDisplay,
            &CoregDisplay::imshow);

    connect(worker, &DataProcWorker::error, this, &MainWindow::logError);

    workerThread.start();
  }

  /**
   * Setup GUI
   */

  {
    auto *dock = new QDockWidget("Log", this);
    dock->setFeatures(dock->features() ^ (QDockWidget::DockWidgetClosable));
    this->addDockWidget(Qt::TopDockWidgetArea, dock);

    // Error box
    // dockLayout->addWidget(textEdit);
    dock->setWidget(textEdit);
    textEdit->setReadOnly(true);
    textEdit->setPlainText("Application started.\n");
    textEdit->appendPlainText(ARPAM_GUI_ABOUT()());
  }

  // Frame controller
  {
    auto *dock = new QDockWidget("Frame Controller", this);
    dock->setFeatures(dock->features() ^ (QDockWidget::DockWidgetClosable));
    this->addDockWidget(Qt::TopDockWidgetArea, dock);

    m_frameController = new FrameController;
    // dockLayout->addWidget(m_frameController);
    dock->setWidget(m_frameController);

    connect(m_frameController, &FrameController::sigBinfileSelected,
            [=](const QString &filepath) {
              const auto pathUtf8 = filepath.toUtf8();
              std::filesystem::path path(pathUtf8.constData());

              QMetaObject::invokeMethod(worker, &DataProcWorker::setBinfile,
                                        path);

              m_coregDisplay->setSequenceName(path2QString(path.stem()));
            });

    connect(m_frameController, &FrameController::sigFrameNumUpdated, worker,
            &DataProcWorker::playOne);

    connect(m_frameController, &FrameController::sigPlay, [=] {
      QMetaObject::invokeMethod(worker, &DataProcWorker::play);
      m_coregDisplay->resetZoomOnNextImshow();
    });

    connect(m_frameController, &FrameController::sigPause, this,
            [&]() { worker->pause(); });

    connect(worker, &DataProcWorker::maxFramesChanged, [=](int maxIdx) {
      m_frameController->updateMaxFrameNum(maxIdx);
      m_coregDisplay->setMaxIdx(maxIdx);
    });

    connect(worker, &DataProcWorker::frameIdxChanged, [=](int idx) {
      m_frameController->updateFrameNum(idx);
      m_coregDisplay->setIdx(idx);
    });

    connect(worker, &DataProcWorker::finishedPlaying,
            [=] { m_frameController->updatePlayingState(false); });
  }

  // Recon parameters controller
  {
    auto *dock = new QDockWidget("Recon Parameters", this);
    dock->setFeatures(dock->features() ^ (QDockWidget::DockWidgetClosable));
    this->addDockWidget(Qt::TopDockWidgetArea, dock);

    auto *reconParamsController = new ReconParamsController;
    // dockLayout->addWidget(reconParamsController);
    dock->setWidget(reconParamsController);

    connect(reconParamsController, &ReconParamsController::paramsUpdated,
            [this](uspam::recon::ReconParams2 params,
                   uspam::io::IOParams ioparams) {
              // Update params
              this->worker->updateParams(std::move(params), ioparams);

              // Only invoke "replayOne" if not currently worker is not playing
              if (this->worker->isReady() && !this->worker->isPlaying()) {
                QMetaObject::invokeMethod(worker, &DataProcWorker::replayOne);

                // Save params to file
                this->worker->saveParamsToFile();
              }
            });

    connect(reconParamsController, &ReconParamsController::error, this,
            &MainWindow::logError);
  }

  // Exit button
  {
    auto *dock = new QDockWidget("Exit", this);
    dock->setFeatures(dock->features() ^ (QDockWidget::DockWidgetClosable));
    this->addDockWidget(Qt::TopDockWidgetArea, dock);

    auto *w = new QWidget;
    auto *layout = new QVBoxLayout;
    // dockLayout->addLayout(layout);
    w->setLayout(layout);
    dock->setWidget(w);

    auto *closeBtn = new QPushButton("Close");
    layout->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, this, &QMainWindow::close);
    closeBtn->setObjectName("closeButton");

    auto *toggleFullscreenBtn = new QPushButton("Toggle Fullscreen");
    layout->addWidget(toggleFullscreenBtn);
    connect(toggleFullscreenBtn, &QPushButton::clicked, this, [this] {
      if (this->isFullScreen()) {
        this->setWindowState(Qt::WindowMaximized);
      } else {
        this->setWindowState(Qt::WindowFullScreen);
      }
    });
    toggleFullscreenBtn->setObjectName("toggleFullscreenButton");
  }
  // End dock config

  // Coreg display
  setCentralWidget(m_coregDisplay);

  connect(m_coregDisplay, &CoregDisplay::message, this, &MainWindow::logError);
  connect(m_coregDisplay, &CoregDisplay::mouseMoved, this,
          [&](QPoint pos, double depth_mm) {
            auto msg = QString("Pos: (%1, %2), depth: %3 mm")
                           .arg(pos.x())
                           .arg(pos.y())
                           .arg(depth_mm);
          });

  // auto *modeSwitchButton = new QPushButton("Switch Mode", this);
  // connect(modeSwitchButton, &QPushButton::clicked, this,
  //         &MainWindow::switchMode);
  // layout->addWidget(modeSwitchButton);

  // Add mode views
  // stackedWidget->addWidget(new RealTimeView());
  // stackedWidget->addWidget(new PostProcessingView());

  //   layout->addWidget(modeSwitchButton);

  // Set global style
  setGlobalStyle(m_coregDisplay->layout());

  // Create menus
  m_fileMenu = menuBar()->addMenu(tr("&File"));
  m_fileMenu->addAction(m_frameController->get_actOpenFileSelectDialog());
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  const auto *mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    const auto &urls = mimeData->urls();
    // Only allow dropping a single file
    if (urls.size() == 1) {
      const auto filepath = urls[0].toLocalFile();

      // Only allow a specific extension
      if (filepath.endsWith(".bin")) {
        event->acceptProposedAction();
      }
    }
  }
}

void MainWindow::dropEvent(QDropEvent *event) {
  const auto *mimeData = event->mimeData();
  if (mimeData->hasUrls()) {
    const auto &urls = mimeData->urls();
    const auto filepath = urls[0].toLocalFile();
    m_frameController->acceptNewBinfile(filepath);

    event->acceptProposedAction();
  }
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  const auto KeyNextFrame = Qt::Key_Period;
  const auto KeyPrevFrame = Qt::Key_Comma;
  const auto KeyPlayPause = Qt::Key_Space;

  switch (event->key()) {
  case KeyNextFrame:
    m_frameController->nextFrame();
    break;
  case KeyPrevFrame:
    m_frameController->prevFrame();
    break;
  case KeyPlayPause:
    m_frameController->togglePlayPause();
    break;
  default:
    QMainWindow::keyPressEvent(event);
  }
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event) {
  // TODO
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // Stop the worker thread
  if (workerThread.isRunning()) {
    this->worker->pause();
    workerThread.quit();
    workerThread.wait();
  }
  event->accept();
}

void MainWindow::logError(QString message) {
  textEdit->appendPlainText(message);
}

void MainWindow::switchMode() {
  //   int currentIndex = stackedWidget->currentIndex();
  //   stackedWidget->setCurrentIndex(1 - currentIndex); // Toggle between 0 and
  //   1
}
