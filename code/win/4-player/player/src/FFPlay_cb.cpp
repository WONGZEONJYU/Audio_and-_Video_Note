#include "FFPlay.hpp"
#include <algorithm>


int FFPlay::decode_interrupt_cb(void *_this)
{
    static int64_t s_pre_time {};
    const auto cur_time  {av_gettime_relative() / 1000};
    //std::cerr << "decode_interrupt_cb interval:\t" << (cur_time - s_pre_time) << "\n";
    s_pre_time = cur_time;
    auto this_{static_cast<FFPlay*>(_this)};
    return this_->m_abort_request;
}

void FFPlay::sdl_audio_callback(void *_this, Uint8 *stream, int len)
{
    auto this_{static_cast<FFPlay*>(_this)};

    while (len > 0){
        if (this_->m_audio_buf_index >= this_->m_audio_buf_size) { //此处代表上一次剩余数据已经被拷贝完毕,需重获取新数据
            const auto audio_size {this_->audio_decode_frame()};
            if (audio_size < 0){
                this_->m_audio_buf = nullptr;
                this_->m_audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / this_->m_audio_tgt.frame_size * this_->m_audio_tgt.frame_size;

            } else{
                this_->m_audio_buf_size = audio_size; /*读到多少字节数据*/
            }

            this_->m_audio_buf_index = 0;
        }

        auto len1{this_->m_audio_buf_size - this_->m_audio_buf_index};

        if (len1 > len){ //超出缓冲区给的长度,先拷贝缓冲区给出的长度
            len1 = len;
        }

        if (this_->m_audio_buf){
            std::copy_n(this_->m_audio_buf + this_->m_audio_buf_index,len1,stream);
        }

        len -= static_cast<int>(len1); //读取了多少数据,len要减去相应的长度
        stream += len1; //stream也需偏移
        /* 更新m_audio_buf_index,指向audio_buf中未被拷贝到stream的数据(剩余数据)的起始位置 */
        this_->m_audio_buf_index += static_cast<int>(len1);
    }

    if (!isnan(this_->m_audio_clock)){
        set_clock(&(this_->m_audclk),this_->m_audio_clock);
    }

}
