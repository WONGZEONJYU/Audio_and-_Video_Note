#include <iostream>
#include <string>
#include <memory_resource>
#include <fstream>
#include <thread>
#include <exception>
#include <SDL.h>

#undef main
using namespace std;
using namespace chrono;
using namespace this_thread;

/**
 * SDL2播放PCM
 * 本程序使用SDL2播放PCM音频采样数据。SDL实际上是对底层绘图
 * API（Direct3D，OpenGL）的封装，使用起来明显简单于直接调用底层
 * API。
 * 测试的PCM数据采用采样率44.1k, 采用精度S16SYS, 通道数2
 *
 * 函数调用步骤如下:
 *
 * [初始化]
 * SDL_Init(): 初始化SDL。
 * SDL_OpenAudio(): 根据参数（存储于SDL_AudioSpec）打开音频设备。
 * SDL_PauseAudio(): 播放音频数据。
 *
 * [循环播放数据]
 * SDL_Delay(): 延时等待播放完成。
 *
 */



// 音频PCM数据缓存
static Uint8 *s_audio_buf {};
// 目前读取的位置
static Uint8 *s_audio_pos {};
// 缓存结束位置
static Uint8 *s_audio_end {};

//音频设备回调函数(调用次数由读取多少数据决定,调用间隔由spec.samples * spec.channels * spec.format)
/*比如,1024个采样点 , 16bit格式 , 2个通道 */
void fill_audio_pcm(void *udata, Uint8 *stream, int len)
{
    (void)udata;

    SDL_memset(stream, 0, len);

    if(s_audio_pos >= s_audio_end) {    //数据读取完毕
        return;
    }

    // 数据够了就读预设长度，数据不够就只读部分（不够的时候剩多少就读取多少）
    /*当remain_buffer_len 比 len 还大 , 就先拷贝len ,
     *如果不足len , 直接拷贝剩余的长度remain_buffer_len
    */
    const auto remain_buffer_len {s_audio_end - s_audio_pos},
                length {(len < remain_buffer_len) ? len : remain_buffer_len};

    // 拷贝数据到stream并调整音量
    SDL_MixAudio(stream, s_audio_pos, static_cast<Uint32>(length), SDL_MIX_MAXVOLUME/8);

    cout << "len = " << length << "\n";
    s_audio_pos += length;  // 移动缓存指针
}

// 提取PCM文件
// ffmpeg -i input.mp4 -t 20 -codec:a pcm_s16le -ar 44100 -ac 2 -f s16le 44100_16bit_2ch.pcm
// 测试PCM文件
// ffplay -ar 44100 -ac 2 -f s16le 44100_16bit_2ch.pcm

int main()
{
    //SDL initialize
    if(SDL_Init(SDL_INIT_AUDIO)){    // 支持AUDIO
        throw string("Could not initialize SDL - ") + SDL_GetError() + "\n";
    }

    //打开PCM文件
    constexpr auto path{"44100_16bit_2ch.pcm"};
    ifstream ifs(path,ios::binary);
    if(!ifs){
        const string errmsg(string("Failed to open pcm file!\n"));
        cerr << errmsg;
        throw errmsg;
    }

    pmr::pool_options opt{};
    pmr::synchronized_pool_resource mptool(opt);

    //s_audio_buf = (uint8_t *)malloc(PCM_BUFFER_SIZE);
    //以1024个采样点一帧 2通道 16bit采样点为例(2字节),每次读取2帧数据
    constexpr auto PCM_BUFFER_SIZE (1024*2*2*2);
    try {
        s_audio_buf = static_cast<Uint8*>( mptool.allocate(PCM_BUFFER_SIZE));
    } catch (const std::bad_alloc & e) {
        ifs.close();
        cerr << e.what();
        throw e.what();
    }

    // 音频参数设置SDL_AudioSpec
    SDL_AudioSpec spec{};
    spec.freq = 44100;          // 采样频率
    spec.format = AUDIO_S16SYS; // 采样点格式
    spec.channels = 2;          // 2通道
    spec.silence = 0;
    spec.samples = 1024;       // 23.2ms -> 46.4ms 每次读取的采样数量，多久产生一次回调和 samples
    spec.callback = fill_audio_pcm; // 回调函数
    spec.userdata = nullptr;

    //打开音频设备

    if(SDL_OpenAudio(&spec, nullptr)) {
        mptool.deallocate(s_audio_buf,PCM_BUFFER_SIZE);
        mptool.release();
        ifs.close();
        const string errmsg(string("Failed to open audio device, ")  + SDL_GetError() + "\n");
        cout << errmsg;
        throw errmsg;
    }

    //play audio
    SDL_PauseAudio(0);

    long long data_count {};

    for(;;){
        // 从文件读取PCM数据
        try {
            ifs.read(reinterpret_cast<char*>(s_audio_buf),PCM_BUFFER_SIZE);
        } catch (const ios::failure &e) {
            cerr << e.what() << "\n";
            cerr << "Failed to read data from pcm file!\n";
            cerr << SDL_GetError();
            break;
        }

        // 每次缓存的长度
        const auto read_buffer_len {ifs.gcount()};

        if(read_buffer_len <= 0) {
            break;
        }

        data_count += read_buffer_len; // 统计读取的数据总字节数
        cout << "now playing " << data_count << " bytes data.\n";
        s_audio_end = s_audio_buf + read_buffer_len;    // 更新buffer的结束位置
        s_audio_pos = s_audio_buf;  // 更新buffer的起始位置
        //the main thread wait for a moment
        while(s_audio_pos < s_audio_end){
            sleep_for(10ms);  // 等待PCM数据消耗
            /* delay_ms < spec.samples / spec.freq */
        }
    }

    cout << "play PCM finish\n";
    // 关闭音频设备
    SDL_CloseAudio();

    //release some resources
    mptool.deallocate(s_audio_buf,PCM_BUFFER_SIZE);
    mptool.release();

    ifs.close();
    //quit SDL
    SDL_Quit();

    return 0;
}
