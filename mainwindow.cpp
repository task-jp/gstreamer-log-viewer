#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gstreamerlogwidget.h"
#include "preferences.h"

#include <QtCore/QSettings>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QProgressBar>

class MainWindow::Private : public Ui::MainWindow
{
public:
    Private(::MainWindow *parent);
    ~Private();

private:
    void openFile(const QString &fileName);

private:
    ::MainWindow *q;
    QSettings settings;
};

MainWindow::Private::Private(::MainWindow *parent)
    : q(parent)
{
    setupUi(q);
    statusbar->addPermanentWidget(progressBar);
    statusbar->addPermanentWidget(counts);
    progressBar->setVisible(false);
    q->setCentralWidget(tabWidget);
    tabWidget->clear();
    connect(tabWidget, &QTabWidget::currentChanged, [this](int index) {
        if (index == -1)
            return;
        auto widget = tabWidget->widget(index);
        auto tableView = qobject_cast<GStreamerLogWidget *>(widget);
        if (!tableView)
            return;
        counts->setText(QStringLiteral("%1/%2").arg(tableView->filteredCount()).arg(tableView->count()));
    });

    settings.beginGroup(q->metaObject()->className());
    q->restoreGeometry(settings.value(QStringLiteral("geometry")).toByteArray());
    q->restoreState(settings.value(QStringLiteral("state")).toByteArray());
    const auto recentFiles = settings.value(QStringLiteral("recentFiles")).toStringList();

    reload->setEnabled(false);
    close->setEnabled(false);

    connect(open, &QAction::triggered, [this]() {
        const auto caption = tr("Open GStreamer log file");
        QFileDialog dialog(q, caption);
        dialog.setFileMode(QFileDialog::ExistingFiles);
        QStringList nameFilters = {
            tr("GStreamer log files (*.log *.txt)"),
            tr("All Files (*)"),
        };
        dialog.setNameFilters(nameFilters);
        dialog.setAcceptMode(QFileDialog::AcceptOpen);
        if (dialog.exec() != QDialog::Accepted)
            return;
        openFile(dialog.selectedFiles().first());
    });

    openRecent->setEnabled(!recentFiles.isEmpty());
    for (const auto &file : recentFiles) {
        const auto action = openRecent->addAction(file);
        connect(action, &QAction::triggered, [this, file]() {
            openFile(file);
        });
        if (qEnvironmentVariableIsSet("GLV_OPEN_FIRST")) {
            static bool first = true;
            if (first) {
                first= false;
                openFile(file);
            }
        }
    }
    connect(reload, &QAction::triggered, [this]() {
        static_cast<GStreamerLogWidget *>(tabWidget->currentWidget())->reload();
    });

    connect(preferences, &QAction::triggered, [this]() {
        Preferences dialog(q);
        dialog.exec();
    });

    connect(close, &QAction::triggered, [this]() {
        tabWidget->currentWidget()->deleteLater();
        tabWidget->removeTab(tabWidget->currentIndex());
        reload->setEnabled(tabWidget->count() > 0);
        close->setEnabled(tabWidget->count() > 0);
    });

    connect(quit, &QAction::triggered, q, &QMainWindow::close);

    connect(aboutApp, &QAction::triggered, [this]() {
        QString title = tr("About %1").arg(q->windowTitle());
        QString description = tr("%1 is a tool to view log from GStreamer(e.g. GST_DEBUG=4)").arg(q->windowTitle());
        QMessageBox::about(q, title, description);
    });

    connect(aboutQt, &QAction::triggered, [this]() {
        QMessageBox::aboutQt(q);
    });
}

MainWindow::Private::~Private()
{
    settings.setValue(QStringLiteral("geometry"), q->saveGeometry());
    settings.setValue(QStringLiteral("state"), q->saveState());
}

void MainWindow::Private::openFile(const QString &fileName) {
    for (int i = 0; i < tabWidget->count(); ++i) {
        auto tabToolTip = tabWidget->tabToolTip(i);
        if (tabToolTip == fileName) {
            tabWidget->setCurrentIndex(i);
            return;
        }
    }

    QFileInfo fileInfo(fileName);
    QGuiApplication::setOverrideCursor(Qt::BusyCursor);
    auto tableView = new GStreamerLogWidget(fileName);
    QGuiApplication::restoreOverrideCursor();
    connect(tableView, &GStreamerLogWidget::busyChanged, [this](bool busy) {
        progressBar->setVisible(busy);
        if (busy)
            QGuiApplication::setOverrideCursor(Qt::BusyCursor);
        else
            QGuiApplication::restoreOverrideCursor();
    });
    connect(tableView, &GStreamerLogWidget::openPreferences, [this](const QString &focus) {
        Preferences dialog(q);
        dialog.setCurrentField(focus);
        dialog.exec();
    });
    connect(tableView, &GStreamerLogWidget::errorOccurred, [this](const QString &message) {
        statusbar->showMessage(message, 10000);
    });
    connect(tableView, &GStreamerLogWidget::filteredCountChanged, [tableView, this](int count) {
        counts->setText(QStringLiteral("%1/%2").arg(count).arg(tableView->count()));
    });
    counts->setText(QStringLiteral("%1/%2").arg(tableView->filteredCount()).arg(tableView->count()));

    int index = tabWidget->addTab(tableView, fileInfo.fileName());
    tabWidget->setCurrentIndex(index);
    tabWidget->setTabToolTip(index, fileName);
    reload->setEnabled(true);
    close->setEnabled(true);

    auto recentFiles = settings.value(QStringLiteral("recentFiles")).toStringList();
    if (recentFiles.contains(fileName))
        recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    while (recentFiles.count() > 10)
        recentFiles.removeLast();
    settings.setValue(QStringLiteral("recentFiles"), recentFiles);
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , d(new Private(this))
{
}

MainWindow::~MainWindow() = default;
