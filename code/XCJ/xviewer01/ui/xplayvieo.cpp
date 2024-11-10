// You may need to build the project (run Qt uic code generator) to get "ui_XPlayVieo.h" resolved

#include "xplayvieo.hpp"
#include <xvideo_view.hpp>
#include "ui_xplayvieo.h"

void XPlayVideo::timerEvent(QTimerEvent *const event) {

    if (const auto frame{m_decode_task_.CopyFrame()}) {
#ifdef MACOS
        Repaint(*frame);
#else
        IS_SMART_NULLPTR(m_view_,QWidget::timerEvent(event);return);
        m_view_->DrawFrame(*frame);
#endif
    }
    QWidget::timerEvent(event);
}

void XPlayVideo::closeEvent(QCloseEvent * const event) {
    Close();
    QWidget::closeEvent(event);
}

XPlayVideo::XPlayVideo(QWidget *parent) :
#ifdef MACOS
XVideoWidget(parent),
#else
QWidget(parent),
#endif
m_ui_(new Ui::XPlayVieo) {
    m_ui_->setupUi(this);
}

XPlayVideo::~XPlayVideo() {
    Close();
}

bool XPlayVideo::Open(const QString &url) {

    if (url.isEmpty()) {
        return {};
    }

    Close();

    CHECK_FALSE_(m_demux_task_.Open(url.toStdString()),return {});

    XCodecParameters_sp vp;
    IS_SMART_NULLPTR(vp = m_demux_task_.CopyVideoParm(),return {});

    CHECK_FALSE_(m_decode_task_.Open(vp),return {});

    m_demux_task_.set_next(std::addressof(m_decode_task_));
    m_demux_task_.set_sync_type(SYNC_VIDEO);

#ifndef MACOS
    if (!m_view_) {
        IS_SMART_NULLPTR(m_view_ = XVideoView::create_sp(),return {});
    }
    m_view_->Set_Win_ID(reinterpret_cast<void*>(this->winId()));
    CHECK_FALSE_(m_view_->Init(*vp),return {});
#else
    CHECK_FALSE_(Init(*vp));
#endif
    m_timer_id = startTimer(10);
    m_demux_task_.Start();
    m_decode_task_.Start();
    return true;
}

void XPlayVideo::Close() {
    if (m_timer_id >= 0){
        killTimer(m_timer_id);
        m_timer_id = -1;
    }
    m_demux_task_.Stop();
    m_decode_task_.Stop();
#ifndef MACOS
    if (m_view_) {
        m_view_->Close();
    }
#endif
}
