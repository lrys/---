#include "attendancewin.h"
#include "ui_attendancewin.h"
#include <opencv.hpp>
#include <QDateTime>
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
AttendanceWin::AttendanceWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AttendanceWin)
{
    ui->setupUi(this);
    //qtcpServer当有客户端连会发送newconnection
    connect(&mserver, &QTcpServer::newConnection, this, &AttendanceWin::accept_client);
    mserver.listen(QHostAddress::Any,9999);//监听，启动服务器
    bsize=0;

    //给模型绑定表格
    model.setTable("employee");

    //创建一个线程
    QThread *thread=new QThread();
    //把QFaceObject
    fobj.moveToThread(thread);
    //启动线程
    thread->start();
    connect(this,&AttendanceWin::query,&fobj,&QFaceObject::face_query);
    //关联QFaceObject对象里面的send_faceid信号
    connect(&fobj,&QFaceObject::send_faceid,this,&AttendanceWin::recv_faceid);
}

AttendanceWin::~AttendanceWin()
{
    delete ui;
}

//接受客户端连接
void AttendanceWin::accept_client()
{
    //获取与客户端通信的套接字
    msocket = mserver.nextPendingConnection();

    //当客户端有数据到达会发送readyRead信号
    connect(msocket,&QTcpSocket::readyRead,this,&AttendanceWin::read_data);
}

//读取客户端发送的数据
void AttendanceWin::read_data()
{
    //读取所有的数据
    QDataStream stream(msocket);//把套接字绑定到数据流
    stream.setVersion(QDataStream::Qt_5_14);
    if(bsize==0){
        if(msocket->bytesAvailable()<(qint64)sizeof(bsize)) return;
        //采集数据的长度
        stream>>bsize;
    }

    if(msocket->bytesAvailable()<bsize)//说明数据还没有方式完成
    {
        return;
    }
    QByteArray data;
    stream>>data;
    bsize=0;
    if(data.size()==0)//没有读取到数据
    {
        return;
    }
    //显示图片
    QPixmap mmp;
    mmp.loadFromData(data,"jpg");
    mmp=mmp.scaled(ui->picLb->size());
    ui->picLb->setPixmap(mmp);
    //识别人脸
    cv::Mat faceImage;
    std::vector<uchar> decode;
    decode.resize(data.size());
    memcpy(decode.data(),data.data(),data.size());
    faceImage=cv::imdecode(decode,cv::ImreadModes::IMREAD_COLOR);
    //int faceid=fobj.face_query(faceImage);//消耗资源较多
    emit query(faceImage);
    //

}

void AttendanceWin::recv_faceid(int64_t faceid)
{
    //查询faceid对应的个人信息
    //设置过滤器

    if(faceid<0)
    {
         QString sdmsg=QString("{\"employeeID\":\"\",\"name\":\"\",\"department\":\"\",\"time\":\"\"}");
         msocket->write(sdmsg.toUtf8());//把打包好的数据发送给客户端
         return;
    }
    model.setFilter(QString("faceID=%1").arg(faceid));
    //查询
    model.select();
    //判断是否查询到数据
    if(model.rowCount()==1)
    {
        //工号,姓名,部门,时间
        //{employeeID:%1,name:%2,department:软件,time:%3}
        QSqlRecord record=model.record(0);
        QString sdmsg=QString("{\"employeeID\":\"%1\",\"name\":\"%2\",\"department\":\"软件\",\"time\":\"%3\"}")
                .arg(record.value("employeeID").toString()).arg(record.value("name").toString())
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));


        //把数据写入数据库--考勤表

        QString insertSql=QString("insert into attendance(employeeID) values(%1)").arg(record.value("employeeID").toString());
        QSqlQuery query;
        if(!query.exec(insertSql))
        {
            QString sdmsg=QString("{\"employeeID\":\"\",\"name\":\"\",\"department\":\"\",\"time\":\"\"}");
            msocket->write(sdmsg.toUtf8());//把打包好的数据发送给客户端
            qDebug()<<query.lastError().text();
             qDebug()<<faceid;

            return;
        }
        else
        {
             msocket->write(sdmsg.toUtf8());//把打包好的数据发送给客户端
                      qDebug()<<faceid;
        }
    }
}

























