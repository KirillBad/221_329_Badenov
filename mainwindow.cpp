#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QFileDialog>
#include <QBuffer>
#include <openssl/evp.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    filePath = "C:/Users/user/Desktop/221_329_Badenov/221_329_Badenov/transactionsDecrypted.txt";
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::decryptQByteArray(const QByteArray& encryptedBytes, QByteArray& decryptedBytes, unsigned char *key)
{
    QByteArray iv_hex("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f");
    QByteArray iv_ba = QByteArray::fromHex(iv_hex);

    unsigned char iv[16] = {0};
    memcpy(iv, iv_ba.data(), 16);

    EVP_CIPHER_CTX *ctx;
    ctx = EVP_CIPHER_CTX_new();
    if (!EVP_DecryptInit_ex2(ctx, EVP_aes_256_cbc(), key, iv, NULL)) {
        qDebug() << "Error";
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    qDebug() << "NoError";

#define BUF_LEN 256
    unsigned char encrypted_buf[BUF_LEN] = {0}, decrypted_buf[BUF_LEN] = {0};
    int encr_len, decr_len;

    QDataStream encrypted_stream(encryptedBytes);

    decryptedBytes.clear();
    QBuffer decryptedBuffer(&decryptedBytes);
    decryptedBuffer.open(QIODevice::ReadWrite);


    encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
    while(encr_len > 0){

        if (!EVP_DecryptUpdate(ctx, decrypted_buf, &decr_len, encrypted_buf, encr_len)) {
            qDebug() << "Error";
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }

        decryptedBuffer.write(reinterpret_cast<char*>(decrypted_buf), decr_len);
        encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
    }

    int tmplen;
    if (!EVP_DecryptFinal_ex(ctx, decrypted_buf, &tmplen)) {
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    decryptedBuffer.write(reinterpret_cast<char*>(decrypted_buf), tmplen);
    EVP_CIPHER_CTX_free(ctx);

    decryptedBuffer.close();
    return 0;
}

bool MainWindow::decryptTextFile(const QString &filePath, unsigned char *key)
{
    QFile encryptedFile(filePath);

    if (!encryptedFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось открыть файл: " + encryptedFile.errorString());
        return false;
    }

    QByteArray hexEncryptedBytes = encryptedFile.readAll();

    QByteArray encryptedBytes = QByteArray::fromHex(hexEncryptedBytes);

    QByteArray decryptedBytes;

    int ret_code = decryptQByteArray(encryptedBytes, decryptedBytes, key);

    if (ret_code != 0) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось расшифровать данные.");
        encryptedFile.close();
        return false;
    }

    qDebug() << decryptedBytes;

    encryptedFile.close();

    readTransactions(decryptedBytes);

    ui->stackedWidget->setCurrentIndex(1);

    return true;
}

QString MainWindow::calculateHash256(const QStringList &data) {
    QString combinedString = data.join("");
    QByteArray byteArray = combinedString.toUtf8();
    QByteArray hash = QCryptographicHash::hash(byteArray, QCryptographicHash::Sha256);
    QString hashString = hash.toHex();

    return hashString;
}

void MainWindow::readTransactions(const QByteArray &data)
{
    ui->tableWidget->setRowCount(0);

    QTextStream in(data);
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
}

void MainWindow::on_openButton_clicked()
{
    this->filePath = QFileDialog::getOpenFileName(this, tr("Открыть файл"), "", tr("Text Files (*.txt);;All Files (*)"));
    if (!filePath.isEmpty()) {
        ui->stackedWidget->setCurrentIndex(0);
    }
}

void MainWindow::on_authButton_clicked()
{
    QByteArray hash = QCryptographicHash::hash(ui->linePincode->text().toUtf8(), QCryptographicHash::Sha256);

    unsigned char hash_key[32] = {0};
    memcpy(hash_key, hash.data(), 32);

    decryptTextFile(this->filePath, hash_key);

    ui->linePincode->setText("");

}

bool is_sum_valid, is_address_valid, is_data_valid;


void MainWindow::on_lineEditSum_textChanged(const QString &arg1)
{
    QString input = arg1.trimmed();

    is_sum_valid = false;

    if (input.length() == 7) {
        for (const QChar &ch : input) {
            if (!ch.isDigit()) {
                is_sum_valid = false;
                break;
            }
            else {
                is_sum_valid = true;
            }
        }
    }

    ui->confirmData->setEnabled(is_sum_valid && is_address_valid && is_data_valid);
}

void MainWindow::on_lineEditAddress_textChanged(const QString &arg1)
{
    QString input = arg1.trimmed();

    is_address_valid = false;

    if (input.length() == 6) {
        for (const QChar &ch : input) {
            if (!ch.isDigit()) {
                is_address_valid = false;
                break;
            }
            else {
                is_address_valid = true;
            }
        }
    }

    ui->confirmData->setEnabled(is_sum_valid && is_address_valid && is_data_valid);
}


void MainWindow::on_lineEditData_textChanged(const QString &arg1)
{
    is_data_valid = false;

    QString input = arg1.trimmed();

    if (input.length() == 19) {
        if (input[4] == '.' && input[7] == '.' && input[10] == '_' && input[13] == ':' && input[16] == ':') {
            is_data_valid = true;
        }
    }

    ui->confirmData->setEnabled(is_sum_valid && is_address_valid && is_data_valid);
}

void MainWindow::on_lineEditSum_editingFinished()
{
    int length = ui->lineEditSum->text().length();
    for(int i = 0; i < (7 - length); ++i)
    {
        ui->lineEditSum->setText("0" + ui->lineEditSum->text());
    }
}

void MainWindow::on_addButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(2);
}
