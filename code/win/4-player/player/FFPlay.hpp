//
// Created by Administrator on 2024/6/27.
//

#ifndef PLAYER_FFPLAY_HPP
#define PLAYER_FFPLAY_HPP

#include <string>
#include "MessageQueue.hpp"

class FFPlay final : public MessageQueue{

public:
    explicit FFPlay();
    ~FFPlay() override = default;
    void prepare_async(const std::string &);

private:

public:
    FFPlay(const FFPlay&) = delete;
    FFPlay& operator=(const FFPlay&) = delete;
};

#endif //PLAYER_FFPLAY_HPP
