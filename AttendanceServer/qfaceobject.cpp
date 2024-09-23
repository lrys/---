#include "qfaceobject.h"

QFaceObject::QFaceObject(QObject *parent) : QObject(parent)
{
    //初始化
    seeta::ModelSetting FDmode("C:/SeetaFace/bin/model/fd_2_00.dat",seeta::ModelSetting::CPU,0);
    seeta::ModelSetting PDmode("C:/SeetaFace/bin/model/pd_2_00_pts5.dat",seeta::ModelSetting::CPU,0);
    seeta::ModelSetting FRmode("C:/SeetaFace/bin/model/fr_2_10.dat",seeta::ModelSetting::CPU,0);
    this->fenginptr=new seeta::FaceEngine(FDmode,PDmode,FRmode);

    //导入已有的人脸数据库
    this->fenginptr->Load("./face.db");
}

QFaceObject::~QFaceObject()
{
    delete fenginptr;
}

int64_t QFaceObject::face_register(cv::Mat &faceImage)
{
    //把opencv的Mat数据转为seetaface的数据
    SeetaImageData simage;
    simage.data=faceImage.data;
    simage.width=faceImage.cols;
    simage.height=faceImage.rows;
    simage.channels=faceImage.channels();
    int64_t faceid=this->fenginptr->Register(simage);//返回人脸id
    if(faceid>=0){
        fenginptr->Save("./face.db");
    }
    return faceid;
}

int QFaceObject::face_query(cv::Mat &faceImage)
{
    SeetaImageData simage;
    simage.data=faceImage.data;
    simage.width=faceImage.cols;
    simage.height=faceImage.rows;
    simage.channels=faceImage.channels();
    float similarity=0;
    int64_t faceid=fenginptr->Query(simage,&similarity);//运行时间比较长
    if(similarity>0.7)
    {
         emit send_faceid(faceid);
    }
    else
    {
        emit send_faceid(-1);
    }
    return faceid;
}
