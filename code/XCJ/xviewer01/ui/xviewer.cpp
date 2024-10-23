// You may need to build the project (run Qt uic code generator) to get "ui_XViewer.h" resolved
#include <filesystem>
#include <QMouseEvent>
#include "xviewer.hpp"
#include "ui_xviewer.h"
#include <xhelper.hpp>

XViewer_sp XViewer::create() {
    XViewer_sp obj;
    TRY_CATCH(CHECK_EXC(obj.reset(new XViewer())),return {});
    if (!obj->Construct()){
        obj.reset();
    }
    return obj;
}

XViewer::XViewer(QWidget *parent) :QWidget(parent){

}

XViewer::~XViewer() {
    Destroy();
}

bool XViewer::Construct() {
    TRY_CATCH(CHECK_EXC(m_ui_.reset(new Ui::XViewer)),return {});
    m_ui_->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    return true;
}

void XViewer::Destroy() {

}

/////////////////////////////鼠标拖动窗口事件///////////////////////////////////////
void XViewer::mouseMoveEvent(QMouseEvent *event) {

    if (!m_is_mouse_pressed_) {
        QWidget::mouseMoveEvent(event);
        return;
    }
#if 0
    const auto global_pos{event->globalPosition()}; //获取鼠标相对桌面的坐标
    const auto delta_{global_pos - m_mouse_pos_}; //计算出两坐标的差值
    this->move(pos() + delta_.toPoint()); //Widget窗口相对桌面的坐标 + 鼠标坐标差值 = 计算出Widget移动到的坐标
    m_mouse_pos_ = global_pos; //把当前鼠标坐标记录下来
#else
    this->move((event->globalPosition() - m_mouse_pos_).toPoint());
#endif
}

void XViewer::mousePressEvent(QMouseEvent *event) {

    if (Qt::LeftButton == event->button()) {
        m_is_mouse_pressed_ = true;
#if 0
        m_mouse_pos_ = event->globalPosition(); //获取鼠标相对桌面的坐标
#else
        m_mouse_pos_ = event->position(); //获取鼠标相对当前Widget的坐标
#endif
    }
    QWidget::mousePressEvent(event);
}

void XViewer::mouseReleaseEvent(QMouseEvent *event) {
    m_is_mouse_pressed_ = false;
    QWidget::mouseReleaseEvent(event);
}
/////////////////////////////鼠标拖动窗口事件///////////////////////////////////////
///