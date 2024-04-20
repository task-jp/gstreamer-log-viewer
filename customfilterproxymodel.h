#ifndef CUSTOMFILTERPROXYMODEL_H
#define CUSTOMFILTERPROXYMODEL_H

#include <QtCore/QSortFilterProxyModel>

using QIntList = QList<int>;

class CustomFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged FINAL)

public:
    explicit CustomFilterProxyModel(QObject *parent = nullptr);
    ~CustomFilterProxyModel() override;

    QString filter() const;

public slots:
    void setFilter(const QString &filter);

signals:
    void filterChanged(const QString &filter);

protected:
    QVariant data(const QModelIndex &index, int role) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    class Private;
    QScopedPointer<Private> d;

    // QAbstractItemModel interface
public:
    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const override;
};

#endif // CUSTOMFILTERPROXYMODEL_H
