#include "faceattendence.h"
#include "ui_faceattendence.h"
#include <QImage>
#include <QPainter>
#include <QDebug>

#include<QJsonDocument>
#include<QJsonParseError>
#include<QJsonObject>
FaceAttendence::FaceAttendence(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::FaceAttendence)
{
    ui->setupUi(this);
    //打开摄像头
    cap.open("0");
    //启动定时器事件
    startTimer(100);
    //导入级联分类器
    cascade.load("C:/opencv452/etc/haarcascades/haarcascade_frontalface_alt2.xml");

    //QTcpSocket当断开连接的时候disconnected信号,连接成功会发送connected
    connect(&msocket,&QTcpSocket::disconnected,this,&FaceAttendence::start_connect);
    connect(&msocket,&QTcpSocket::connected,this,&FaceAttendence::stop_connect);
    //关联接收数据的槽函数
    connect(&msocket,&QTcpSocket::readyRead,this,&FaceAttendence::recv_data);
    //定时器连接服务器
    connect(&mtimer,&QTimer::timeout,this,&FaceAttendence::timer_connect);
    //启动定时器
    mtimer.start(5000);//每5s种连接一次
    flag=0;

    ui->widgetLb->hide();
}

FaceAttendence::~FaceAttendence()
{
    delete ui;
}

void FaceAttendence::timerEvent(QTimerEvent *e)
{
    //采集数据
    Mat srcImage;
    if(cap.grab())
    {
        cap.read(srcImage);//读取一帧数据

    }
    Mat grayImage;
    //转灰度图
    cvtColor(srcImage,grayImage,COLOR_BGR2GRAY);
    //检测人脸数据
    std::vector<Rect> faceRects;
    cascade.detectMultiScale(srcImage,faceRects,1.1);
    if(faceRects.size()>0&&flag>=0)
    {
        Rect rect=faceRects.at(0);//第一个人脸的矩形框
        //rectangle(srcImage,rect,Scalar(0,0,255));//检测到人脸
        //移动人脸框(图片--QLabel)
        ui->headpicLb->move(rect.x,rect.y);

        if(flag>2){
            //把Mat数据转化为QbyteArray,编码成jpg格式
            std::vector<uchar> buf;
            cv::imencode(".jpg",srcImage,buf);
            QByteArray byte((const char*)buf.data(),buf.size());
            //准备发送
            quint64 backsize=byte.size();
            QByteArray sendData;
            QDataStream stream(&sendData,QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_5_7);
            stream<<backsize<<byte;
            //发送
            msocket.write(sendData);
            flag=-2;
            faceMat =srcImage(rect);
            qDebug()<<"发送";
            imwrite("./face.jpg",faceMat);
        }
        flag++;

    }
    if(faceRects.size()==0)
    {
        //把人脸框移动到中心位置
        ui->headpicLb->move(100,60);
        ui->widgetLb->hide();
        flag=0;

    }
    if(srcImage.data==nullptr) return;
    //把opencv里面的Mat格式数据(BGR)转Qt里面的QImage(RGB)
    cvtColor(srcImage,srcImage,COLOR_BGR2RGB);
    QImage image(srcImage.data,srcImage.cols,srcImage.rows,srcImage.step1(),QImage::Format_RGB888);
    QPixmap mmp=QPixmap::fromImage(image);
    ui->videoLb->setPixmap(mmp);


}
void FaceAttendence::recv_data()
{
    QByteArray array=msocket.readAll();
    qDebug()<<array;
    //json解析
    QJsonParseError err;
    QJsonDocument doc=QJsonDocument::fromJson(array,&err);
    if(err.error!=QJsonParseError::NoError)
    {
        qDebug()<<"json数据错误";
        return;
    }
    QJsonObject obj=doc.object();
    QString employeeID = obj.value("employeeID").toString(); // 员工ID
    QString name = obj.value("name").toString();             // 姓名
    QString timestr = obj.value("time").toString();          // 时间字符串
    QString department = obj.value("department").toString();       // 地址


    // 更新界面显示
    ui->numberEdit->setText(employeeID);  // 员工ID标签
    ui->nameEdit->setText(name);          // 姓名标签
    ui->departmentEdit->setText(department);        // 地址标签
    ui->timeEdit->setText(timestr);           // 时间标签

    ui->widgetLb->show();
    ui->headLb->setStyleSheet("border-radius:75px;border-image:url(./face.jpg);");
}

void FaceAttendence::timer_connect()
{
    //连接服务器
    msocket.connectToHost("localhost",9999);
    qDebug()<<"正在连接服务器";
}

void FaceAttendence::stop_connect()
{
    mtimer.stop();
    //
    qDebug()<<"成功连接服务器";
}

void FaceAttendence::start_connect()
{
    mtimer.start(5000);//启动定时器
    qDebug()<<"断开连接";
}
