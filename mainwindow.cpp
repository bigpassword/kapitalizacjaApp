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
    const double podatek = 0.1; /* 10% */

    double kapitalPoczatkowy;
    int liczbaKapitalizacji;
    int czasTrwania;

    double kapitalKoncowy;
    double podatekKoncowy = 0;

    double stopaProcentowa;

    std::vector<double> kapitalInTime;
    std::vector<double> podatekInTime;
} Kapital;

QString months[] = {"Styczneń", "Luty", "Marzec", "Kwiecień", "Maj", "Czerwiec", "Lipiec", "Sierpień", "Wrzesień", "Październik", "Listopad", "Grudzień"};

void kapitalizacjaOdsetek(Kapital* kapital);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // setup lineEdit validators
    //QIntValidator *intValidator = new QIntValidator(1, 999999999, this);
    QDoubleValidator *doubleValidator = new QDoubleValidator(0.01, 999999999, 2, this);
    QDoubleValidator *percentValidator = new QDoubleValidator(0.0001, 999999999, 4, this);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);
    percentValidator->setNotation(QDoubleValidator::StandardNotation);

    ui->lineEdit_kapitalPocz->setValidator(doubleValidator);
    ui->lineEdit_stopaProc->setValidator(percentValidator);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void kapitalizacjaOdsetek(Kapital* kapital) {
    int iloscKapitalizacji = kapital->liczbaKapitalizacji * kapital->czasTrwania;
    double tempKapital, tempPodatek, stopaOkresu = 1+(kapital->stopaProcentowa / kapital->liczbaKapitalizacji);

    kapital->kapitalInTime.push_back(kapital->kapitalPoczatkowy);
    kapital->podatekInTime.push_back(0);
    for (int i=0; i<iloscKapitalizacji; i++) {
        tempKapital = kapital->kapitalInTime[i] * stopaOkresu;
        tempPodatek = (tempKapital - kapital->kapitalInTime[i]) * kapital->podatek;
        tempKapital -= tempPodatek;

        kapital->kapitalInTime.push_back(tempKapital);
        kapital->podatekInTime.push_back(tempPodatek);
        kapital->podatekKoncowy += tempPodatek;
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

void MainWindow::on_buttonCalculate_clicked()
{
    // check if any input has 0
    if (!ui->lineEdit_kapitalPocz->text().toDouble() + !ui->spinBox_liczbaKap->value() + !ui->spinBox_okresTrwania->value() + !ui->lineEdit_stopaProc->text().toDouble()) {
        return;
    }

    delete kapital;
    kapital = new Kapital;

    // get data from user
    kapital->kapitalPoczatkowy = ui->lineEdit_kapitalPocz->text().toDouble();
    kapital->liczbaKapitalizacji = ui->spinBox_liczbaKap->value();
    kapital->czasTrwania = ui->spinBox_okresTrwania->value();
    kapital->stopaProcentowa = ui->lineEdit_stopaProc->text().toDouble()/100;

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
        pieSeries->append("Podatek", kapital->podatekKoncowy);

        // make the hole
        pieSeries->setHoleSize(0.4);
        //pieSeries->setLabelsVisible();

        pieChart = new QChart;
        pieChart->addSeries(pieSeries);
        //pieChart->legend()->hide();
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
    ui->label_kapitalPocz->setText(QString::number(kapital->kapitalPoczatkowy, 'f', 2));
    ui->label_kapitalKon->setText(QString::number(kapital->kapitalKoncowy, 'f', 2));
    ui->label_kapitalRoz->setText(QString::number(kapital->kapitalKoncowy - kapital->kapitalPoczatkowy, 'f', 2));
    ui->label_kapitalPodatek->setText(QString::number(kapital->podatekKoncowy, 'f', 2));


    // table of each step
    // get table widget
    QTableWidget* tableWidget = ui->tableWidget;
    QTableWidgetItem* tableItem{};

    // check if table was used
    if (tableWidget->item(0, 0)) {
        // deallocate and clear items from table
        for (int row=0; row<tableWidget->rowCount(); row++) {
            for (int column=0; column<tableWidget->columnCount(); column++) {
                tableItem = tableWidget->item(row, column);
                delete tableItem;
            }
        }
        tableWidget->clear();
    }

    // generate table
    QStringList tableHeaders;
    tableHeaders << "Rok" << "Miesiąc" << "Kapitał" << "Kapitalizacja" << "Podatek" << "Opodatkowane" << "Suma";

    tableWidget->setColumnCount(tableHeaders.size());
    tableWidget->setHorizontalHeaderLabels(tableHeaders);

    int rowCount = kapital->liczbaKapitalizacji * kapital->czasTrwania;
    tableWidget->setRowCount(rowCount);
    for (int i=0; i<rowCount; i++) {
        tableItem = new QTableWidgetItem(QString::number(i / kapital->liczbaKapitalizacji + 1));
        tableItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(i, 0, tableItem);

        tableItem = new QTableWidgetItem(months[((i % kapital->liczbaKapitalizacji) * 12/kapital->liczbaKapitalizacji + ui->comboBoxMonths->currentIndex()) % 12]);
        tableWidget->setItem(i, 1, tableItem);

        tableItem = new QTableWidgetItem(QString::number(kapital->kapitalInTime[i], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, 2, tableItem);

        tableItem = new QTableWidgetItem(QString::number(kapital->kapitalInTime[i+1] - kapital->kapitalInTime[i] + kapital->podatekInTime[i+1], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, 3, tableItem);

        tableItem = new QTableWidgetItem(QString::number(kapital->podatekInTime[i+1], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, 4, tableItem);

        tableItem = new QTableWidgetItem(QString::number(kapital->kapitalInTime[i+1] - kapital->kapitalInTime[i], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, 5, tableItem);

        tableItem = new QTableWidgetItem(QString::number(kapital->kapitalInTime[i+1], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, 6, tableItem);
    }
    tableWidget->resizeColumnsToContents();
    tableWidget->resizeRowsToContents();

    // set minimum width for columns 3..5
    for (int i=2; i<7; i++) {
        if (tableWidget->columnWidth(i)<71) {
            tableWidget->setColumnWidth(i, 71);
        }
    }

    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
}

