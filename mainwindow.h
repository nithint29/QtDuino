#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    int box[3] = {0,0,0};

private slots:
    void on_R1Button_clicked();

    void on_R2Button_clicked();

    void on_R3Button_clicked();

    void on_setAllButton_clicked();
    void setValue(int currVal, int target, int pinNum);
    void delay(int ms);


  //  void ClearScreen(void);
  //  void GetTextCursor(int *x, int *y);
  //  void MoveCursor(int x, int y);
  //  void stepMotorUp(int amount, int *boxcount, int boxPin);
  //  void stepMotorDown(int amount, int *boxcount, int boxPin);
  //  void setBox(int *boxcount, int boxPin, int target);
  //  void setOptOptions(int *interval, int *box1dist, int *box2dist, int *box3dist);



    void on_startCounter_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *arduino;
    static const quint16 unoVendorId = 9025;
    static const quint16 unoProductId = 1;
    QString portName;
    bool isAvailable;

    QVector<double> qX,qY;
};

#endif // MAINWINDOW_H
