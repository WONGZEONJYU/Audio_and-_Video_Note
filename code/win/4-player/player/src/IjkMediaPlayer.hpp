//
// Created by Administrator on 2024/6/27.
//

#ifndef PLAYER_IJKMEDIAPLAYER_HPP
#define PLAYER_IJKMEDIAPLAYER_HPP

#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <string>
#include <thread>
#include <vector>
#include "MessageAbstract.hpp"
#include "FFPlay.hpp"

class IjkMediaPlayer final {
    friend class std::shared_ptr<IjkMediaPlayer> new_IjkMediaPlayer(MessageAbstract<IjkMediaPlayer*> &);
    explicit IjkMediaPlayer(MessageAbstract<IjkMediaPlayer*> &);
    void construct() noexcept(false);
public:

    ~IjkMediaPlayer();
    void start();
    void stop();
    void set_data_source(std::string &&) noexcept(false);
    void prepare_async() noexcept(false);
    int get_msg(AVMessage_Sptr &,const bool & = true);
    void Add_VideoRefreshCallback(auto &&f) noexcept(true){
        m_ff->Add_VideoRefreshCallback(std::forward<decltype(f)>(f));
    }

private:
    MessageAbstract<IjkMediaPlayer*> &m_msg_loop;
    std::thread m_msg_thread;
    std::mutex m_mux;
    std::string m_data_source;
    FFPlay_sptr m_ff;
    int m_mp_state{};

public:
    IjkMediaPlayer(const IjkMediaPlayer&) = delete;
    IjkMediaPlayer& operator=(const IjkMediaPlayer&) = delete;

};

using IjkMediaPlayer_sptr = std::shared_ptr<IjkMediaPlayer>;
IjkMediaPlayer_sptr new_IjkMediaPlayer(MessageAbstract<IjkMediaPlayer*> &) noexcept(false);

#endif
