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
#define motorPin1  8
#define motorPin2  9
#define motorPin3  10
#define motorPin4  11

#define pulseWidth 25
int prevCoil = 0;

//QSerialPort serial;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
      ui->setupUi(this);
      ui->progressBar->setTextVisible(false);
      ui->progressBar_2->setTextVisible(false);
      ui->progressBar_3->setTextVisible(false);
      ui->progressBar_4->setTextVisible(false);
      ui->progressBar_5->setTextVisible(false);
      ui->progressBar_6->setTextVisible(false);

      isAvailable = false;
      portName = "";
      arduino = new QSerialPort;

      QSerialPortInfo* info = new QSerialPortInfo;
      QList<QSerialPortInfo> * PortList = new QList<QSerialPortInfo>;
      *PortList = QSerialPortInfo::availablePorts();
      qDebug() << "Number of available Ports:" << PortList->length();
      //QMessageBox::warning(this,"Port Initialized","Arduino Attached");

      foreach(*info, *PortList){
          qDebug() << "Has Vendor ID: "<< info->hasVendorIdentifier();
          if(info->hasVendorIdentifier() && info->hasProductIdentifier())
          {
              //qDebug() << "vendor ID: "<<serialPortInfo.vendorIdentifier();
              //qDebug() << "product ID: "<<serialPortInfo.productIdentifier();
              if(info->vendorIdentifier() == unoVendorId)
              {
                  if(info->productIdentifier() == unoProductId || info->productIdentifier() == unoProductId2)
                  {

                      portName = info->portName();
                      isAvailable = true;
                  }
              }

          }
      }
      delete info;
//      qDebug() << "Number of available Ports:" << QSerialPortInfo::availablePorts().length();
//      QMessageBox::warning(this,"Port Initialized","Arduino Attached");

//      foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
//          qDebug() << "Has Vendor ID: "<< serialPortInfo.hasVendorIdentifier();
//          if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier())
//          {
//              //qDebug() << "vendor ID: "<<serialPortInfo.vendorIdentifier();
//              //qDebug() << "product ID: "<<serialPortInfo.productIdentifier();
//              if(serialPortInfo.vendorIdentifier() == unoVendorId)
//              {
//                  if(serialPortInfo.productIdentifier() == unoProductId || serialPortInfo.productIdentifier() == unoProductId2)
//                  {

//                      portName = serialPortInfo.portName();
//                      isAvailable = true;
//                  }
//              }

//          }
//      }

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


      QFile file("storage.txt");       //Create or load the applications data file
          if(!file.exists())
          {
              QFile file;
              file.setFileName("storage.txt");
              file.open(QIODevice::ReadWrite | QIODevice::Text);
              QTextStream stream(&file);
              stream<<0;
              stream<<"\n";
              stream<<0;
              qDebug()<<"New File created";
              file.close();
          }
          else if (file.open(QIODevice::ReadOnly))
          {
             QTextStream in(&file);
             while (!in.atEnd())
             {
                prevCoil = qAbs(in.readLine().toInt());
                if(prevCoil>3 || prevCoil<0)
                {
                    qDebug()<<"Coil Number in text file is out of bounds";
                    QMessageBox::warning(this,"Data Error","Coil Number in text file is out of bounds");
                }
                box[0] = in.readLine().toInt();
                box[1] = in.readLine().toInt();
                box[2] = in.readLine().toInt();

             }
             file.close();
          }
//          file.close();
          ui->R1out->display(box[0]);
          ui->R2out->display(box[1]);
          ui->R3out->display(box[2]);


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
    saveToFile();
    delete ui;
}

void MainWindow::saveToFile()
{
    QFile file("storage.txt");      //text file to store program/stepper data (previous coil of stepper 1 only)
        if(file.exists())
        {
            file.open(QIODevice::ReadWrite | QIODevice::Text);
            QTextStream stream(&file);
            stream<<prevCoil;
            stream<<"\n";
            stream<<box[0];
            stream<<"\n";
            stream<<box[1];
            stream<<"\n";
            stream<<box[2];
            stream<<"\n";
            //qDebug()<<"written";
            file.close();
        }
}

int MainWindow::setValue(int currVal, int target, int pinNum)
{
    int numSteps = target-currVal;
    bool pos = numSteps>=0;
    //int diff = target-currVal;
    QByteArray buffer =QByteArray::number(numSteps) + "\n";
    buffer.prepend(QByteArray::number(pinNum)+",");
    buffer.prepend(QByteArray::number(prevCoil)+",");
    buffer.prepend(stepperMode);

    if(arduino->isWritable())
    {
        arduino->write(buffer);                            //send command
        //QMessageBox::warning(this,"Port Initialized","Arduino Attached"); //ERROR HERE
        delay(300);                                      //to read serial data properly
        QByteArray serialData = arduino->readLine(20);
        QString temp = QString::fromStdString(serialData.toStdString());
        qDebug()<<temp;

        delay(1000+pulseWidth*abs(numSteps));                                    //look for stop pin response
        QByteArray response = arduino->readLine(6);            //read number of actual steps taken
        QString rSteps = (QString::fromStdString(response.toStdString()));
        int realSteps = rSteps.toInt();
        qDebug()<<realSteps;
        if(realSteps != -1)     //if end of stepper is reached (a switch was triggered)
        {
            qDebug()<<"End reached";
            numSteps = realSteps;
        }
        prevCoil = mod((prevCoil+numSteps),4);
        qDebug()<<prevCoil;

        //update switch status bars
        switch(pinNum)
        {
        case r1Pin:
                if(pos)
                {
                    if(realSteps==-1){ui->progressBar->setValue(0);}
                    else{ui->progressBar->setValue(100);}
                    //Other switch should now be off
                    ui->progressBar_2->setValue(0);
                }
                else{
                    if(realSteps==-1){ui->progressBar_2->setValue(0);}
                    else{ui->progressBar_2->setValue(100);}
                    ui->progressBar->setValue(0);
                }
            break;
        case r2Pin:
                if(pos)
                {
                    if(realSteps==-1){ui->progressBar_3->setValue(0);}
                    else{ui->progressBar_3->setValue(100);}
                    ui->progressBar_4->setValue(0);
                }
                else{
                    if(realSteps==-1){ui->progressBar_4->setValue(0);}
                    else{ui->progressBar_4->setValue(100);}
                    ui->progressBar_3->setValue(0);
                }
            break;
        case r3Pin:
                if(pos)
                {
                    if(realSteps==-1){ui->progressBar_5->setValue(0);}
                    else{ui->progressBar_5->setValue(100);}
                    ui->progressBar_6->setValue(0);
                }
                else{
                    if(realSteps==-1){ui->progressBar_6->setValue(0);}
                    else{ui->progressBar_6->setValue(100);}
                    ui->progressBar_5->setValue(0);
                }
        break;

        }

        return numSteps;
    }
    else
    {
//        prevCoil = mod((prevCoil+target-currVal),4);
//        qDebug()<<prevCoil;
        qDebug()<<"Couldnt write to serial";
        return 0;
    }
}

void MainWindow::on_R1Button_clicked()
{
    int setR1 =ui->R1in->toPlainText().toInt();
    int change = setValue(box[0],setR1,r1Pin);
    box[0] = box[0]+change;
    ui->R1out->display(box[0]);
    //update text file
    saveToFile();
}

void MainWindow::on_R2Button_clicked()
{
    int setR2 =ui->R2in->toPlainText().toInt();
    int change = setValue(box[1],setR2,r2Pin);
    box[1] = box[1]+change;
    ui->R2out->display(box[1]);
    saveToFile();
}

void MainWindow::on_R3Button_clicked()
{
    int setR3 =ui->R3in->toPlainText().toInt();
    int change = setValue(box[2],setR3,r3Pin);
    box[2] = box[2]+change;
    ui->R3out->display(box[2]);
    saveToFile();
}


void MainWindow::on_setAllButton_clicked()
{
    on_R1Button_clicked();
    on_R2Button_clicked();
    on_R3Button_clicked();
}
int MainWindow::mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
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
        delay(1000+stepTime+stepsPerPoint*5);           //delay to ensure arduino's response arrived (could make it a function of pulseWidth)
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
