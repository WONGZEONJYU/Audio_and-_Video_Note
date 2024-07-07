//
// Created by Administrator on 2024/6/27.
//

#ifndef PLAYER_MESSAGEABSTRACT_HPP
#define PLAYER_MESSAGEABSTRACT_HPP

#include <tuple>

template<typename ...Args>
class MessageAbstract{

public:
    using Args_type = std::tuple<Args...>;
    virtual void msg_loop(Args_type &&) = 0;
};

#endif
