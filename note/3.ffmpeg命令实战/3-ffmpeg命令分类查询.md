# 1. 部分命令

|  命令参数  |                内容                 |   命令参数   |          内容          |
| :--------: | :---------------------------------: | :----------: | :--------------------: |
|  -version  |              显示版本               |    -bsfs     |  显示可用比特流filter  |
| -buildconf |            显示编译配置             |  -protocols  |     显示可用的协议     |
|  -formats  |     显示格式 (muxers+demuxers)      |   -filters   |    显示可用的过滤器    |
|  -muxers   |           显示可用复用器            |  -pix_fmts   |   显示可用的像素格式   |
| -demuxers  |          显示可用解复用器           |   -layouts   |    显示标准声道名称    |
|  -codecs   | 显示可用编解码器(decoders+encoders) | -sample_fmts | 显示可用的音频采样格式 |
| -decoders  |           显示可用解码器            |   -colors    |   显示可用的颜色名称   |
| -encoders  |           显示可用编码器            |              |                        |

## 1.1 `ffmpeg -version` (显示版本)

<img src="assets/image-20231221173915702.png" alt="image-20231221173915702" /> 

## 1.2 `ffmpeg -buildconf` (显示编译配置)

<img src="assets/image-20231221174129308.png" alt="image-20231221174129308" />  

## 1.3 `ffmpeg -formats ` (显示格式)

<img src="assets/image-20231221175137992.png" alt="image-20231221175137992" /> 

> ```tex
> D.和E. 是命令行选项 -formats 的输出中的一部分,它们显示了与多媒体格式相关的信息
> "D." 表示"Demuxing supported",意思是FFmpeg支持解复用,也就是从特定格式中分离出音频、视频和其他流
> "E." 表示"Muxing supported",即FFmpeg支持复用,也就是将多个流(比如音频和视频)合并到特定格式中
> 因此,如果你看到某个格式标有 D. 或 E.,则表示FFmpeg可以在该格式上执行相应的操作
> ```

## 1.3 `ffmpeg -muxers` (显示可用复用器)

<img src="assets/image-20231221180120819.png" alt="image-20231221180120819" /> 