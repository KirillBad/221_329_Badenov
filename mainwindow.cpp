#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QFileDialog>

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

QString MainWindow::calculateHash256(const QStringList &data) {
    QString combinedString = data.join("");
    QByteArray byteArray = combinedString.toUtf8();
    QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);
    QString hashString = hash.toHex();

    return hashString;
}

void MainWindow::readTransactions(const QString &filePath)
{
    ui->tableWidget->setRowCount(0);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    QTextStream in(&file);
    QStringList transactionData;
    QString previousHash;

    int row = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty()) {
            transactionData.append(line);
        } else {
            if (transactionData.size() == 4) {

                QString currentHash = transactionData.at(3);
                bool hashMismatch = !previousHash.isEmpty() && (currentHash != previousHash);
                QString calculatedHash = calculateHash256(transactionData.mid(0, 3));

                ui->tableWidget->insertRow(row);
                for (int col = 0; col < 3; ++col) {
                    QTableWidgetItem *item = new QTableWidgetItem(transactionData.at(col));
                    if (hashMismatch) {
                        item->setBackground(Qt::red);
                    }
                    ui->tableWidget->setItem(row, col, item);
                }
                ++row;
                previousHash = calculatedHash;
                transactionData.clear();
            } else if (transactionData.size() == 3) {
                QString currentHash = calculateHash256(transactionData.mid(0, 3));
                ui->tableWidget->insertRow(row);
                for (int col = 0; col < 3; ++col) {
                    QTableWidgetItem *item = new QTableWidgetItem(transactionData.at(col));
                    ui->tableWidget->setItem(row, col, item);
                }
                ++row;
                previousHash = currentHash;
                transactionData.clear();
            }
        }
    }

    file.close();
}

void MainWindow::on_openButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Открыть файл"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (!filePath.isEmpty()) {
        readTransactions(filePath);
    }
}

