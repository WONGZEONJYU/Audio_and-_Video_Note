#ifndef XCALENDAR_HPP_
#define XCALENDAR_HPP_

#include <QCalendarWidget>

class XCalendarWidget final : public QCalendarWidget{

    void paintCell(QPainter *painter,
                   const QRect &rect,
                   QDate date) const override;

public:
    explicit XCalendarWidget(QWidget *p);
    inline void AddDate(const QDate &d){
        m_date_.insert(d);
    }

    inline void ClearDate(){
        m_date_.clear();
    }

private:
    QSet<QDate> m_date_;
};

#endif
