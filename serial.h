#ifndef SERIAL_H
#define SERIAL_H

#include <QObject>
#include <QString>
#include <QtSerialPort/QSerialPort>

class serial : public QObject
{
    Q_OBJECT
private:
     QSerialPort *serialPort;
     bool isConnected_;
     unsigned int serialBRate;
public:
    explicit serial(QObject *parent = 0);
     bool isConnected();
     bool send(QString cmd);
     QSerialPort* getPort(void) {
        return serialPort;
     }
     QString receiveData(void);

signals:
    void dataSent(QString data);

public slots:
     bool connect(QString portName,unsigned int rate);
     bool setBaudrate(unsigned int rate);
     bool reconnect(void);
     bool disconnect(void);

};

#endif // SERIAL_H
