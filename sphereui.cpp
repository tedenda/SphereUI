#include "sphereui.h"
#include "ui_sphereui.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QScrollBar>

SphereUI::SphereUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SphereUI)
{
    ui->setupUi(this);

    /* Group actions in "View" menu, make them mutually exclusive */
    QActionGroup* grpView = new QActionGroup(this);
    ui->actionMain->setActionGroup(grpView);
    ui->actionGCode->setActionGroup(grpView);
    ui->actionSVG->setActionGroup(grpView);
    ui->actionSettings->setActionGroup(grpView);

    /* SVG display */
    SVGscene = new QGraphicsScene();
    ui->graphicsView->setScene(SVGscene);

    /* Serial connection */
    serialConn = new serial();

    layerIndex = 0;

    /* Connect slots */
    connect(ui->actExit,SIGNAL(triggered()),qApp,SLOT(closeAllWindows()));
    connect(ui->actAbout,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
    connect(serialConn, SIGNAL(dataSent(QString)),this, SLOT(sendDataUI(QString)));
    connect(serialConn->getPort(), SIGNAL(readyRead()), this, SLOT(receiveData()));
    connect(&Transceiver,SIGNAL(progressChanged(int)),this,SLOT(refreshSendProgress(int)));
    connect(&Transceiver,SIGNAL(fileTransmitted()),this,SLOT(finishedTransmission()));
    connect(&Transceiver,SIGNAL(layerTransmitted()),this,SLOT(finishedLayer()));
    connect(serialConn, SIGNAL(dataSent(QString)),this,SLOT(interpretSentString(QString)));

    /* Init */
    refreshCOMList();
    ui->pages->setCurrentIndex(0);
    setState(stIdle);
    resetSettings();
}

SphereUI::~SphereUI()
{
    delete serialConn;
    delete SVGscene;
    delete ui;
}

void SphereUI::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    fitgraphicsView();
}

void SphereUI::fitgraphicsView(void)      ////function to trigger the fitIn function for the graphics view. Actually this shouldn´t be necessary!
{
    if(!ui->graphicsView->items().isEmpty()) {
        QGraphicsItem *item = ui->graphicsView->items().first();
        ui->graphicsView->fitInView(item);
        ui->graphicsView->ensureVisible(item);
    }
}

    /* Menu choices **************************/
void SphereUI::on_actionMain_triggered(void) {
    ui->pages->setCurrentIndex(0);
}

void SphereUI::on_actionGCode_triggered(void) {
    ui->pages->setCurrentIndex(1);
}

void SphereUI::on_actionSVG_triggered(void) {
    ui->pages->setCurrentIndex(2);
}

void SphereUI::on_actionSettings_triggered(void) {
    ui->pages->setCurrentIndex(3);
}

    /* Main view *****************************/
void SphereUI::UIserialConnected(bool v) {
    ui->grpSerialComm->setEnabled(v);
    ui->grpParams->setEnabled(v);
    ui->grpMotors->setEnabled(v);
    ui->grpPen->setEnabled(v);
    ui->progbarGCode->setValue(0);
    if(v) {
        ui->spinServoPen->setValue(ui->spinPenUp->value());
        ui->spinMotorEgg->setValue(ui->spinMotorEggZero->value());
        ui->spinMotorPen->setValue(ui->spinMotorPenZero->value());
        ui->btnSerialConnect->setText("Disconnect");
        ui->comboSerial->setEnabled(false);
        if(!ui->txtGCode->toPlainText().isEmpty())
            ui->grpGCExecActions->setEnabled(true);
        on_btnApplySettings_clicked();
    } else {
        refreshCOMList();
        ui->btnSerialConnect->setText("Connect");
        ui->comboSerial->setEnabled(true);
        ui->grpGCExecActions->setEnabled(false);
//        setState(stIdle);
    }
}

void SphereUI::refreshCOMList(void) {

    QSerialPortInfo info;
    QList<QSerialPortInfo> serialList;
    serialList = info.availablePorts();
    ui->comboSerial->clear();
    // ui->cboxDefaultPort->clear();
    for(int i=0;i<serialList.size();i++)
    {
        ui->comboSerial->addItem(serialList[i].portName());
        // ui->cboxDefaultPort->addItem(serialList[i].portName());
    }
}

void SphereUI::on_btnSerialConnect_clicked(void) {
    if(!serialConn->isConnected()) {
        QString port = ui->comboSerial->currentText();
        if(port.length())
            UIserialConnected(serialConn->connect(port,ui->spinBaud->value()));
        setState(stIdle);
    } else {
        UIserialConnected(!serialConn->disconnect());
    }
}

void SphereUI::on_btnSerialRefresh_clicked(void) {
    refreshCOMList();
}

void SphereUI::on_lineSerialTX_returnPressed(void) {
    QString str = ui->lineSerialTX->text();
    if(!str.isEmpty()) {
        // str.append("\n");
        if(serialConn->isConnected()) {
            if(!serialConn->send(str))
                sendDataUI("Error while sending data!\n");
//            else
//                sendDataUI("Data sent !\n");
        }
        else
            sendDataUI("Not connected");
    }
}

void SphereUI::on_btnSerialSend_clicked(void) {
    on_lineSerialTX_returnPressed();
}

void SphereUI::on_btnZeroPen_clicked(void) {
    ui->spinMotorPen->setValue(ui->spinMotorPenZero->value());
}

void SphereUI::on_btnZeroEgg_clicked(void) {
    ui->spinMotorEgg->setValue(ui->spinMotorEggZero->value());
}


void SphereUI::on_btnSendPenUp_clicked(void) {
    ui->spinServoPen->setValue(ui->spinPenUp->value());
}

void SphereUI::on_btnSendPenDown_clicked(void) {
    ui->spinServoPen->setValue(ui->spinPenDown->value());
}

void SphereUI::sendDataUI(QString data)
{
    data = data.trimmed();
    data.append("\n");
    ui->txtSerialComm->insertPlainText(data);
        /* Autoscroll */
    QScrollBar *sb = ui->txtSerialComm->verticalScrollBar();
    sb->setValue(sb->maximum());
}

void SphereUI::on_btnSetDiam_clicked(void) {
    if(uiState != stSending)
    {
        QString tmp = ("M400 S" + QString::number(ui->spinDiam->value())+"\n");
        serialConn->send(tmp);
        tmp.clear();
        tmp = ("M401 S" + QString::number(ui->spinDiam->value())+"\n");
        serialConn->send(tmp);
    }
}

void SphereUI::on_btnSetFeed_clicked(void) {
    if(uiState != stSending)
    {
        QString tmp = ("G1 F" + QString::number(ui->spinFeed->value())+"\n");
        serialConn->send(tmp);
    }
}

void SphereUI::on_btnSetZoom_clicked(void) {
    if(uiState != stSending)
    {
        QString tmp = ("M402 S" + QString::number(ui->spinZoom->value())+"\n");
        serialConn->send(tmp);
    }
}

void SphereUI::on_spinServoPen_valueChanged(int val) {
    if(uiState != stSending)
    {
        QString tmp = ("M300 S" + QString::number(val)+"\n");
        serialConn->send(tmp);
    }
}

void SphereUI::on_spinMotorPen_valueChanged(int val) {
    if(uiState != stSending)
    {
        QString tmp = ("G1 Y" + QString::number(val)+"\n");
        serialConn->send(tmp);
    }
}

void SphereUI::on_spinMotorEgg_valueChanged(int val) {
    if(uiState != stSending)
    {
        QString tmp = ("G1 X" + QString::number(val)+"\n");
        serialConn->send(tmp);
    }
}
void SphereUI::receiveData()
{
    QString str = serialConn->receiveData();
    if(str.length() > 1) {
        str.chop(1);
        str.prepend("<< ");
        str.append("\n");
        sendDataUI(str);
    }
}


    /* GCode view ****************************/
void SphereUI::on_txtGCode_textChanged(void) {
    ui->btnGCsave->setEnabled(true);
    if(!ui->txtGCode->toPlainText().isEmpty()) {
        ui->grpGCExecActions->setEnabled(serialConn->isConnected());
    }
}


void SphereUI::on_btnGCundo_clicked()
{
    ui->txtGCode->undo();
}

void SphereUI::on_btnGCredo_clicked()
{
    ui->txtGCode->redo();
}

void SphereUI::on_txtGCode_undoAvailable(bool b)
{
    ui->btnGCsave->setEnabled(true);
    ui->btnGCundo->setEnabled(b);
}

void SphereUI::on_txtGCode_redoAvailable(bool b)
{
    ui->btnGCredo->setEnabled(b);
}

void SphereUI::on_btnGCopen_clicked(void) {
    if(uiState == stIdle)
    {
        QString fileName;
        if(!curDir.isEmpty())
        {
            fileName = QFileDialog::getOpenFileName(this,"",curDir);
        }
        else
        {
            fileName = QFileDialog::getOpenFileName(this);
        }
        if (!fileName.isEmpty())
        {
            //    qDebug()<<"loading file: "<<fileName;
            QFile file(fileName);
            if (!file.open(QFile::ReadOnly | QFile::Text)) {
                QMessageBox::warning(this, tr("Application"),
                                     tr("Cannot read file %1:\n%2.")
                                     .arg(fileName)
                                     .arg(file.errorString()));
                return;
            }
            curDir = QFileInfo(fileName).absoluteFilePath();
            curFile = fileName;
            statusBar()->showMessage(tr("File loaded"), 2000);

            QString code = file.readAll();
            //    extractOptions(code);
            //    interpretGcode(code);
//            refreshLayerNames(code);
            ui->txtGCode->setPlainText(code);

            SVGscene->clear();
            QString picPath = QFileInfo(fileName).absoluteFilePath();
            picPath.chop(5);        //cut .gcode
            picPath.append("svg");
            QGraphicsSvgItem *item = new QGraphicsSvgItem(picPath);
            SVGscene->addItem(item);
            ui->graphicsView->setEnabled(true);
            //    ui->graphicsView->fitInView(item);
            //    fitgraphicsView();

            if(serialConn->isConnected())
                ui->grpGCExecActions->setEnabled(!ui->txtGCode->toPlainText().isEmpty());
            ui->lbGCfilename->setText(fileName);
        }
    }
}

void SphereUI::on_btnGCsave_clicked(void) {
    if(curFile.isEmpty()) {
        if(!curDir.isEmpty())
        {
            curFile = QFileDialog::getSaveFileName(this,"",curDir);
        }
        else
        {
            curFile = QFileDialog::getSaveFileName(this);
        }

    }
    QFile file(curFile);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(curFile)
                             .arg(file.errorString()));
    } else {
        QTextStream out(&file);
        ui->lbGCfilename->setText(curFile);
        out<<ui->txtGCode->toPlainText();
        statusBar()->showMessage(tr("File saved"), 2000);
        ui->btnGCsave->setEnabled(false);
    }
}

void SphereUI::on_btnExecPrint_clicked(void) {
    if(uiState == stIdle) { // Go Print !
        setState(stSending);
    } else { // Go stopped
        setState(stIdle);
        ui->progbarGCode->setValue(0);
    }
}

void SphereUI::on_btnExecPause_clicked(void) {
    if(uiState == stSending) {
        setState(stStopped);
    } else if(uiState == stStopped) {
        setState(stSending);
    }
}

void SphereUI::finishedTransmission(void)
{
    disconnectTranceiver();
    Transceiver.resetState();
    setState(stIdle);
    statusBar()->showMessage(tr("File successfully sent"));
    layerIndex = 0;
//    SetBotToHomePosition();
    QMessageBox* restartLayerMsgBox = new QMessageBox(QMessageBox::Information,
                                             "Restart?",
                                             "Do you want to restart the print?",
                                             QMessageBox::Yes|QMessageBox::No);
    if (QMessageBox::Yes == restartLayerMsgBox->exec())
    {
        on_btnExecPrint_clicked();
    }
}

void SphereUI::interpretSentString(QString string)
{
    if(uiState == stSending)        //if currently sending
    {

        QStringList list = string.split(" ");
//        qDebug()<<"string is :  "<<string<<" "<<list.size();
        for(int i = 0;i<list.size();i++)
        {
            if(!list[i].isEmpty())
            {
                if (list[i].startsWith('X'))
                {
                    //qDebug()<<"setting eggslidervalue: ";
                    ui->spinMotorEgg->setValue(list[i].remove(0,1).toDouble());
                }
                else if (list[i].startsWith('Y'))
                {
                    //qDebug()<<"setting penslidervalue";
                    ui->spinMotorPen->setValue(list[i].remove(0,1).toDouble());
                }
                else if (list[i].startsWith('M'))
                {
                    if(list[i].remove(0,1) == "300")
                    {
//                        qDebug()<<"setting servoSlider";
                        ui->spinServoPen->setValue(list[i+1].remove(0,1).toDouble());
                    }
                    else if(list[i].remove(0,1) == "400")
                    {
                        // qDebug()<<"setting diameterSlider";
                        ui->spinDiam->setValue(list[i+1].remove(0,1).toDouble());
                    }
                }
                else if (list[i].startsWith('F'))
                {
                    //qDebug()<<"setting servoFeedrateSlider";
                    ui->spinFeed->setValue(list[i].remove(0,1).toDouble());
                }
            }
        }
    }
}

void SphereUI::connectTranceiver()
{
    connect(serialConn->getPort(),SIGNAL(readyRead()),(&Transceiver),SLOT(sendNext()));
}

void SphereUI::disconnectTranceiver()
{
    disconnect(serialConn->getPort(),SIGNAL(readyRead()),(&Transceiver),SLOT(sendNext()));
}

void SphereUI::finishedLayer()
{
    if(layerNames.size() > 1)
    {
        layerIndex++;
    }
    qDebug()<<"layerIndex: "<<layerIndex;
    qDebug()<<"layerNames: "<<layerNames;
    qDebug()<<"layerNames.size(): "<<layerNames.size();
    if(layerIndex < layerNames.size())      //next layer
    {
        QMessageBox* nextLayerMsgBox = new QMessageBox(QMessageBox::Information,
                                                                         "Next Layer",
                                                                         "The Layer has been finished!\nplease insert the tool for the layer: " + QString::number(layerIndex),
                                                                         QMessageBox::Yes|QMessageBox::No);
        nextLayerMsgBox->setButtonText(QMessageBox::Yes,"OK");
        nextLayerMsgBox->setButtonText(QMessageBox::No,"Abort");
        nextLayerMsgBox->setText("Please change the tool for layer: " + layerNames[layerIndex]);
//        SetBotToHomePosition();
        switch(nextLayerMsgBox->exec())
        {
            case(QMessageBox::Yes):
            break;
            default:
            setState(stIdle);    //abort
            break;
        }

    }
}

void SphereUI::setState(uiStates state) {
    if(state == stIdle) {
        uiState = state;
        UIserialConnected(serialConn->isConnected());
        ui->pgSettings->setEnabled(true);

        ui->btnExecPause->setEnabled(false);
        ui->btnExecPrint->setText("Print");

        ui->progbarGCode->setEnabled(false);

        disconnectTranceiver();
        Transceiver.resetState();
    } else if (state == stSending) {

        ui->grpMotors->setEnabled(false);
        ui->grpParams->setEnabled(false);
        ui->grpPen->setEnabled(false);
        ui->grpSerialComm->setEnabled(false);
        ui->pgSettings->setEnabled(false);

        ui->btnExecPause->setEnabled(true);
        ui->btnExecPrint->setText("Abort");
        ui->btnExecPause->setText("Pause");

        ui->progbarGCode->setEnabled(true);
        if(uiState == stIdle) {
            Transceiver.set(ui->txtGCode->toPlainText(),(*serialConn));
            Transceiver.run();
            statusBar()->showMessage(tr("Sending File"));
        } else {
            Transceiver.sendNext();
        }
        connectTranceiver();
        uiState = state;
    } else if (state == stStopped) {
        uiState = state;
        ui->btnExecPause->setText("Continue");
        disconnectTranceiver();
    } else
        qDebug() << "Unknown state: " << state;

}

void SphereUI::refreshSendProgress(int value)
{
    ui->progbarGCode->setValue(value);
}
    /* Settings view ****************************/
void SphereUI::resetSettings(void) {
    ui->spinBaud->setValue(115200);

    ui->spinMinDiam->setValue(25);
    ui->spinMaxDiam->setValue(100);
    ui->spinDiam->setRange(ui->spinMinDiam->value(),ui->spinMaxDiam->value());
    ui->spinDiam->setValue(45);
    ui->spinMinFeed->setValue(1000);
    ui->spinMaxFeed->setValue(6000);
    ui->spinFeed->setRange(ui->spinMinFeed->value(),ui->spinMaxFeed->value());
    ui->spinFeed->setValue(3000);

    ui->spinMotorPenMin->setValue(-20);
    ui->spinMotorPenMax->setValue(20);
    ui->spinMotorPenZero->setRange(ui->spinMotorPenMin->value(),ui->spinMotorPenMax->value());
    ui->spinMotorPenZero->setValue(0);
    ui->spinMotorEggMin->setValue(-450);
    ui->spinMotorEggMax->setValue(450);
    ui->spinMotorEggZero->setRange(ui->spinMotorEggMin->value(),ui->spinMotorEggMax->value());
    ui->spinMotorEggZero->setValue(0);

    ui->spinServoMin->setValue(0);
    ui->spinServoMax->setValue(180);
    ui->spinPenUp->setValue(25);
    ui->spinPenDown->setValue(40);
}

void SphereUI::on_btnApplySettings_clicked(void) {
    serialConn->setBaudrate(ui->spinBaud->value());
    ui->spinDiam->setRange(ui->spinMinDiam->value(),ui->spinMaxDiam->value());
    ui->spinFeed->setRange(ui->spinMinFeed->value(),ui->spinMaxFeed->value());
    ui->spinMotorPen->setRange(ui->spinMotorPenMin->value(),ui->spinMotorPenMax->value());
    ui->slideMotorPen->setRange(ui->spinMotorPenMin->value(),ui->spinMotorPenMax->value());
    ui->spinMotorEgg->setRange(ui->spinMotorEggMin->value(),ui->spinMotorEggMax->value());
    ui->slideMotorEgg->setRange(ui->spinMotorEggMin->value(),ui->spinMotorEggMax->value());
    ui->spinServoPen->setRange(ui->spinServoMin->value(),ui->spinServoMax->value());
    ui->slideServoPen->setRange(ui->spinServoMin->value(),ui->spinServoMax->value());
}
