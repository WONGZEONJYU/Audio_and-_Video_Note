//
// Created by wong on 2024/8/26.
//

#ifndef INC_1_RGB_TEST_TESTRGB_HPP
#define INC_1_RGB_TEST_TESTRGB_HPP

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class TestRGB; }
QT_END_NAMESPACE

class TestRGB : public QWidget {
Q_OBJECT
    void paintEvent(QPaintEvent *) override;
public:
    explicit TestRGB(QWidget *parent = nullptr);

    ~TestRGB() override;

private:
    Ui::TestRGB *ui;
};


#endif //INC_1_RGB_TEST_TESTRGB_HPP
