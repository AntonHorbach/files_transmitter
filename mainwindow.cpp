#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    server = new QTcpServer(this);

    server->listen(QHostAddress::Any, 2121);
    socket->connectToHost("localhost", 2121);

    connect(server, SIGNAL(newConnection()), SLOT(my_slot()));
    connect(socket, SIGNAL(readyRead()), SLOT(slot_reader()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ip_port_edited() {
    QString ip = ui->ipLine->text();
    int port = ui->portLine->text().toInt();

    socket->connectToHost(ip, port);
    server->listen(QHostAddress::Any, port);
}

void MainWindow::on_transfer_button_clicked() {
    QFile file(QFileDialog::getOpenFileName(this, "Choose file", "."));

    {
        QFileInfo file_info(file);
        file_name = file_info.fileName();

        std::string text="Send the "+file_name.toStdString();
        ui->label->setText(QString(text.c_str()));

        QByteArray f_info;
        QDataStream finfo(&f_info, QIODevice::WriteOnly);
        finfo<<file.size();
        finfo<<file_name;

        socket->write(f_info);
        socket->waitForBytesWritten();
    }

    if(!file.open(QFile::ReadOnly)) return;
    QDataStream read(&file);
    char buff[64]{};
    long int bytes = 0;

    while(!read.atEnd()){
      long int num = read.readRawData(buff, sizeof(char) * 64);
      QByteArray data(buff, sizeof(char) * num);

      bytes += socket->write(data, sizeof(char) * num);
      socket->flush();

      if (bytes==-1){
        qDebug() << "Error";
        socket->close();
        return;
      }
    }
}

void MainWindow::my_slot(){
    socket = server->nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    connect(socket, SIGNAL(readyRead()), SLOT(slot_reader()));
}

void MainWindow::slot_reader()
{
    socket = dynamic_cast<QTcpSocket*>(sender());
    quint64 file_size;
    long int bytes_done=0;
    long int bytes=0;

    {
        QByteArray file_info;
        QDataStream f_info(&file_info, QIODevice::ReadWrite);
        file_info = socket->readAll();

        f_info >> file_size;
        f_info >> file_name;

        std::string text = "Get the " + file_name.toStdString();
        ui->label->setText(QString(text.c_str()));
    }

    QFile file(file_name);
    if(file.open(QIODevice::WriteOnly)) return;

    QDataStream write(&file);
    while (bytes_done < file_size){
        bytes = 0;
        while (bytes == 0)
            bytes=socket->waitForReadyRead(-1);

        if (bytes == -1){
            qDebug()<<"Download error";
            socket->close();
            return;
        }

        QByteArray tmp = socket->readAll();
        bytes += write.writeRawData(tmp.data(), tmp.size());
        bytes_done += tmp.size();
    }

    ui->label->setText("Finished!");
    file.close();
}
