//
// Created by wong on 2024/7/21.
//

#ifndef XPLAY2_WIDGET_HPP
#define XPLAY2_WIDGET_HPP

#include <QWidget>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget;

using Widget_sptr = QSharedPointer<Widget>;

class Widget : public QWidget {
Q_OBJECT
    friend Widget_sptr new_Widget();
    explicit Widget(QWidget *parent = nullptr);
    void Construct() noexcept(false);
public:
    ~Widget() override;

private:
    Ui::Widget *ui;
};

Widget_sptr new_Widget() noexcept(false);

#endif //XPLAY2_WIDGET_HPP
