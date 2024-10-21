// You may need to build the project (run Qt uic code generator) to get "ui_XViewer.h" resolved

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
    return true;
}

void XViewer::Destroy() {

}
