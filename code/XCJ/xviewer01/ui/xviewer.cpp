// You may need to build the project (run Qt uic code generator) to get "ui_XViewer.h" resolved

#include <QMouseEvent>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFormLayout>
#include <QMessageBox>
#include "xviewer.hpp"
#include "ui_xviewer.h"
#include <xhelper.hpp>
#include "xcamera_config.hpp"
#include "xcamera_widget.hpp"

#if _WIN32
#define C(s) QString::fromUtf8(s)
#else
#define C(s) QString::fromLocal8Bit(s)
#endif

static constexpr auto CAM_CONF_PATH{"cams.db"};

XViewer_sp XViewer::create() {
    XViewer_sp obj;
    TRY_CATCH(CHECK_EXC(obj.reset(new XViewer())),return {});
    if (!obj->Construct()){
        obj.reset();
    }
    return obj;
}

XViewer::XViewer(QWidget *parent) :
QWidget(parent),m_cam_wins_(16){}

XViewer::~XViewer() {
    Destroy();
}

void XViewer::RefreshCams() const {
    const auto c{XCamera_Config_()};
    m_ui_->cam_list->clear();
    const auto count{c->GetCamCount()};
    for (int i {}; i < count; ++i) {
        const auto &[m_name_, m_url,
            m_sub_url, m_save_path_]{c->GetCam(i)};
        const auto item{new QListWidgetItem(QIcon(GET_STR(:/img/cam.png)),
                                            C(m_name_))};
        m_ui_->cam_list->addItem(item);
    }
}

bool XViewer::Construct() {
    TRY_CATCH(CHECK_EXC(m_ui_.reset(new Ui::XViewer())),return {});
    m_ui_->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    {
        QVBoxLayout *vlay{};
        TRY_CATCH(CHECK_EXC(vlay = new QVBoxLayout(this)),return {});
        //设置基类地址 == setLayout(vlay);
        //边框距
        vlay->setContentsMargins({});
        //元素距
        vlay->setSpacing(0);
        vlay->addWidget(m_ui_->head);
        vlay->addWidget(m_ui_->body);
    }

    {
        QHBoxLayout *hlay{};
        TRY_CATCH(CHECK_EXC(hlay = new QHBoxLayout(m_ui_->body)),return {});
        hlay->setContentsMargins({});
        hlay->setSpacing(0);
        hlay->addWidget(m_ui_->left);
        hlay->addWidget(m_ui_->cams);
    }

    {
        const auto m{m_left_menu_.addMenu(C(GET_STR(view)))};
        auto a{m->addAction(C(GET_STR(1)))};
        IS_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::View1),return {});
        a = m->addAction(C(GET_STR(4)));
        IS_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::View4),return {});
        a = m->addAction(C(GET_STR(9)));
        IS_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::View9),return {});
        a = m->addAction(C(GET_STR(16)));
        IS_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::View16),return {});
    }

    //默认显示9个窗口
    View9();
    (void )XCamera_Config_()->Load(CAM_CONF_PATH);
    RefreshCams();
    startTimer(1);
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

void XViewer::MaxWindow() {
    m_ui_->max->setVisible(false);
    m_ui_->normal->setVisible(true);
    showMaximized();
}

void XViewer::NormalWindow() {
    m_ui_->normal->setVisible(false);
    m_ui_->max->setVisible(true);
    showNormal();
}

void XViewer::resizeEvent(QResizeEvent *event) {

    const auto x{width() - m_ui_->head_button->width()},
                y{m_ui_->head_button->y()};
    m_ui_->head_button->move(x,y);
    QWidget::resizeEvent(event);
}

void XViewer::View1(){
    View(1);
}

void XViewer::View4(){
    View(4);
}

void XViewer::View9(){
    View(9);
}
void XViewer::View16(){
    View(16);
}

void XViewer::SetCam(const int &index) {

    QDialog dlg(this);
    dlg.resize(800,200);

    QFormLayout lay(&dlg);

    QLineEdit name_edit;
    lay.addRow(C(GET_STR(名称)),&name_edit);

    QLineEdit url_edit;
    lay.addRow(C(GET_STR(主码流)),&url_edit);

    QLineEdit sub_url_edit;
    lay.addRow(C(GET_STR(子码流)),&sub_url_edit);

    QLineEdit save_path_edit;
    lay.addRow(C(GET_STR(保存路径)),&save_path_edit);

    QPushButton save_(C(GET_STR(保存)));
    IS_FALSE_(QObject::connect(&save_,&QPushButton::clicked,&dlg,&QDialog::accept));
    lay.addRow(&save_);

    const auto c{XCamera_Config_()};

    if (index >= 0) {
        const auto &[name,url,sub_url_
            ,save_path]{c->GetCam(index)};
        name_edit.setText(C(name));
        url_edit.setText(C(url));
        sub_url_edit.setText(C(sub_url_));
        save_path_edit.setText(C(save_path));
    }

    while (true) {
        if (QDialog::Accepted == dlg.exec()) {

            if (name_edit.text().isEmpty() ) {
                QMessageBox::information({},GET_STR(error),C(GET_STR("请输入名称")));
                continue;
            }

            if (url_edit.text().isEmpty() ) {
                QMessageBox::information({},GET_STR(error),C(GET_STR("请输入主码流")));
                continue;
            }

            if (sub_url_edit.text().isEmpty() ) {
                QMessageBox::information({},GET_STR(error),C(GET_STR("请输入辅码流")));
                continue;
            }

            if (save_path_edit.text().isEmpty() ) {
                QMessageBox::information({},GET_STR(error),C(GET_STR("请输入保存路径")));
                continue;
            }
            break;
        }
        return;
    }

    XCameraData data;
    auto&[name,url,sub_url,save_path]{data};
    strcpy(name,name_edit.text().toStdString().c_str());
    strcpy(url,url_edit.text().toStdString().c_str());
    strcpy(sub_url,sub_url_edit.text().toStdString().c_str());
    strcpy(save_path,save_path_edit.text().toStdString().c_str());

    index >= 0 ? (void)c->SetCam(index,data) : c->Push(data);

    IS_FALSE_(c->Save(CAM_CONF_PATH));
    RefreshCams();
}

void XViewer::AddCam() {
    SetCam(-1);
}

void XViewer::SetCam() {
    const auto row{m_ui_->cam_list->currentIndex().row()};
    if (row < 0) {
        QMessageBox::information(this,"error",C(GET_STR(请选择摄像机!)));
        return;
    }
    SetCam(row);
}

void XViewer::DelCam() {
    const auto row{m_ui_->cam_list->currentIndex().row()};
    if (row < 0) {
        QMessageBox::information(this,"error",C(GET_STR(请选择要删除的摄像机!)));
        return;
    }

    QStringList ss;
    ss << GET_STR(你确认需要删除摄像机: ) <<
        m_ui_->cam_list->currentItem()->text() << " " << GET_STR(吗?);

    if (QMessageBox::question(this,"confirm",ss.join("")) != QMessageBox::Yes) {
        return;
    }

    const auto c{XCamera_Config_()};
    c->DelCam(row);
    (void)c->Save(CAM_CONF_PATH);
    RefreshCams();
}

void XViewer::contextMenuEvent(QContextMenuEvent *event) {
    m_left_menu_.exec(QCursor::pos());
    event->accept();
}

void XViewer::View(const int &count) {
    //qDebug() << __func__ << GET_STR(()) << count;
    //2x2 3x3 4x4
    const auto cols{static_cast<int>(qSqrt(count))};

    auto lay{dynamic_cast<QGridLayout*>(m_ui_->cams->layout())};
    if (!lay){
        TRY_CATCH(CHECK_EXC(lay = new QGridLayout(m_ui_->cams)), return;);
        lay->setContentsMargins({});
        lay->setSpacing(2);
    }

    for(int i{};i < count;++i){
        if (!m_cam_wins_[i]) {
            TRY_CATCH(CHECK_EXC(m_cam_wins_[i].reset(new XCameraWidget())), return;);
            m_cam_wins_[i]->setStyleSheet(GET_STR(background-color:rgb(51, 51, 51);));
        }
        lay->addWidget(m_cam_wins_[i].get(), i / cols , i % cols);
    }

    for (int i {count}; i < m_cam_wins_.capacity(); ++i) {
        if (m_cam_wins_[i]){
            m_cam_wins_[i].reset();
        }
    }
}

void XViewer::timerEvent(QTimerEvent *e){

    foreach(const auto &item,m_cam_wins_){
        if (item){
            item->Draw();
        }
    }

    QWidget::timerEvent(e);
}
