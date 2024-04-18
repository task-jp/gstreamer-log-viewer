#ifndef GSTREAMERLOGWIDGET_H
#define GSTREAMERLOGWIDGET_H

#include <QtWidgets/QWidget>

class GStreamerLogWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ isBusy WRITE setBusy NOTIFY busyChanged FINAL)
    Q_PROPERTY(int count READ count CONSTANT FINAL)
    Q_PROPERTY(int filteredCount READ filteredCount NOTIFY filteredCountChanged FINAL)
public:
    explicit GStreamerLogWidget(const QString &fileName, QWidget *parent = nullptr);
    ~GStreamerLogWidget() override;

    bool isBusy() const;
    int count() const;
    int filteredCount() const;

public slots:
    void setBusy(bool busy);
    void reload();

signals:
    void busyChanged(bool busy);
    void filteredCountChanged(int count);
    void openPreferences();
    void errorOccurred(const QString &message);

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // GSTREAMERLOGWIDGET_H
