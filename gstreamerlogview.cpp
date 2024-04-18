#include "gstreamerlogview.h"
#include "gstreamerlogmodel.h"

#include <QtCore/QSettings>

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>

class GStreamerLogView::Private
{
public:
    Private(GStreamerLogView *parent);

private:
    GStreamerLogView *q;
public:
    QSettings settings;
};

GStreamerLogView::Private::Private(GStreamerLogView *parent)
    : q(parent)
{
    settings.beginGroup(q->metaObject()->className());
    q->setTextElideMode(Qt::ElideNone);
    connect(q->horizontalHeader(), &QHeaderView::sectionDoubleClicked, [this](int logicalIndex) {
        if (logicalIndex == 0)
            return;
        const auto header = q->model()->headerData(logicalIndex, Qt::Horizontal).toString();
        emit q->activated(header + QStringLiteral(":"));
    });

    connect(q, &GStreamerLogView::doubleClicked, [this](const QModelIndex &index) {
        const auto text = q->model()->data(index).toString();
        switch (index.column()) {
        case GStreamerLogModel::TimestampColumn: {
            const auto line = index.data(Qt::UserRole).toInt();
            emit q->jumpToLog(line);
            break; }
        case GStreamerLogModel::LineColumn: {
            const auto source = index.siblingAtColumn(GStreamerLogModel::SourceColumn).data().toString();
            const auto line = index.data().toInt();
            emit q->jumpToSource(source, line);
            break; }
        case GStreamerLogModel::MessageColumn:
            // message
            emit q->activated(text);
            break;
        default: {
            const auto header = q->model()->headerData(index.column(), Qt::Horizontal).toString();
            emit q->activated(QStringLiteral("%1:%2 ").arg(header).arg(text.trimmed()));
            break; }
        }
    });
}

GStreamerLogView::GStreamerLogView(QWidget *parent)
    : QTableView(parent)
    , d(new Private(this))
{}

GStreamerLogView::~GStreamerLogView()
{
    d->settings.setValue(QStringLiteral("headerState"), horizontalHeader()->saveState());
}

void GStreamerLogView::showEvent(QShowEvent *event)
{
    if (d->settings.contains(QStringLiteral("headerState")))
        horizontalHeader()->restoreState(d->settings.value(QStringLiteral("headerState")).toByteArray());
    else
        resizeColumnsToContents();
}
