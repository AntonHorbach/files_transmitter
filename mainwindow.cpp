#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_transfer_button_clicked()
{
    file_data.clear();
    QFile file(QFileDialog::getOpenFileName(this, "Choose file", "."));
    file_name=file.fileName().toLocal8Bit();

    if(!file.open(QFile::ReadOnly)) return;

    file_data=file.readAll();
    socket->write(file_data);
}

void MainWindow::my_slot(){
    socket=server->nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    connect(socket, SIGNAL(readyRead()), SLOT(slot_reader()));
}

void MainWindow::slot_reader()
{
    socket=dynamic_cast<QTcpSocket*>(sender());
    file_name=socket->readAll();
    file_data=socket->readAll();

    QFile file;
    file.setFileName(file_name);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        file.write(file_data);
        file.close();
    }
}

void MainWindow::on_server_button_clicked()
{
    ui->label->setText("U r the server");
    server=new QTcpServer(this);
    server->listen(QHostAddress::Any, 2121);
    connect(server, SIGNAL(newConnection()), SLOT(my_slot()));
}

void MainWindow::on_listener_button_clicked()
{

}
