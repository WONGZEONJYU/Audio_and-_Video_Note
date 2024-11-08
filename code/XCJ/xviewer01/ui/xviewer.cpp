// You may need to build the project (run Qt uic code generator) to get "ui_XViewer.h" resolved

#include <QMouseEvent>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QFormLayout>
#include <QMessageBox>
#include <QDir>
#include <QStringList>
#include "xviewer.hpp"
#include "ui_xviewer.h"
#include <xhelper.hpp>
#include "xcamera_config.hpp"
#include "xcamera_record.hpp"
#include "xcamera_widget.hpp"
#include "xplayvieo.hpp"

#if _WIN32
#define C(s) QString::fromUtf8(s)
#else
#define C(s) QString::fromLocal8Bit(s)
#endif

static inline constexpr auto CAM_CONF_PATH{"cams.db"};

#if 1
XViewer::XViewer(QWidget *parent) :
QWidget(parent){}
#else
XViewer::XViewer(QWidget *parent) :
QWidget(parent),m_cam_wins_(16){}
#endif

void XViewer::RefreshCams() const {
    const auto c{XCamCfg()};
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

    //TRY_CATCH(CHECK_EXC(m_alone_play_.reset(new XPlayVideo())),return {});

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
        hlay->addWidget(m_ui_->playback_wid);
    }

    {
        const auto m{m_left_menu_.addMenu(C(GET_STR(view)))};
        auto a{m->addAction(C(GET_STR(1)))};
        CHECK_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::View1),return {});
        a = m->addAction(C(GET_STR(4)));
        CHECK_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::View4),return {});
        a = m->addAction(C(GET_STR(9)));
        CHECK_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::View9),return {});
        a = m->addAction(C(GET_STR(16)));
        CHECK_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::View16),return {});
        a = m->addAction(C(GET_STR(Start ALL Record)));
        CHECK_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::StartRecord),return {});
        a = m->addAction(C(GET_STR(Stop ALL Record)));
        CHECK_FALSE_(QObject::connect(a, &QAction::triggered, this, &XViewer::StopRecord),return {});
    }

    //默认显示9个窗口
    View9();
    CHECK_FALSE_(XCamCfg()->Load(CAM_CONF_PATH));
    RefreshCams();
    startTimer(1);
    Preview();
    m_ui_->time_list->clear();
    return true;
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
    CHECK_FALSE_(QObject::connect(&save_,&QPushButton::clicked,&dlg,&QDialog::accept));
    lay.addRow(&save_);

    const auto c{XCamCfg()};

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
    auto &[name,url,sub_url,save_path]{data};
    strcpy(name,name_edit.text().toStdString().c_str());
    strcpy(url,url_edit.text().toStdString().c_str());
    strcpy(sub_url,sub_url_edit.text().toStdString().c_str());
    strcpy(save_path,save_path_edit.text().toStdString().c_str());

    index >= 0 ? (void)c->SetCam(index,data) : c->Push(data);

    CHECK_FALSE_(c->Save(CAM_CONF_PATH));
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

    const auto c{XCamCfg()};
    c->DelCam(row);
    (void)c->Save(CAM_CONF_PATH);
    RefreshCams();
}

void XViewer::StartRecord() {
    StopRecord();
    m_ui_->status->setText(C("录制中。。。"));
    const auto c{XCamCfg()};
    const auto count{c->GetCamCount()};

    for (int i{}; i < count; ++i) {
        const auto &[name,url,
            sub_url,save_path]{c->GetCam(i)};

        QStringList ss;
        ss << save_path << GET_STR(/) << QString::number(i) << GET_STR(/);
        (void)QDir().mkpath(ss.join(""));

        QSharedPointer<XCameraRecord> rec;
        TRY_CATCH(CHECK_EXC(rec = m_cam_records_.emplace_back(new XCameraRecord())));
        if (rec.isNull()){
            continue;
        }
        rec->set_file_sec(10);
        rec->set_save_path(ss.join("").toStdString());
        rec->set_rtsp_url(url);
        rec->Start();
    }
}

void XViewer::StopRecord() {
    m_ui_->status->setText(C("监控中。。。"));
    m_cam_records_.clear();
}

void XViewer::contextMenuEvent(QContextMenuEvent *event) {
    m_left_menu_.exec(QCursor::pos());
    event->accept();
}

void XViewer::Preview() const{
    m_ui_->cams->show();
    m_ui_->playback_wid->hide();
    m_ui_->preview->setChecked(true);
}

void XViewer::Playback() const{
    m_ui_->cams->hide();
    m_ui_->playback_wid->show();
    m_ui_->playback->setChecked(true);
}

void XViewer::SelectCamera(const QModelIndex &index) {

    const auto &c{XCamCfg_Ref()};
    const auto cam{c[index.row()]};
    const auto &[name,url,
            sub_url,save_path]{cam};

    if (!name[0]) {
        return;
    }

    QStringList ss;
    ss << save_path << GET_STR(/) << QString::number(index.row()) << GET_STR(/);

    qDebug() << ss.join("");

    QDir dir(ss.join(""));
    if (!dir.exists()) {
        qDebug() << ss.join("") << " " << GET_STR(is empty!);
        return;
    }

    //获取当前目录下所有mp4 avi文件
    QStringList filters;
    filters << GET_STR(*.mp4) << GET_STR(*.avi);
    dir.setNameFilters(filters); //筛选
    m_ui_->cal->ClearDate();
    m_cam_videos_.clear();

    for (const auto files{dir.entryInfoList()};
        const auto &file: files) {

        const auto file_name{file.fileName()};

        if (file_name == GET_STR(.) || file_name == GET_STR(..)){
            continue;
        }
        //"cam_2024_11_06_23_41_46.mp4"

        auto t_date{file_name.left(file_name.size() - 4)}; //去掉.mp4
        t_date = t_date.right(t_date.size() - 4); //去掉cam_

        const auto dt{QDateTime::fromString(t_date,GET_STR(yyyy_MM_dd_hh_mm_ss))};
        m_ui_->cal->AddDate(dt.date());

        XCamVideo v;
        v.m_file_path = file.absoluteFilePath();
        v.m_date_time = dt;

        m_cam_videos_[dt.date()].push_back(v);
    }
    m_ui_->cal->showNextMonth();
    m_ui_->cal->showPreviousMonth();

}

void XViewer::SelectDate(const QDate date) {

    //qDebug() << date;
    m_ui_->time_list->clear();
    for (const auto dates{m_cam_videos_[date]};
        const auto &[m_file_path, m_date_time]: dates) {
        const auto item{new QListWidgetItem(m_date_time.time().toString())};
        item->setData(Qt::UserRole, m_file_path);
        m_ui_->time_list->addItem(item);
    }
}

void XViewer::PlayVideo(const QModelIndex &index) {
    (void)index;
    //qDebug() << index;
#if 1
    const auto file_path{index.data(Qt::UserRole).toString()};
    qDebug() << file_path;
#else
    const auto item {m_ui_->time_list->currentItem()};
    if (!item) {
        return;
    }
    const auto file_path{item->data(Qt::UserRole).toString()};
    qDebug() << file_path;
#endif
    //const auto play{dynamic_cast<XPlayVideo*>(m_alone_play_.get())};
    static XPlayVideo play;
    play.show();
    play.Open(file_path);
}

void XViewer::View(const int &count) {
    //qDebug() << __func__ << GET_STR(()) << count;
    //2x2 3x3 4x4
    const auto cols{static_cast<int>(qSqrt(count))}; //开根号得到列数

    auto lay {dynamic_cast<QGridLayout*>(m_ui_->cams->layout())};
    if (!lay){
        TRY_CATCH(CHECK_EXC(lay = new QGridLayout(m_ui_->cams)), return);
        lay->setContentsMargins({});
        lay->setSpacing(2);
    }
#if 1
    m_cam_wins_.clear();
    for (int i {}; i < count; ++i) {
        XCameraWidget_sp w;
        TRY_CATCH(CHECK_EXC(w = m_cam_wins_.emplace_back(new XCameraWidget())), return;);
        w->setStyleSheet(GET_STR(background-color:rgb(51, 51, 51);));
        lay->addWidget(w.get(), i / cols , i % cols);
    }

#else
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

#endif
}

void XViewer::timerEvent(QTimerEvent *e){

    foreach(const auto &item,m_cam_wins_){
        if (item){
            item->Draw();
        }
    }
    QWidget::timerEvent(e);
}

XViewer_sp XViewer::create() {
    XViewer_sp obj;
    TRY_CATCH(CHECK_EXC(obj.reset(new XViewer())),return {});
    if (!obj->Construct()){
        obj.reset();
    }
    return obj;
}
