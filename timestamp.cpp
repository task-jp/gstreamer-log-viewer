#include "timestamp.h"

#include <QtCore/QTime>

int Timestamp::metaTypeId = qRegisterMetaType<Timestamp>();

class Timestamp::Private
{
public:
    Private()
    {}

    Private(const Private &other)
        : time(other.time)
        , nsecs(other.nsecs)
        , string(other.string)
    {}

public:
    QTime time;
    qint64 nsecs = 0;
    mutable QString string;
};

Timestamp::Timestamp()
    : d(new Private)
{}

Timestamp::Timestamp(const Timestamp &other)
    : d(new Private(*other.d))
{}

Timestamp::~Timestamp() = default;

Timestamp Timestamp::fromString(const QString &text)
{
    Timestamp ret;
    const auto hmmdd = text.section('.', 0, 0);
    ret.d->time = QTime::fromString(hmmdd, "H:mm:ss");
    ret.d->nsecs = text.section('.', 1, 1).toLongLong();
    ret.d->string = text;
    return ret;
}

QString Timestamp::toString() const
{
    return d->string;
}

Timestamp &Timestamp::operator=(const Timestamp &other)
{
    d.reset(new Private(*other.d));
    return *this;
}

bool Timestamp::operator==(const Timestamp &other) const
{
    return d->time == other.d->time && d->nsecs == other.d->nsecs;
}

int Timestamp::operator<=>(const Timestamp &other) const
{
    auto secs = other.d->time.secsTo(d->time);
    if (secs != 0)
        return secs;
    return d->nsecs - other.d->nsecs;
}

qint64 Timestamp::secsTo(const Timestamp &other) const
{
    return d->time.secsTo(other.d->time);
}

qint64 Timestamp::msecsTo(const Timestamp &other) const
{
    auto secs = d->time.secsTo(other.d->time);
    return secs * 1000 + (other.d->nsecs - d->nsecs) / 1000000;
}

qint64 Timestamp::usecsTo(const Timestamp &other) const
{
    auto secs = d->time.secsTo(other.d->time);
    return secs * 1000000 + (other.d->nsecs - d->nsecs) / 1000;
}

qint64 Timestamp::nsecsTo(const Timestamp &other) const
{
    auto secs = d->time.secsTo(other.d->time);
    return secs * 1000000000 + (other.d->nsecs - d->nsecs);
}
