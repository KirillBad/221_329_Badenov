#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString filePath = "C:/Users/user/Desktop/221_329_Badenov/221_329_Badenov/transactions.txt";

    readTransactions(filePath);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readTransactions(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QTextStream in(&file);
    QStringList transactionData;
    int row = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed(); // Читаем строку и удаляем лишние пробелы
        if (!line.isEmpty()) {
            transactionData.append(line);
        } else {
            // Если прочитали пустую строку, то добавляем данные в таблицу
            if (transactionData.size() == 3) {
                ui->tableWidget->insertRow(row);
                for (int col = 0; col < 3; ++col) {
                    QTableWidgetItem *item = new QTableWidgetItem(transactionData.at(col));
                    qDebug() << transactionData.at(col);
                    ui->tableWidget->setItem(row, col, item);
                }
                ++row;
                transactionData.clear();
            }
        }
    }

    file.close();
}
