#include "attendancewin.h"

#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <opencv.hpp>
#include "registerwin.h"
#include "selectwin.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<cv::Mat>("cv::Mat&");
    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<int64_t>("int64_t");
//    RegisterWin ww;
//    ww.show();
    //连接数据库
    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
    //设置数据名称
    db.setDatabaseName("server.db");
    //打开数据库
    if(!db.open())
    {
        qDebug()<<db.lastError().text();
        return -1;
    }
    //创建员工信息表格
    QString createsql="create table if not exists employee(employeeID integer primary key autoincrement,name varchar(256),sex varchar(32),"
                      "birthday text,address text,phone text,faceID integer unique,headfile text)";
    QSqlQuery query;
    if(!query.exec(createsql))
    {
        qDebug()<<query.lastError().text();
        return -1;
    }
    //考勤表格
    createsql="create table if not exists attendance(attendanceID integer primary key autoincrement"
              ",employeeID integer,attendanceTime TimeStamp NOT NULL DEFAULT(datetime('now','localtime')))";
    if(!query.exec(createsql))
    {
        qDebug()<<query.lastError().text();
        return -1;
    }
    AttendanceWin w;
    w.show();

//    SelectWin sw;
//    sw.show();
    return a.exec();
}
