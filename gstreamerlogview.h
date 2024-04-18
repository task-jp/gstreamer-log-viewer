#ifndef GSTREAMERLOGVIEW_H
#define GSTREAMERLOGVIEW_H

#include <QtWidgets/QTableView>

class GStreamerLogView : public QTableView
{
    Q_OBJECT
public:
    explicit GStreamerLogView(QWidget *parent = nullptr);
    ~GStreamerLogView() override;

signals:
    void jumpToLog(int line);
    void jumpToSource(const QString &source, int line);
    void activated(const QString &header);

protected:
    void showEvent(QShowEvent *event) override;

private:
    class Private;
    QScopedPointer<Private> d;
};

#endif // GSTREAMERLOGVIEW_H
