#include "serial.h"
#include <QDebug>
#include <QMessageBox>

serial::serial(QObject *parent) :
    QObject(parent)
{
    serialPort = new QSerialPort();
    serialBRate = 115200;
//    isConnected_ = false;
}

bool serial::reconnect()
{
    serialPort->open(QIODevice::ReadWrite);
    QString cmd;
    if(!serialPort->isOpen())
    {
        cmd.append("Impossible to open ");
        cmd.append(serialPort->portName());
        cmd.append(" : ");
        cmd.append(serialPort->errorString());
        cmd.append("\n");
        emit dataSent(cmd);
        return false;
    }
    cmd.append("Opened port: ");
    cmd.append(serialPort->portName());
    cmd.append("\n");
    emit dataSent(cmd);

    serialPort->setBaudRate(serialBRate);//modify the port settings on your own
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setStopBits(QSerialPort::OneStop);
//    isConnected_ = true;
    return true;
}

bool serial::connect(QString portName,unsigned int rate)
{
    serialPort->setPortName(portName);
    setBaudrate(rate);
    return reconnect();
}

bool serial::setBaudrate(unsigned int rate)
{
    serialBRate = rate;
    return true;
}

bool serial::disconnect()
{
//    qDebug("port closed");
    serialPort->close();
//    isConnected_ = false;
    return true;
}

bool serial::send(QString cmd)
{
    cmd = cmd.trimmed();
    if(serialPort->isOpen())
    {
        serialPort->flush();
        cmd.append("\n");
//        qDebug()<<"Sending: " + cmd;
        serialPort->write((const char*)cmd.toUtf8(),cmd.length());
    }
    else
    {
        cmd.clear();
        cmd.append(" Serial error: ");
        cmd.append(serialPort->errorString());
//        emit dataSent(cmd);
        return false;
    }

    if(cmd.trimmed().length()) {
        cmd.prepend(">> ");
        emit dataSent(cmd);
    }
    return true;
}

QString serial::receiveData(void) {
    QString str;
    if(serialPort->canReadLine())
        str = serialPort->readLine(1024);
    return str;
}

bool serial::isConnected()
{
    return serialPort->isOpen();
}
