#include "xcalendar_widget.hpp"
#include <QPainter>

XCalendarWidget::XCalendarWidget(QWidget *p) :
QCalendarWidget(p) {
}

void XCalendarWidget::paintCell(QPainter * const painter,
                                const QRect &rect,const QDate date) const {

    //qDebug() << date;
    //有视频的日期特殊显示
    if (m_date_.find(date) == m_date_.end()){ //如果没有视频文件,则按照默认显示
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

