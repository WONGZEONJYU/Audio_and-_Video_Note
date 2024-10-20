//
// Created by wong on 2024/10/20.
//

#ifndef XVIEWER01_XVIEWER_HPP
#define XVIEWER01_XVIEWER_HPP

#include <QWidget>
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class XViewer; }
QT_END_NAMESPACE

class XViewer;
using XViewer_sp = QSharedPointer<XViewer>;

class XViewer : public QWidget {
Q_OBJECT
    explicit XViewer(QWidget *parent = nullptr);
    bool Construct();
    void Destroy();
public:
    ~XViewer() override;

private:
    QSharedPointer<Ui::XViewer> m_ui_{};

public:
    static XViewer_sp create();
};

#endif
