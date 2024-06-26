#define TEST 0

#if !TEST

#include <QApplication>
#include "mainwindow.hpp"

int main(int argc, char *argv[]) {

    QApplication a(argc, argv);

    try {
        auto win{new_MainWindow()};
        win->show();
    } catch (const std::exception &e) {
        qDebug() << e.what();
        return -1;
    }

    return QApplication::exec();
}

#else

#include <iostream>
#include <type_traits>
#include <thread>

#include "MessageQueue.hpp"

using namespace std;
using namespace chrono;
using namespace this_thread;

class Base {

public:

    int m_1{};

    Base(const Base& other) {
        std::cerr << "Base(const Base& other)\n";
        m_1 = other.m_1;
    }

    Base(Base&& other) noexcept {
        std::cerr << "Base(Base&& other) noexcept\n";
        m_1 = other.m_1;
        other.m_1 = 0;
    }

    Base() = default;

    Base& operator=(const Base& other) {
        std::cerr << "Base& operator=(const Base& other)\n";
        if (this != &other){
            m_1 = other.m_1;
        }
        return *this;
    }

    Base& operator=(Base&& other) noexcept {

        std::cerr << "Base& operator=(Base&& other) noexcept\n";
        if (this != &other){
            m_1 = other.m_1;
            other.m_1 = 0;
        }

        return *this;
    }

    virtual ~Base() = default;

};

class Derived : public Base {
public:
    explicit Derived() = default;
//    Derived(const Derived& other) : Base(other) {
//        // 拷贝构造函数实现
//    }

    Derived(const Derived& other) = default;

//    Derived(Derived&& other) noexcept : Base(std::move(other)) {
//        // 移动构造函数实现
//    }
    Derived(Derived&& other) noexcept = default;
    Derived& operator=(const Derived& other) {
        if (this != &other) {
            Base::operator=(other);
            // 拷贝赋值运算符实现
        }
        return *this;
    }

    Derived& operator=(Derived&& other) noexcept {
        if (this != &other) {
            Base::operator=(std::move(other));
            // 移动赋值运算符实现
        }
        return *this;
    }

    ~Derived() override = default;
};

int main(int argc,char *argv[])
{
//    Derived s;
//    s.m_1 = 100;
//    Derived s1(std::move(s));
    //s1 = std::move(s);

   // std::cerr << s1.m_1 << "\n";

    MessageQueue q;
    q.start();

    std::thread t1([&q]{

        int i{};
        for (;;) {
            //AVMessage s(i++);
            if (q.msg_put(i++) < 0){
                break;
            }
            sleep_for(100ms);
        }
        cerr << "t1 complete\n";
    });

    std::thread t2([&q]{
        for(;;){
            AVMessage_Sptr s;
            if (q.msg_get(s, true) < 0){
                break;
            }
            cerr << "t2: " << s->m_what << "\n";
            sleep_for(500ms);
        }
        cerr << "t2 complete\n";
    });

    std::thread t3([&q]{
        for(;;){
            AVMessage_Sptr s;
            if (q.msg_get(s, true) < 0){
                break;
            }
            cerr << "t3: " << s->m_what << "\n";
            sleep_for(500ms);
        }
        cerr << "t3 complete\n";
    });

    for(;;){
        cerr << "input: \n";
        int val{};
        cin >> val;
        if (val){
            q.abort();
            break;
        }
    }

    t1.join();
    t2.join();
    t3.join();

    std::cerr << "\n";
    return 0;
}

#endif

