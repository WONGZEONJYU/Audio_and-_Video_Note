//
// Created by Administrator on 2024/6/27.
//

#ifndef PLAYER_MESSAGEABSTRACT_HPP
#define PLAYER_MESSAGEABSTRACT_HPP

template<typename ...Args>
class MessageAbstract{

public:
    virtual void msg_loop(Args&&...args) = 0;
};

#endif
