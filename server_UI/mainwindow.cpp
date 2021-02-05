#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "server.cpp"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    connectMYSQL();
    server();
    ui->setupUi(this);
    //connect(this,SIGNAL(), this,SLOT(on_pushButton_clicked()));
    connect(ui->pl_radio_button, SIGNAL(clicked()), this, SLOT(radioFunction_1()));
    connect(ui->car_radio_button, SIGNAL(clicked()), this, SLOT(radioFunction_1()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_shutdown_clicked()
{
    CloseServer();
}

void MainWindow::radioFunction_1()
{
    if (ui->pl_radio_button->isChecked())
        select = "parkinglot";
        //QMessageBox::information(this, "title", ui.radioButton_1->text());
    if (ui->car_radio_button->isChecked())
        select = "car";
        //QMessageBox::information(this, "title", ui.radioButton_2->text());

}


void MainWindow::on_pushButton_clicked()
{
    std::string query = "";
    keyword = ui->search->text();
    if(select == "car"){
        if(!Isout(keyword.toStdString(), "car"))
            query = "SELECT number, indate, intime, location_c, location_p FROM car WHERE number = '"+keyword.toStdString()+"'";

        else if(Isout(keyword.toStdString(), "car"))
            query = "SELECT number, indate, intime, outdate, intime, outtime, toll FROM car WHERE number = '"+keyword.toStdString()+"'";
    }
    else if(select == "parkinglot"){
        query = "SELECT * FROM "+select +" WHERE number = "+keyword.toStdString();
    }

    std::string str = SearchData(query);
    ui->resulttext->setText(QString(str.c_str()));
    ui->resultbar->setText("Success");
}

void MainWindow::on_All_Car_clicked()
{
    ui->resultbar->setText("Success");
    std::string str = Store_c();
    ui->resulttext->setText(QString(str.c_str()));

}

void MainWindow::on_pushButton_4_clicked()
{
    std::string str = Store_p();
    ui->resulttext->setText(QString(str.c_str()));
    ui->resultbar->setText("Success");
}

void MainWindow::on_readfile_button_clicked()
{
    std::string filedir = "/tmp/";
    std::string filename = "input.csv";
    std::string res = ReadData(filedir+filename);
    ui->resultbar->setText(QString(res.c_str()));
    ui->resultbar->setText("Success");

}

void MainWindow::on_writefile_button_clicked()
{

    std::string filedir = "/tmp/";
    std::string filename = "output.csv";
    std::string res = WriteData(filedir+filename);
    ui->resultbar->setText(QString(res.c_str()));
    ui->resultbar->setText("Success");
}



