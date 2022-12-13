#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow::~MainWindow";
    delete ui;
}


void MainWindow::on_recordButton_clicked()
{
    if (!_thread) {
        _thread = new RecordThread(this);
        _thread->start();
        ui->recordButton->setText("暂停录音");
        connect(_thread, &QThread::finished, [this]() {
            ui->recordButton->setText("开始录音");
            _thread = nullptr;
        });
    } else {
        _thread->requestInterruption();
        _thread = nullptr;
//        ui->recordButton->setText("开始录音");
    }
}

