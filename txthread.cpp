#include <QDebug>
#include "txthread.h"
txThread::txThread()
{
    resetState();
}

txThread::~txThread()
{

}

void txThread::resetState()
{
    lineCounter = 0;
    ignoreFirstM01 = true;
}

QString txThread::removeComments(QString intext)
{
    ////////////////////////////////////////////////remove comments
    QString outTmp1,outTmp2;
    bool state=1;   //1= send, 0 = ignore
    for(int i=0;i<intext.size();i++)
    {
        if(intext.at(i) == '(' ) state=0;
        if(state == 1) outTmp1.append(intext[i]);
        if(intext.at(i) == ')' ) state=1;
    }
    //////////////////////////////////////////////////remove empty lines
    for(int i=0;i<outTmp1.size();i++)
        {
            if(i != outTmp1.size())
            {
                if(!(outTmp1[i] == '\n' && outTmp1[i+1] == '\n'))
                {
                    outTmp2.append(outTmp1[i]);
                }
            }
        }
        ///////////////////////////////////////////////////
    return outTmp2;
}

void txThread::set(QString intextfile,serial &uibot)
{
    lineCounter = 0;
    textfile.clear();
    textfile.append(removeComments(intextfile));
    //qDebug()<<"The textfile String is: \n\n" + textfile + "\n\nENDE\n\n";
    lineMax = textfile.count("\n");
    serialConn = &uibot;
}

void txThread::run()
{
//    qDebug()<<"entering run";
    lineCounter = 0;
    sendNext();
}

void txThread::sendNext()
{
    QString tmp;
    if(lineCounter <= lineMax)
    {
        tmp = textfile.section("\n",lineCounter,lineCounter);
//        qDebug() << tmp;
        if(tmp.contains("M01"))
        {
            if(ignoreFirstM01)
            {
                ignoreFirstM01 = false;
            }
            else
            {
                emit layerTransmitted();
            }
        }
        tmp.append("\n");
        serialConn->send(tmp);
        double progress= (double) lineCounter/(double)lineMax;
        emit progressChanged(progress*100);
        lineCounter++;
    }
    else
    {
        emit fileTransmitted();
        resetState();
        return;
    }
    if(tmp.contains("G4"))
    {
        msleep(300);
    }
}

int txThread::getLineCounter()
{
    return lineCounter;
}
