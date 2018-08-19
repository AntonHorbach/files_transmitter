#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QFileDialog>
#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QTcpServer* server;
    QTcpSocket* socket;
    QByteArray file_name;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void my_slot();
    void slot_reader();
    void on_transfer_button_clicked();
    void on_server_button_clicked();
    void on_client_button_clicked();
};

#endif // MAINWINDOW_H
