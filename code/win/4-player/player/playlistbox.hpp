//
// Created by wong on 2024/6/22.
//

#ifndef PLAYER_PLAYLISTBOX_HPP
#define PLAYER_PLAYLISTBOX_HPP

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class PlayListBox; }
QT_END_NAMESPACE

class PlayListBox : public QWidget {
    Q_OBJECT

public:
    explicit PlayListBox(QWidget *parent = nullptr);

    ~PlayListBox() override;

private:
    Ui::PlayListBox *ui;
};


#endif //PLAYER_PLAYLISTBOX_HPP
