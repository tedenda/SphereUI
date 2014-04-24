#ifndef SPHEREUI_H
#define SPHEREUI_H

#include <QMainWindow>
#include <QDebug>

#include <QtSerialPort/QSerialPortInfo>
#include <QGraphicsSvgItem>

#include "serial.h"
#include "txthread.h"

namespace Ui {
class SphereUI;
}

class SphereUI : public QMainWindow
{
    Q_OBJECT
    enum uiStates {stIdle,stSending,stStopped};
public:
    explicit SphereUI(QWidget *parent = 0);
    ~SphereUI();

public slots:
    void sendDataUI(QString data);
    void finishedTransmission();

private slots:
        /* Main ****/
    void on_actionMain_triggered(void);
    void on_actionGCode_triggered(void);
    void on_actionSVG_triggered(void);
    void on_actionSettings_triggered(void);

            /* Serial */
    void on_btnSerialConnect_clicked(void);
    void on_btnSerialRefresh_clicked(void);
            /* Params */
    void on_btnSetDiam_clicked(void);
    void on_btnSetFeed_clicked(void);
    void on_btnSetZoom_clicked(void);
            /* Motors */
    void on_btnZeroPen_clicked(void);
    void on_btnZeroEgg_clicked(void);
    void on_spinMotorEgg_valueChanged(int val);
    void on_spinMotorPen_valueChanged(int val);
    // Nope    void on_slideMotorEgg_sliderMoved(int val);
    // Nope    void on_slideMotorPen_sliderMoved(int val);
            /* Servo pen */
    void on_btnSendPenUp_clicked(void);
    void on_btnSendPenDown_clicked(void);
    void on_spinServoPen_valueChanged(int val);
    // Nope    void on_slideServoPen_sliderMoved(int val);
            /* Serial comm */
    void on_btnSerialSend_clicked(void);
    void on_lineSerialTX_returnPressed(void);
    void receiveData(void);

        /* GCode ****/
    void on_btnGCopen_clicked(void);
    void on_btnGCundo_clicked(void);
    void on_btnGCredo_clicked(void);
    void on_btnGCsave_clicked(void);
    void on_txtGCode_textChanged(void);
    void on_txtGCode_undoAvailable(bool b);
    void on_txtGCode_redoAvailable(bool b);

    void on_btnExecPrint_clicked(void);
    void on_btnExecPause_clicked(void);
    void refreshSendProgress(int value);

        /* Settings ****/
    void on_btnApplySettings_clicked(void);

    void fitgraphicsView(void);
    void interpretSentString(QString string);
    void finishedLayer(void);
private:
    Ui::SphereUI *ui;
    serial *serialConn;
    QGraphicsScene *SVGscene;
    uiStates uiState;

    QString curFile;
    QString curDir;
    QList<QString> layerNames;        //layerFile, layerColorString
    int layerIndex;

    txThread Transceiver;
    void refreshCOMList(void);
    void UIserialConnected(bool v);
    void resetSettings(void);
    void setState(uiStates state);
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void interpretGcode(QString code);
    void refreshLayerNames(QString file);
    QString removeComments(QString file);
    void resizeEvent(QResizeEvent *event);
    void connectTranceiver(void);
    void disconnectTranceiver(void);
};

#endif // SPHEREUI_H
