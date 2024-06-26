#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_openButton_clicked();
    void on_authButton_clicked();
    void on_lineEditSum_textChanged(const QString &arg1);
    void on_lineEditAddress_textChanged(const QString &arg1);
    void on_lineEditData_textChanged(const QString &arg1);
    void on_addButton_clicked();
    void on_lineEditSum_editingFinished();

private:
    Ui::MainWindow *ui;
    QString filePath;
    static int decryptQByteArray(const QByteArray& encryptedBytes, QByteArray& decryptedBytes, unsigned char *key);
    void readTransactions(const QByteArray &data);
    QString calculateHash256(const QStringList &data);
    bool decryptTextFile(const QString &filePath, unsigned char *key);

};
#endif // MAINWINDOW_H
