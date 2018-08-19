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
    QFile file(QFileDialog::getOpenFileName(this, "Choose file", "."));
    file_name=file.fileName().toLocal8Bit();
    socket->write(file_name);

    QString file_size;
    file_size.number(QFileInfo(file).size());
    socket->write(file_size.toLocal8Bit());

    {
        std::string text="Send the "+file_name.toStdString()+"(size "+file_size.toStdString()+")";
        ui->label->setText(QString(text.c_str()));
        qDebug()<<file.size();
        qDebug()<<file.fileName();
    }

    if(!file.open(QFile::ReadOnly)) return;
    QDataStream read(&file);
    char buff[64]{};
    long int bytes=0;

    while(!read.atEnd()){
      int num = read.readRawData(buff, sizeof(char)*64);
      QByteArray data(buff, sizeof(char)*num);

      bytes += socket->write(data, sizeof(char)*num);
      socket->flush();

      if (bytes==-1){
        qDebug() << "Error";
        socket->close();
        return;
      }
      float precents=(static_cast<float>(bytes) / file_size.toInt()) * 100;
      ui->label->setText(QString::number(precents));
    }
}

void MainWindow::my_slot(){
    socket=server->nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    connect(socket, SIGNAL(readyRead()), SLOT(slot_reader()));
}

void MainWindow::slot_reader()
{
    socket=dynamic_cast<QTcpSocket*>(sender());
    file_name=socket->read(64);
    long int file_size=socket->read(64).toInt();
    long int bytes_done = 0;
    long int bytes=0;

    {
        std::string text="Get the "+file_name.toStdString();
        ui->label->setText(QString(text.c_str()));
        qDebug()<<file_size;
    }

    QFile file;
    file.setFileName(file_name);
    file.open(QIODevice::WriteOnly);

    QDataStream write(&file);

    while (bytes_done < file_size){
        bytes=0;
        while (bytes==0)
            bytes=socket->waitForReadyRead(-1);

        if (bytes=-1){
            qDebug()<<"Download error";
            socket->close();
            return;
        }

        QByteArray tmp = socket->readAll();
        bytes += write.writeRawData(tmp.data(), tmp.size());
        bytes_done += tmp.size();

        float precents=(static_cast<float>(bytes) / file_size) * 100;
        ui->label->setText(QString::number(precents));
    }
    file.close();
}

void MainWindow::on_server_button_clicked()
{
    ui->label->setText("U r the server");
    server=new QTcpServer(this);
    server->listen(QHostAddress::Any, 2121);
    connect(server, SIGNAL(newConnection()), SLOT(my_slot()));
}

void MainWindow::on_client_button_clicked()
{
    socket=new QTcpSocket(this);
    socket->connectToHost("localhost", 2121);
    ui->label->setText("U r the client");
    connect(socket, SIGNAL(readyRead()), SLOT(slot_reader()));
}
