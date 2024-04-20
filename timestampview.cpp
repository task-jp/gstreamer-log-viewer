#include "timestampview.h"
#include "gstreamerlogmodel.h"
#include "timestamp.h"

#include <QtCore/QTime>

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
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
    QLabel *label;
    Cache cache;
};

TimestampView::TimestampView(QWidget *parent)
    : QWidget{parent}
    , d(new Private)
{
    d->label = new QLabel(this);
    setMouseTracking(true);
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

void TimestampView::mousePressEvent(QMouseEvent *event)
{
    const auto headerHeight = d->buddy->horizontalHeader()->height();
    const auto h = height();
    const auto y = qBound<qreal>(headerHeight, event->position().y(), h);
    const auto count = d->buddy->model()->rowCount();
    QModelIndex minIndex = d->buddy->model()->index(0, GStreamerLogModel::TimestampColumn);
    QModelIndex maxIndex = d->buddy->model()->index(count - 1, GStreamerLogModel::TimestampColumn);
    const auto minTimestamp = Timestamp::fromString(minIndex.data().toString());
    const auto maxTimestamp = Timestamp::fromString(maxIndex.data().toString());
    const auto mix = Timestamp::mix(minTimestamp, maxTimestamp, (qreal)(y - headerHeight) / (h - headerHeight));
    const auto indices = d->buddy->model()->match(minIndex, Qt::DisplayRole, QVariant::fromValue(mix), 1, Qt::MatchStartsWith); // abuse the flag for nearest timestamp match
    if (!indices.isEmpty()) {
        const auto index = indices.first();
        d->buddy->scrollTo(index, QAbstractItemView::PositionAtCenter);
    }
}

void TimestampView::mouseMoveEvent(QMouseEvent *event)
{
    const auto headerHeight = d->buddy->horizontalHeader()->height();
    const auto h = height();
    const auto y = qBound<qreal>(headerHeight, event->position().y(), h);
    const auto count = d->buddy->model()->rowCount();
    QString text;
    const QModelIndex minIndex = d->buddy->model()->index(0, GStreamerLogModel::TimestampColumn);
    const QModelIndex maxIndex = d->buddy->model()->index(count - 1, GStreamerLogModel::TimestampColumn);
    if (minIndex.isValid() && maxIndex.isValid()) {
        const auto minTimestamp = Timestamp::fromString(minIndex.data().toString());
        const auto maxTimestamp = Timestamp::fromString(maxIndex.data().toString());
        const auto mix = Timestamp::mix(minTimestamp, maxTimestamp, (qreal)(y - headerHeight) / (h - headerHeight));
        text = mix.toString();
    }
    d->label->setText(text);
    if (event->buttons() & Qt::LeftButton) {
        mousePressEvent(event);
    }
}

void TimestampView::paintEvent(QPaintEvent *event)
{
    if (!d->buddy) return;
    const auto model = d->buddy->model();
    if (!model) return;
    const auto count = model->rowCount();
    if (count < 1) return;

    const int w = width() / 3;
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

    const auto timestampMin = index2timestamp(0);
    const auto timestampMax = index2timestamp(count - 1);
    const qreal range = timestampMin.usecsTo(timestampMax);

    if (d->cache.timeline.size() != QSize(w, h) || d->cache.rowCount != count) {
        d->cache.timeline = QImage(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
        d->cache.timeline.fill(Qt::transparent);
        d->cache.rowCount = count;
        QPainter painter(&d->cache.timeline);
        painter.setRenderHint(QPainter::Antialiasing);

        QPen pen(QColor(255, 0, 0, 10));
        pen.setWidthF(0.5);
        painter.setPen(pen);

        QPolygonF lines;
        for (int i = 0; i < count; i++) {
            const auto timestamp = index2timestamp(i);
            const qreal y = timestampMin.usecsTo(timestamp) / range * (h - headerHeight) + headerHeight;
            lines << QPointF(0, y) << QPointF(w, y);
        }
        painter.drawLines(lines);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(0.5);
    const auto brush = [&]() {
        QLinearGradient gradient(w, h / 2, w * 3, h / 2);
        gradient.setColorAt(0.0, Qt::red);
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
    const auto firstTimestamp = index2timestamp(firstRow);
    const auto lastTimestamp = index2timestamp(lastRow);
    const qreal range2 = qMax(firstTimestamp.usecsTo(lastTimestamp), 1);
    const qreal yFirst = timestampMin.usecsTo(firstTimestamp) / range * (h - headerHeight) + headerHeight;
    const qreal yLast = timestampMin.usecsTo(lastTimestamp) / range * (h - headerHeight) + headerHeight;

    QPolygonF polygon = { QPointF(w, yFirst), QPointF(w * 2, headerHeight), QPointF(w * 2, h), QPointF(w, yLast) };
    painter.drawPolygon(polygon);

    QPointF origin(w, (yLast - yFirst) / 2);
    for (int row = firstRow; row <= lastRow; row++) {
        const auto rect = d->buddy->visualRect(firstIndex.siblingAtRow(row));
        const auto timestamp = index2timestamp(row);
        const qreal y = firstTimestamp.usecsTo(timestamp) / range2 * (h - headerHeight);
        QPolygonF polygon = {
            QPointF(w * 2, y + headerHeight),
            QPointF(w * 3, rect.top() + 5 + headerHeight),
            QPointF(w * 3, rect.bottom() - 5 + headerHeight),
        };
        painter.drawPolygon(polygon);
    }
}

void TimestampView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    d->label->resize(event->size().width(), d->label->sizeHint().height());
    update();
}
