#ifndef TXTHREAD_H
#define TXTHREAD_H
#include <QThread>
#include <QString>

#include <QMessageBox>
#include <QTimer>
#include <QString>

#include "serial.h"

class txThread : public QThread
{
    Q_OBJECT
public:
    txThread();
    void set(QString textfile,serial &bot);
    void run();
    ~txThread();
    int getLineCounter();

signals:
    void progressChanged(int);
    void layerTransmitted();
    void fileTransmitted();

public slots:
    void sendNext();
    void resetState();
private:
    QString textfile;
    int lineCounter;
    int lineMax;
    bool ignoreFirstM01;
    serial *serialConn;
    QString removeComments(QString file);
};

#endif // TXTHREAD_H
