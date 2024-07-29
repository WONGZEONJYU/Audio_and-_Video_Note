//
// Created by Administrator on 2024/7/27.
//

#ifndef XPLAY2_XPLAY2WIDGET_HPP
#define XPLAY2_XPLAY2WIDGET_HPP

#include <QWidget>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class XPlay2Widget; }
QT_END_NAMESPACE

class XPlay2Widget;
using XPlay2Widget_sptr = QSharedPointer<XPlay2Widget>;

class XPlay2Widget final : public QWidget {
Q_OBJECT
    explicit XPlay2Widget(QWidget *parent = nullptr);
    void Construct() noexcept(false);
    void DeConstruct() noexcept;
public:
    static XPlay2Widget_sptr Handle() noexcept(false);
    ~XPlay2Widget() override;
private:
    QSharedPointer<Ui::XPlay2Widget> m_ui;
    static XPlay2Widget_sptr uni_win;
};

#endif //XPLAY2_XPLAY2WIDGET_HPP
