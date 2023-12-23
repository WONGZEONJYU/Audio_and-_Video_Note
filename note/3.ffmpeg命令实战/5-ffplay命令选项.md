# 1. ffplay 命令 选项 (部分)

> ```tex
> 选项后的括号"()"是参数,与选项一个空格的距离
> ```

| 选项                                    | 说明                                                         |
| --------------------------------------- | ------------------------------------------------------------ |
| -x width                                | 强制显示宽度                                                 |
| -y height                               | 强制显示高度                                                 |
| -video_size (size)                      | 帧尺寸,设置显示帧存储(WxH格式) , 仅适用于经济类似原始YUV等没有包含帧大小(WxH)的视频。比如：ffplay -pixel_format yuv420p -video_size 320x420 -framerate 5 yuv420p_320x240.yuv |
| -pixel_format                           | 格式设置像素格式                                             |
| -fs                                     | 以全屏模式启动                                               |
| -an                                     | 禁用音频（不播放声音）                                       |
| -vn                                     | 禁用视频（不播放视频）                                       |
| -sn                                     | 禁用字幕（不显示字幕）                                       |
| -ss pos                                 | 根据设置的秒进行定位拖动，注意时间单位：比如'55' 55<br/>seconds, '12:03:45' ,12 hours, 03 minutes and 45 seconds, '23.189' 23.189<br/>second |
| -t duration                             | 设置播放视频/音频长度，时间单位如 -ss选项                    |
| -bytes                                  | 按字节进行定位拖动（0=off 1=on -1=auto）                     |
| -seek_interval (interval)               | 自定义左/右键定位拖动间隔（以秒为单位 , 默认值为10秒（代码没有看到实现) ) |
| -nodisp                                 | 关闭图形化显示窗口 , 视频将不显示                            |
| -noborder                               | 无边框窗口                                                   |
| -volume (vol)                           | 设置起始音量 , 音量范围[0 ~100]                              |
| -f fmt                                  | 强制使用设置的格式进行解析。比如-f s16le                     |
| -window_title (title)                   | 设置窗口标题（默认为输入文件名）                             |
| -loop (number)                          | 设置播放循环次数                                             |
| -showmode (mode)                        | 设置显示模式，可用的模式值：0 显示视频，1 显示音频波形，2 显示音频频谱。缺省为0，如果视频不存在则自动选择2 |
| -vf (filtergraph)                       | 设置视频滤镜                                                 |
| -af (filtergraph)                       | 设置音频滤镜                                                 |
| -stats                                  | 打印多个回放统计信息，包括显示流持续时间，编解码器参数，流中的当前位置，以及音频/视频同步差值。默认情况下处于启用状态，要显式禁用它则需要指定-nostats。。 |
| -fast                                   | 非标准化规范的多媒体兼容优化                                 |
| -genpts                                 | 生成pts                                                      |
| -sync (type)                            | 同步类型 将主时钟设置为audio（type=audio）， video（type=video）或external（type=ext），默认是audio为主时钟 |
| -ast (audio_stream_specifier)           | 指定音频流索引，比如-ast 3，播放流索引为3的音频流            |
| -vst  (video_stream_specifier)          | 指定视频流索引，比如-vst 4，播放流索引为4的视频流            |
| -sst (subtitle_stream_specifier)        | 指定字幕流索引，比如-sst 5，播放流索引为5的字幕流            |
| -autoexit                               | 视频播放完毕后退出                                           |
| -exitonkeydown                          | 键盘按下任何键退出播放                                       |
| -exitonmousedown                        | 鼠标按下任何键退出播放                                       |
| -codec : (media_specifier) (codec_name) | 强制使用设置的多媒体解码器，media_specifier可用值为a (音频), v (视频) 和 s (字幕)。比如 : -codec : v h264_qsv 强制视频采用h264_qsv解码 |
| -acodec (codec_name)                    | 强制使用设置的音频解码器进行音频解码                         |
| -vcodec (codec_name)                    | 强制使用设置的视频解码器进行视频解码                         |
| -scodec (codec_name)                    | 强制使用设置的字幕解码器进行字幕解码                         |
| -autorotate                             | 根据文件元数据自动旋转视频。值为0或1 ，默认为1。             |
| -framedrop                              | 如果视频不同步则丢弃视频帧。当主时钟非视频时钟时默认开启。若需禁用则使用 -noframedrop |
| -infbuf                                 | 不限制输入缓冲区大小。尽可能快地从输入中读取尽可能多的数据。播放实时流时默认启用，如果未及时读取数据，则可能会丢弃数据。此选项将不限制缓冲区的大小。若需禁用则使用-noinfbuf |

> ```tex
> 更多参考:http://www.ffmpeg.org/ffplay.html
> ```





