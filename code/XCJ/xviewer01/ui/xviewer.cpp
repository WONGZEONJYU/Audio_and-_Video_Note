// You may need to build the project (run Qt uic code generator) to get "ui_XViewer.h" resolved

#include "xviewer.hpp"
#include "ui_xviewer.h"
#include <QMouseEvent>
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
    this->move((event->globalPosition() - m_mouse_pos_).toPoint());
}

void XViewer::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);

    if (Qt::LeftButton == event->button()) {
        m_is_mouse_pressed_ = true;
        m_mouse_pos_ = event->position();
    }
}

void XViewer::mouseReleaseEvent(QMouseEvent *event) {
    m_is_mouse_pressed_ = false;
    QWidget::mouseReleaseEvent(event);
}
/////////////////////////////鼠标拖动窗口事件///////////////////////////////////////