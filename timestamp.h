#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <QtCore/QMetaType>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>

class Timestamp
{
public:
    static int metaTypeId;
    Timestamp();
    Timestamp(const Timestamp &other);
    ~Timestamp();

    static Timestamp fromString(const QString &text);
    QString toString() const;

    Timestamp &operator=(const Timestamp &other);
    int operator<=>(const Timestamp &other) const;
    bool operator==(const Timestamp &other) const;

    qint64 secsTo(const Timestamp &other) const;
    qint64 msecsTo(const Timestamp &other) const;
    qint64 usecsTo(const Timestamp &other) const;
    qint64 nsecsTo(const Timestamp &other) const;

private:
    class Private;
    QScopedPointer<Private> d;
};

Q_DECLARE_METATYPE(Timestamp)

#endif // TIMESTAMP_H
