#include "timestampview.h"
#include "gstreamerlogmodel.h"
#include "timestamp.h"

#include <QtCore/QTime>

#include <QtGui/QPainter>

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollBar>

namespace {
    struct Cache {
        QImage timeline;
        int rowCount = 0;
    };
}

class TimestampView::Private
{
public:
    QTableView *buddy = nullptr;
    Cache cache;
};

TimestampView::TimestampView(QWidget *parent)
    : QWidget{parent}
    , d(new Private)
{
    connect(this, &TimestampView::buddyChanged, [this](QTableView *buddy) {
        if (buddy) {
            auto scrollBar = buddy->verticalScrollBar();
            connect(scrollBar, &QScrollBar::valueChanged, this, qOverload<>(&TimestampView::update));
            const auto model = buddy->model();
            if (model) {
                connect(model, &QAbstractItemModel::rowsInserted, this, qOverload<>(&TimestampView::update));
                connect(model, &QAbstractItemModel::rowsRemoved, this, qOverload<>(&TimestampView::update));
            } else {
                qFatal("model must be set before setBuddy");
            }
        }
    });
}

TimestampView::~TimestampView() = default;

QTableView *TimestampView::buddy() const
{
    return d->buddy;
}

void TimestampView::setBuddy(QTableView *buddy)
{
    if (d->buddy == buddy) return;
    d->buddy = buddy;
    emit buddyChanged(buddy);
}

void TimestampView::paintEvent(QPaintEvent *event)
{
    if (!d->buddy) return;
    const auto model = d->buddy->model();
    if (!model) return;
    const auto count = model->rowCount();
    if (count < 1) return;

    const int w = width() / 5;
    const int headerHeight = d->buddy->horizontalHeader()->height();
    const int h = height();
    const qreal dy = (qreal)(h - headerHeight) / (count - 1);

    auto index2timestamp = [&](int row) -> Timestamp {
        const auto data = model->index(row, GStreamerLogModel::TimestampColumn).data().toString();
        static QHash<QString, Timestamp> cache;
        if (cache.contains(data))
            return cache.value(data);
        const auto timestamp = Timestamp::fromString(data);
        cache.insert(data, timestamp);
        return timestamp;
    };

    if (d->cache.timeline.size() != QSize(w * 2, h) || d->cache.rowCount != count) {

        const auto timestampMin = index2timestamp(0);
        const auto timestampMax = index2timestamp(count - 1);
        const qreal range = timestampMin.usecsTo(timestampMax);

        d->cache.timeline = QImage(QSize(w * 2, h), QImage::Format_ARGB32_Premultiplied);
        d->cache.timeline.fill(Qt::transparent);
        QPainter painter(&d->cache.timeline);
        painter.setRenderHint(QPainter::Antialiasing);

        QPen pen(Qt::red);
        pen.setWidth(3);
        painter.setPen(pen);

        QPolygonF graph = { QPointF(0, headerHeight) };
        for (int i = 1; i < count; i++) {
            const auto timestamp = index2timestamp(i);
            const qreal x = timestampMin.usecsTo(timestamp) / range * w * 2;
            graph << QPointF(x, dy * i + headerHeight);
        }
        painter.drawPolyline(graph);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(0.5);
    const auto brush = [&]() {
        QLinearGradient gradient(w, h / 2, w * 5, h / 2);
        gradient.setColorAt(0, Qt::red);
        gradient.setColorAt(0.5, Qt::green);
        gradient.setColorAt(1.0, Qt::blue);
        return QBrush(gradient);
    }();
    painter.setPen([&]() {
        QPen pen;
        pen.setBrush(brush);
        return pen;
    }());
    painter.setBrush(brush);

    painter.drawImage(0, 0, d->cache.timeline);

    const auto firstIndex = d->buddy->indexAt(QPoint(0, 0));
    auto firstRow = firstIndex.row();
    if (firstRow < 0)
        firstRow = 0;
    const auto lastIndex = d->buddy->indexAt(QPoint(0, h - headerHeight));
    auto lastRow = lastIndex.row();
    if (lastRow < 0)
        lastRow = count - 1;

    QRectF cursor(0, dy * firstRow + headerHeight, w * 2, dy * (lastRow - firstRow));
    painter.drawRect(cursor);

    QPolygonF polygon = { QPointF(w * 2, cursor.top()), QPointF(w * 3, headerHeight), QPointF(w * 3, h), QPointF(w * 2, cursor.bottom()) };
    painter.drawPolygon(polygon);

    QPointF origin(w, cursor.center().y());
    const auto firstTimestamp = index2timestamp(firstRow);
    const auto lastTimestamp = index2timestamp(lastRow);
    const qreal range = qMax(firstTimestamp.usecsTo(lastTimestamp), 1);
    for (int row = firstRow; row <= lastRow; row++) {
        const auto rect = d->buddy->visualRect(firstIndex.siblingAtRow(row));
        const auto timestamp = index2timestamp(row);
        const qreal y = firstTimestamp.usecsTo(timestamp) / range * (h - headerHeight);
        QPolygonF polygon = {
            QPointF(w * 3, y + headerHeight),
            QPointF(w * 5, rect.top() + 5 + headerHeight),
            QPointF(w * 5, rect.bottom() - 5 + headerHeight),
        };
        painter.drawPolygon(polygon);
    }
}

void TimestampView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}
