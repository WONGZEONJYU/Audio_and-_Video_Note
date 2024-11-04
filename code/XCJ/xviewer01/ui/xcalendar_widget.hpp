#ifndef XCALENDAR_HPP_
#define XCALENDAR_HPP_

#include <QCalendarWidget>

class XCalendarWidget final : public QCalendarWidget{

    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;

public:
    explicit XCalendarWidget(QWidget *p);

};


#endif
