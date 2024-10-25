#include <QApplication>
#include "ui/xviewer.hpp"
#include "xcamera_config.hpp"


int main(int argc,char *argv[]) {

    auto c{XCamera_Config_()};
    {
        XCameraDate date;
        date.m_name_ = GET_STR(cam0);
        date.m_url = "rtsp:://112/ch0";
        date.m_sub_url = "rtsp://112/ch1";
        date.m_save_path_ = "C:\\";
        c->Push(date);
    }

    {
        XCameraDate date;
        date.m_name_ = GET_STR(cam1);
        date.m_url = "rtsp:://512/ch0";
        date.m_sub_url = "rtsp://512/ch1";
        date.m_save_path_ = "C:\\";
        c->Push(date);
    }

    const auto cam_count{c->GetCamCount()};
    for(uint32_t i{};i < cam_count;i++) {
        qDebug()<< c->GetCam(i).m_name_;
    }

    QApplication a(argc, argv);

    if (auto xviewer{XViewer::create()}){
        xviewer->show();
        return QApplication::exec();
    }

    return -1;
}
