#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#define countPin  2     //to count incoming pulses using interrupt
#define r1Pin  4
#define r2Pin  5
#define r3Pin  6
#define SIGNPIN  7
#define counterMode 'C'
#define stepperMode 'S'

//QSerialPort serial;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
      ui->setupUi(this);

      isAvailable = false;
      portName = "";
      arduino = new QSerialPort;
      qDebug() << "Number of available Ports:" << QSerialPortInfo::availablePorts().length();

      foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
          qDebug() << "Has Vendor ID: "<< serialPortInfo.hasVendorIdentifier();
          if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier())
          {
              if(serialPortInfo.vendorIdentifier() == unoVendorId)
              {
                  if(serialPortInfo.productIdentifier() == unoProductId)
                  {
                      portName = serialPortInfo.portName();
                      isAvailable = true;
                  }
              }

          }
      }

      if(isAvailable)
      {
          arduino->setPortName(portName);               //set up port settings
          arduino->open(QSerialPort::ReadWrite);
          arduino->setBaudRate(QSerialPort::Baud9600);
          arduino->setDataBits(QSerialPort::Data8);
          arduino->setParity(QSerialPort::NoParity);
          arduino->setStopBits(QSerialPort::OneStop);
          arduino->setFlowControl(QSerialPort::NoFlowControl);
      }
      else{
          QMessageBox::warning(this,"Port error","No Arduino Attached");
      }

//    serial.setPortName("COM1");
//    serial.setBaudRate(QSerialPort::Baud9600);
//    serial.setDataBits(QSerialPort::Data8);
//    serial.setParity(QSerialPort::NoParity);
//    serial.setStopBits(QSerialPort::OneStop);
//    serial.setFlowControl(QSerialPort::NoFlowControl);
//    serial.open(QIODevice::ReadWrite);
//    //connect(serial,SIGNAL(readyRead()),this,SLOT(on_R1Button_clicked()));
//    serial.write("HelloWorld");
//    if (!serial.open(QIODevice::ReadWrite))
//    {
//        qDebug("false") ;
//    }

}

MainWindow::~MainWindow()
{
    if(arduino->isOpen())
    {
        arduino->close();
    }
    delete ui;
}

void MainWindow::setValue(int currVal, int target, int pinNum)
{
    //int diff = target-currVal;
    QByteArray buffer =QByteArray::number(target-currVal) + "\n";
    buffer.prepend(QByteArray::number(pinNum)+",");
    buffer.prepend(stepperMode);

    if(arduino->isWritable())
    {
        arduino->write(buffer);
        delay(300);                                      //to read serial data properly
        QByteArray serialData = arduino->readAll();
        QString temp = QString::fromStdString(serialData.toStdString());
        qDebug()<<temp;
    }
    else
    {
        qDebug()<<"Couldnt write to serial";
    }
}

void MainWindow::on_R1Button_clicked()
{

    int setR1 =ui->R1in->toPlainText().toInt();
    setValue(box[0],setR1,r1Pin);
    box[0] = setR1;
    ui->R1out->display(box[0]);
}

void MainWindow::on_R2Button_clicked()
{
    int setR2 =ui->R2in->toPlainText().toInt();
    setValue(box[1],setR2,r2Pin);
    box[1] = setR2;
    ui->R2out->display(box[1]);
}

void MainWindow::on_R3Button_clicked()
{
    int setR3 =ui->R3in->toPlainText().toInt();
    setValue(box[2],setR3,r3Pin);
    box[2] = setR3;
    ui->R3out->display(box[2]);
}


void MainWindow::on_setAllButton_clicked()
{
    on_R1Button_clicked();
    on_R2Button_clicked();
    on_R3Button_clicked();
}

void MainWindow::delay(int ms)          //delay method (basically sleep)
{
    QTime dieTime= QTime::currentTime().addMSecs(ms);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
//void MainWindow::setCounter()
//{

//}

void MainWindow::on_startCounter_clicked()      //only works for R1 right now and need to keep track of box value
{
    arduino->clear();
    qX.clear();qY.clear();      //clear any data

    int range =ui->stepRange->toPlainText().toInt();        //read settings from gui
    int points = ui->pointNum->toPlainText().toInt();
    int stepTime = ui->pointDelay->toPlainText().toInt();
    int stepsPerPoint = range/points;
    int boxNum = ui->pinSelect->value();

    QByteArray buffer =QByteArray::number(points) + "\n";       //data to send to arduino
    buffer.prepend(QByteArray::number(stepTime)+",");
    buffer.prepend(QByteArray::number(stepsPerPoint)+",");
    buffer.prepend(QByteArray::number(boxNum+3)+",");
    buffer.prepend('C');
    arduino->write(buffer); //Write the counter settings to arduino

    //arduino->clear();
    delay(200);
    QByteArray serialData =arduino->readLine(1024);
    QString temp = QString::fromStdString(serialData.toStdString());
    qDebug()<<temp;     //check arduino output
    //arduino->clear();

    qDebug()<<stepsPerPoint;


    //begin receiving data and graphing
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(qX,qY);
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    //ui->customPlot->xAxis->setRange(-1, range);
    //ui->customPlot->yAxis->setRange(-10, 100);
    ui->customPlot->replot();
    ui->customPlot->update();

    for(int i = 1;i<=points;i++)
    {
        delay(1000+stepTime);           //delay to ensure arduino's response arrived (could make it a function of pulseWidth)
        int x = i*stepsPerPoint;

        QByteArray yData = arduino->readLine(6);            //read data from arduino line by line
        temp =QString::fromStdString(yData.toStdString());
        int y =temp.toInt();                                //y is the count data from open collector pulses
        qDebug()<<y;

        qX.append(x);
        qY.append(y);
        ui->customPlot->graph(0)->setData(qX,qY);
        ui->customPlot->rescaleAxes();
        ui->customPlot->replot();
        ui->customPlot->update();
    }
    qX.append(500.0);       //testing (remove later)
    qY.append(100);
    ui->customPlot->graph(0)->setData(qX,qY);
    ui->customPlot->rescaleAxes();
    ui->customPlot->replot();
    ui->customPlot->update();
    arduino->clear();




//    QVector<double> x(points), y(points); // initialize with entries 0..100

//    QFile file("hi.txt");
//    if(!file.exists())
//    {
//        qDebug()<<"No File";
//    }
//    else
//    {
//        if(!file.open(QIODevice::ReadOnly)) {
//            QMessageBox::information(0, "error", file.errorString());
//        }


//        QTextStream in(&file);
//        int i = 0;
//        while(!in.atEnd()) {

//            y[i] =in.readLine().toDouble();

//            i++;
//        }
//        qDebug()<<"got here";

//        file.close();

//        ui->customPlot->addGraph();
//        ui->customPlot->graph(0)->setData(x, y);

//        ui->customPlot->xAxis->setLabel("x");
//        ui->customPlot->yAxis->setLabel("y");

//        ui->customPlot->xAxis->setRange(-1, 10);
//        ui->customPlot->yAxis->setRange(0, 10);
//        ui->customPlot->replot();
//    }

}
