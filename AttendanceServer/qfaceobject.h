#ifndef QFACEOBJECT_H
#define QFACEOBJECT_H

#include <QObject>
#include <seeta/FaceEngine.h>
#include <opencv.hpp>
class QFaceObject : public QObject
{
    Q_OBJECT
public:
    explicit QFaceObject(QObject *parent = nullptr);
    ~QFaceObject();

public slots:
    int64_t face_register(cv::Mat& faceImage);
    int face_query(cv::Mat& faceImage);
signals:
    void send_faceid(int64_t faceid);
private:
    seeta::FaceEngine *fenginptr;
    
};

#endif // QFACEOBJECT_H
