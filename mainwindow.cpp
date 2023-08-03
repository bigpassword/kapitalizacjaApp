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
    bool okresMiesiace;
    int czasTrwania;
    int liczbaKapitalizacji;

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

    // change locale
    QLocale::setDefault(QLocale("pl_PL"));

    // setup lineEdit validators
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
    if (kapital->okresMiesiace) {
        stopaOkresu = ((stopaOkresu - 1) / 12) + 1;
    }

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

void MainWindow::on_spinBox_okresTrwania_valueChanged(int arg1)
{
    QComboBox* periodBox = ui->comboBoxPeriod;

    switch (arg1) {
    case 1:
        periodBox->setItemText(0, "rok");
        periodBox->setItemText(1, "miesiąc");
        break;
    case 12:
    case 13:
    case 14:
        periodBox->setItemText(0, "lat");
        periodBox->setItemText(1, "miesięcy");
        break;
    default:
        switch (arg1 % 10) {
        case 2:
        case 3:
        case 4:
            periodBox->setItemText(0, "lata");
            periodBox->setItemText(1, "miesiące");
            break;
        default:
            periodBox->setItemText(0, "lat");
            periodBox->setItemText(1, "miesięcy");
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
    QLocale locale = QLocale("pl_PL");

    // check if any input has 0
    if (!locale.toDouble(ui->lineEdit_kapitalPocz->text()) + !ui->spinBox_liczbaKap->value() + !ui->spinBox_okresTrwania->value() + !locale.toDouble(ui->lineEdit_stopaProc->text())) {
        return;
    }

    delete kapital;
    kapital = new Kapital;

    // get data from user
    kapital->kapitalPoczatkowy = locale.toDouble(ui->lineEdit_kapitalPocz->text());
    kapital->okresMiesiace = ui->comboBoxPeriod->currentIndex();
    kapital->czasTrwania = ui->spinBox_okresTrwania->value();
    kapital->liczbaKapitalizacji = ui->spinBox_liczbaKap->value();
    kapital->stopaProcentowa = locale.toDouble(ui->lineEdit_stopaProc->text())/100;

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
        pieSeries->append("Zysk", kapital->kapitalKoncowy - kapital->kapitalPoczatkowy);
        pieSeries->append("Podatek", kapital->podatekKoncowy);

        // make the hole
        pieSeries->setHoleSize(0.4);
        //pieSeries->setLabelsVisible();

        pieChart = new QChart;
        pieChart->addSeries(pieSeries);
        //pieChart->legend()->hide();
        pieChart->legend()->setAlignment(Qt::AlignLeft);
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
        pieSeries->append("Zysk", kapital->kapitalKoncowy - kapital->kapitalPoczatkowy);
        pieSeries->append("Podatek", kapital->podatekKoncowy);
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
    if (kapital->okresMiesiace) {
        tableHeaders << "Miesiąc" << "Kapitał" << "Kapitalizacja" << "Podatek" << "Opodatkowane" << "Suma";
    } else {
        tableHeaders << "Rok" << "Miesiąc" << "Kapitał" << "Kapitalizacja" << "Podatek" << "Opodatkowane" << "Suma";
    }

    tableWidget->setColumnCount(tableHeaders.size());
    tableWidget->setHorizontalHeaderLabels(tableHeaders);

    int rowCount = kapital->liczbaKapitalizacji * kapital->czasTrwania;
    tableWidget->setRowCount(rowCount);
    for (int i=0; i<rowCount; i++) {
        int j = 0;

        tableItem = new QTableWidgetItem(QString::number(i / kapital->liczbaKapitalizacji + 1));
        tableItem->setTextAlignment(Qt::AlignCenter);
        tableWidget->setItem(i, j, tableItem);
        j++;

        if (!kapital->okresMiesiace) {
            tableItem = new QTableWidgetItem(months[((i % kapital->liczbaKapitalizacji) * 12/kapital->liczbaKapitalizacji + ui->comboBoxMonths->currentIndex()) % 12]);
            tableWidget->setItem(i, j, tableItem);
            j++;
        }

        tableItem = new QTableWidgetItem(QString::number(kapital->kapitalInTime[i], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, j, tableItem);
        j++;

        tableItem = new QTableWidgetItem(QString::number(kapital->kapitalInTime[i+1] - kapital->kapitalInTime[i] + kapital->podatekInTime[i+1], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, j, tableItem);
        j++;

        tableItem = new QTableWidgetItem(QString::number(kapital->podatekInTime[i+1], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, j, tableItem);
        j++;

        tableItem = new QTableWidgetItem(QString::number(kapital->kapitalInTime[i+1] - kapital->kapitalInTime[i], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, j, tableItem);
        j++;

        tableItem = new QTableWidgetItem(QString::number(kapital->kapitalInTime[i+1], 'f', 2));
        tableItem->setTextAlignment(Qt::AlignRight);
        tableWidget->setItem(i, j, tableItem);
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


void MainWindow::on_comboBoxPeriod_currentIndexChanged(int index)
{
    QComboBox* monthBox = ui->comboBoxMonths;
    QLabel* capLabel = ui->label_liczbaKap;

    if (index) {
        monthBox->setEnabled(false);
        capLabel->setText("/ miesiąc");
    } else {
        monthBox->setEnabled(true);
        capLabel->setText("/ rok");
    }
}

