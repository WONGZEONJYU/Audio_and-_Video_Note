#include "xcalendar_widget.hpp"
#include <QPainter>

XCalendarWidget::XCalendarWidget(QWidget *p) :
QCalendarWidget(p) {
}

void XCalendarWidget::paintCell(QPainter * const painter,
                                const QRect &rect, QDate date) const {

    //测试代码
    if (date.day() != 4){
        QCalendarWidget::paintCell(painter, rect, date);
        return;
    }

    auto font {painter->font()};
    font.setPixelSize(40);

    if (date == selectedDate()){
        painter->setBrush(QColor(118, 178, 224)); //刷子颜色
        painter->drawRect(rect); //绘制背景图
    }

    painter->setFont(font); //设置字体
    painter->setPen(QColor(255, 0, 0));  //设置字体颜色
    painter->drawText(rect,Qt::AlignCenter,QString::number(date.day()));
}
