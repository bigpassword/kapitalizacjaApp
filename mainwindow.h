#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QPieSeries>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_spinBox_okresTrwania_valueChanged(int arg1);

    void on_buttonCalculate_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
