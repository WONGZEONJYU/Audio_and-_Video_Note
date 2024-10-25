#include <QApplication>
#include "ui/xviewer.hpp"
#include "xcamera_config.hpp"

#define TEST_CAM "test.db"

int main(int argc,char *argv[]) {

    auto c{XCamera_Config_()};
    c->Load(TEST_CAM);
    {
        XCameraData data;
        strcpy(data.m_name_,GET_STR(cam0));
        strcpy(data.m_url,"rtsp:://112/ch0");
        strcpy(data.m_sub_url,"rtsp://112/ch1");
        strcpy(data.m_save_path_,"C:\\");
        c->Push(data);
    }

    {
        XCameraData data;
        strcpy(data.m_name_,GET_STR(cam1));
        strcpy(data.m_url,"rtsp:://512/ch0");
        strcpy(data.m_sub_url,"rtsp://512/ch1");
        strcpy(data.m_save_path_,"C:\\");
        c->Push(data);
    }

    auto cam_count{c->GetCamCount()};
    for(int i{};i < cam_count;i++) {
        qDebug()<< c->GetCam(i).m_name_;
    }

    qDebug() << GET_STR(=====================update==================);
    auto c1{c->GetCam(0)};
    strcpy(c1.m_name_,GET_STR(cam02));
    c->SetCam(0,c1);
    cam_count = c->GetCamCount();
    for(int i{};i < cam_count;i++) {
        qDebug()<< c->GetCam(i).m_name_;
    }
    qDebug() << GET_STR(==================del=====================);

    c->DelCam(1);

    cam_count = c->GetCamCount();
    for(int i{};i < cam_count;i++) {
        qDebug()<< c->GetCam(i).m_name_;
    }
    c->Save(TEST_CAM);
    QApplication a(argc, argv);

    if (auto xviewer{XViewer::create()}){
        xviewer->show();
        return QApplication::exec();
    }

    return -1;
}
