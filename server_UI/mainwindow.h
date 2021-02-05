#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <QVector>
#include <QDebug>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_shutdown_clicked();

    void on_pushButton_clicked();

    void on_All_Car_clicked();

    void on_pushButton_4_clicked();

    void on_readfile_button_clicked();

    void on_writefile_button_clicked();

    void radioFunction_1();
private:
    Ui::MainWindow *ui;

    QString keyword;
    std::string select;

};
#endif // MAINWINDOW_H
