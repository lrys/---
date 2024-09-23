#include "registerwin.h"
#include "ui_registerwin.h"
#include "qfaceobject.h"
#include <QFileDialog>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QMessageBox>
#include <QDebug>
RegisterWin::RegisterWin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegisterWin)
{
    ui->setupUi(this);
}

RegisterWin::~RegisterWin()
{
    delete ui;
}

void RegisterWin::timerEvent(QTimerEvent *e)
{
    //获取摄像头数据并且显示在界面上

    if(cap.isOpened())
    {
        cap>>image;
        if(image.data==nullptr) return;
    }

    //显示Mat-->QImage
    cv::Mat rgbImage;
    cv::cvtColor(image,rgbImage,cv::COLOR_BGR2RGB);
    QImage qImg(rgbImage.data,rgbImage.cols,rgbImage.rows,rgbImage.step1(),QImage::Format_RGB888);
    //在qt界面显示
    QPixmap mmp=QPixmap::fromImage(qImg);
    mmp=mmp.scaledToWidth(ui->headpicLb->width());
    ui->headpicLb->setPixmap(mmp);

}

void RegisterWin::on_resetBt_clicked()
{
    //清空数据
    ui->nameEdit->clear();
    ui->birthdayEdit->setDate(QDate::currentDate());
    ui->addressEdit->clear();
    ui->phoneEdit->clear();
    ui->picFileEdit->clear();
}

void RegisterWin::on_addpicBt_clicked()
{
    //通过文件对话框 选中图片路径
    QString filepath=QFileDialog::getOpenFileName(this);
    ui->picFileEdit->setText(filepath);
    //显示图片
    QPixmap mmp(filepath);
    mmp=mmp.scaledToWidth(ui->headpicLb->width());
    ui->headpicLb->setPixmap(mmp);
}

void RegisterWin::on_registerBt_clicked()
{
    //1.通过照片,结合faceObject模块得到faceID
    QFaceObject faceobj;
    cv::Mat image=cv::imread(ui->picFileEdit->text().toUtf8().data());
    int faceID=faceobj.face_register(image);
    qDebug()<<faceID;
    //把头像保存到一个固定路径下
    QString headfile=QString("./data/%1.jpg").arg(ui->nameEdit->text().toUtf8().data());
    cv::imwrite(headfile.toUtf8().data(),image);
    //2.把个人信息存储到数据库employee
    QSqlTableModel model;
    model.setTable("employee");//设置表名
    QSqlRecord record=model.record();
    //设置数据
    record.setValue("name",ui->nameEdit->text());
    record.setValue("sex",ui->mrb->isChecked()?"女":"男");
    record.setValue("birthday",ui->birthdayEdit->text());
    record.setValue("address",ui->addressEdit->text());
    record.setValue("phone",ui->phoneEdit->text());
    record.setValue("faceID",faceID);
    //头像路径
    record.setValue("headfile",headfile);
    //把记录插入到数据库表格中
    bool ret=model.insertRecord(0,record);
    if(ret)
    {
        QMessageBox::information(this,"注册提示","注册成功");
        //提交
        model.submitAll();
    }
    else
    {
        QMessageBox::information(this,"注册提示","注册失败");
        //提交
    }


    //提示注册成功

}

void RegisterWin::on_videoswitchBt_clicked()
{
    if(ui->videoswitchBt->text()=="打开摄像头")
    {
        //打开摄像头
        if(cap.open(0))
        {
             ui->videoswitchBt->setText("关闭摄像头");
             //启动定时器时间,
             timerid=startTimer(100);
        }
    }
    else
    {
        killTimer(timerid);//关闭定时器时间
        ui->videoswitchBt->setText("打开摄像头");
        //关闭摄像头
        cap.release();
    }
}

void RegisterWin::on_cameraBt_clicked()
{
    //保存数据
    //把头像保存到一个固定路径下
    QString headfile=QString("./data/%1.jpg").arg(ui->nameEdit->text().toUtf8().data());
    ui->picFileEdit->setText(headfile);
    cv::imwrite(headfile.toUtf8().data(),image);
    killTimer(timerid);//关闭定时器事件
    ui->videoswitchBt->setText("打开摄像头");
    //关闭摄像头
    cap.release();
}
