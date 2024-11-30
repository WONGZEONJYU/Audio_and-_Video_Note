//
// Created by wong on 2024/11/30.
//

#include "xvideoshow.hpp"

XVideoShow::XVideoShow(QWidget *p) :
#ifdef MACOS
XVideoWidget(p)
#else
QWidget(p)
#endif
{
}
