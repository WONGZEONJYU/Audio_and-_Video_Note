#ifndef XVIEWER02_XVIDEOSHOW_HPP
#define XVIEWER02_XVIDEOSHOW_HPP

#ifdef MACOS
#include "xvideo_widget.hpp"
#else
#include <QWidget>
#endif

class XVideoShow :
#ifdef MACOS
    public XVideoWidget {
#else
    public QWidget {
#endif
    Q_OBJECT
public:
    explicit XVideoShow(QWidget * = {});
    ~XVideoShow() override = default;
};
#endif
