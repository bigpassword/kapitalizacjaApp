#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>
#include <vector>

#include <QIntValidator>
#include <QDoubleValidator>

#include <QtCharts>
#include <QLineSeries>
#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QPieSlice>
#include <QFrame>
#include <QFrame>

#include <QListView>
#include <QStringList>
#include <QStringListModel>

typedef struct Kapital {
    double kapitalPoczatkowy;

    double kapitalKoncowy;
    int liczbaKapitalizacji;
    int czasTrwania;

    double stopaProcentowa;

    std::vector<double> kapitalInTime;
} Kapital;

void kapitalizacjaOdsetek(Kapital* kapital);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // disable resize
    //setWindowFlag(Qt::MSWindowsFixedSizeDialogHint, true);

    // setup lineEdit validators
    //QIntValidator *intValidator = new QIntValidator(1, 999999999, this);
    QDoubleValidator *doubleValidator = new QDoubleValidator(0.01, 999999999, 2, this);
    QDoubleValidator *percentValidator = new QDoubleValidator(0.0001, 999999999, 4, this);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    percentValidator->setNotation(QDoubleValidator::StandardNotation);

    ui->lineEdit_1->setValidator(doubleValidator);
    ui->lineEdit_2->setValidator(percentValidator);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void kapitalizacjaOdsetek(Kapital* kapital) {
    int iloscKapitalizacji = kapital->liczbaKapitalizacji * kapital->czasTrwania;
    double tempKapital, stopaOkresu = 1+(kapital->stopaProcentowa / kapital->liczbaKapitalizacji);

    kapital->kapitalInTime.push_back(kapital->kapitalPoczatkowy);
    for (int i=0; i<iloscKapitalizacji; i++) {
        tempKapital = kapital->kapitalPoczatkowy * std::pow(stopaOkresu, i+1);
        kapital->kapitalInTime.push_back(tempKapital);
    }

    //kapital->kapitalKoncowy = kapital->kapitalPoczatkowy * std::pow(stopaOkresu, iloscKapitalizacji);
    kapital->kapitalKoncowy = kapital->kapitalInTime.back();
}

void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
    QLabel* yearLabel = ui->label_7;

    switch (arg1) {
    case 1:
        yearLabel->setText("rok");
        break;
    case 12:
    case 13:
    case 14:
        yearLabel->setText("lat");
        break;
    default:
        switch (arg1 % 10) {
        case 2:
        case 3:
        case 4:
            yearLabel->setText("lata");
            break;
        default:
            yearLabel->setText("lat");
            break;
        }
        break;
    }
}

// === global variables for kapital ===
Kapital* kapital{};
// chart
QLineSeries* lineSeries{};
QChart* lineChart{};
QChartView* lineChartView{};
// pie
QPieSeries* pieSeries{};
QChart* pieChart{};
QChartView* pieChartView{};
// list
QStringList* kapitalList{};
QStringListModel* kapitalListModel{};

void MainWindow::on_buttonCalculate_clicked()
{
    // check if any input has 0
    if (!ui->lineEdit_1->text().toDouble() + !ui->spinBox_1->value() + !ui->spinBox_2->value() + !ui->lineEdit_2->text().toDouble()) {
        return;
    }

    delete kapital;
    kapital = new Kapital;

    // get data from user
    kapital->kapitalPoczatkowy = ui->lineEdit_1->text().toDouble();
    kapital->liczbaKapitalizacji = ui->spinBox_1->value();
    kapital->czasTrwania = ui->spinBox_2->value();
    kapital->stopaProcentowa = ui->lineEdit_2->text().toDouble()/100;

    kapitalizacjaOdsetek(kapital);

    // graph the data from the function
    delete lineSeries;
    lineSeries = new QLineSeries();

    for (int i=0; i<(kapital->liczbaKapitalizacji * kapital->czasTrwania + 1); i++) {
        lineSeries->append(i, kapital->kapitalInTime[i]);
    }

    delete lineChart;
    lineChart = new QChart();

    lineChart->addSeries(lineSeries);
    lineChart->legend()->hide();
    lineChart->createDefaultAxes();
    lineChart->setAnimationOptions(QChart::AllAnimations);

    // remove margins
    lineChart->layout()->setContentsMargins(0, 0, 0, 0);
    lineChart->setBackgroundVisible(false);

    delete lineChartView;
    lineChartView = new QChartView(lineChart);
    lineChartView->setRenderHint(QPainter::Antialiasing);

    // display in line chart view
    QLayout* lineChartLayout = ui->lineChartView->layout();

    if (!lineChartLayout->isEmpty()) {
        lineChartLayout->removeWidget(lineChartLayout->takeAt(0)->widget());
    }
    lineChartLayout->addWidget(lineChartView);

    // pie chart of profits
    // check if pie chart exists, if not create one
    if (!pieChartView) {
        pieSeries = new QPieSeries;
        pieSeries->append("Kapitał początkowy", kapital->kapitalPoczatkowy);
        pieSeries->append("Różnica", kapital->kapitalKoncowy - kapital->kapitalPoczatkowy);

        // make the hole
        pieSeries->setHoleSize(0.4);
        //pieSeries->setLabelsVisible();

        pieChart = new QChart;
        pieChart->addSeries(pieSeries);
        pieChart->legend()->hide();
        pieChart->setAnimationOptions(QChart::AllAnimations);

        // remove margins
        pieChart->layout()->setContentsMargins(0, 0, 0, 0);
        pieChart->setBackgroundVisible(false);

        pieChartView = new QChartView(pieChart);
        pieChartView->setRenderHint(QPainter::Antialiasing);

        // display in pie chart view
        ui->pieChartView->layout()->addWidget(pieChartView);

    // if pie chart exists, update it with new data
    } else {
        pieSeries->clear();

        pieSeries->append("Kapitał początkowy", kapital->kapitalPoczatkowy);
        pieSeries->append("Różnica", kapital->kapitalKoncowy - kapital->kapitalPoczatkowy);
    }

    // display capital change next to donut
    ui->label_12->setText(QString::number(kapital->kapitalPoczatkowy, 10, 2));
    ui->label_13->setText(QString::number(kapital->kapitalKoncowy, 10, 2));
    ui->label_14->setText(QString::number(kapital->kapitalKoncowy - kapital->kapitalPoczatkowy, 10, 2));

    // generate list
    delete kapitalList;
    kapitalList = new QStringList;

    QString tempLine;
    for (int i=0; i<(kapital->liczbaKapitalizacji * kapital->czasTrwania); i++) {
        tempLine = QString("%1. Kapitał: %2 + %3 = %4 PLN").arg(i+1).arg(kapital->kapitalInTime[i]).arg(kapital->kapitalInTime[i+1] - kapital->kapitalInTime[i]).arg(kapital->kapitalInTime[i+1]);
        kapitalList->append(tempLine);
    }

    delete kapitalListModel;
    kapitalListModel = new QStringListModel;

    kapitalListModel->setStringList(*kapitalList);
    ui->listView->setModel(kapitalListModel);
}

