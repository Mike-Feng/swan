#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H
#include <QThread>
#include <QImage>

namespace Cg { namespace Swan { namespace Adapter {

class CaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit CaptureThread(QObject *parent = 0);

public:
    void run();
    void stream();
    void pause();
    void stop();

public slots:

signals:
    void captured(QImage & img);

private:
    bool pause_status;
    bool quit;
    QVector<QRgb> grayColourTable;
    QVector<QRgb> ColourTable;

};
} } }
#endif // CAPTURETHREAD_H
