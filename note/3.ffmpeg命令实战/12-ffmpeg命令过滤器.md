# 1.生成测试文件

## 1. 1 找三个不同的视频每个视频截取10秒内容

> ```bash
> ffmpeg -i 沙海02.mp4 -ss 00:05:00 -t 10 -codec copy1.mp4 
> ffmpeg -i 复仇者联盟3.mp4 -ss 00:05:00 -t 10 -codec copy2.mp4 
> ffmpeg -i 红海行动.mp4 -ss 00:05:00 -t 10 -codec copy3.mp4 
> #如果音视频格式不统一则强制统一为 -vcodec libx264 -acodec aac
> ```

## 1.2 将上述1.mp4/2.mp4/3.mp4转成ts格式

> ```bash
> ffmpeg -i 1.mp4 -codec copy -vbsf h264_mp4toannexb 1.ts 
> ffmpeg -i 2.mp4 -codec copy -vbsf h264_mp4toannexb 2.ts 
> ffmpeg -i 3.mp4 -codec copy -vbsf h264_mp4toannexb 3.ts
> ```

## 1.3 转成flv格式

> ```bash
> ffmpeg -i 1.mp4 -codec copy 1.flv 
> ffmpeg -i 2.mp4 -codec copy 2.flv 
> ffmpeg -i 3.mp4 -codec copy 3.flv
> ```

> ```tex
> 分离某些封装格式(例如MP4/FLV/MKV等)中的 H.264的时候,需要首先写入SPS和PPS,否则会导致分离出来的数据没有SPS、PPS而无法播放。
> H.264码流 的SPS和PPS信息存储在AVCodecContext结构体的extradata中。需要使用ffmpeg中名称为 "h264_mp4toannexb" 的 bitstream filter处理
> ```

# 2. 开始拼接文件

## 2.1 以MP4格式进行拼接

>```bash
>方法1:ffmpeg -i "concat:1.mp4|2.mp4|3.mp4" -codec copy out_mp4.mp4 
>方法2:ffmpeg -f concat -i mp4list.txt -codec copy out_mp42.mp4
>```

## 2.2 以TS格式进行拼接

>```bash
>方法1:ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out_ts.mp4 
>方法2:ffmpeg -f concat -i tslist.txt -codec copy out_ts2.mp4
>```

## 2.3 以FLV格式进行拼接

> ```bash
> 方法1:ffmpeg -i "concat:1.flv|2.flv|3.flv" -codec copy out_flv.mp4 
> 方法2:ffmpeg -f concat -i flvlist.txt -codec copy out_flv2.mp4
> ```

* 方法1只适用部分封装格式 , 比如TS
* 建议 : 
  * 使用方法2进行拼接
  * **$\color{red}{转成TS格式再进行拼接}$**

# 3. 测试不同编码拼接

## 3.1 修改音频编码

>```bash
>ffmpeg -i 2.mp4 -vcodec copy -acodec ac3 -vbsf h264_mp4toannexb2.ts
>ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out1.mp4 结果第二段没有声音
>```

## 3.2 修改音频采样率

>```bash
>ffmpeg -i 2.mp4 -vcodec copy -acodec aac -ar 96000 -vbsf h264_mp4toannexb2.ts ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out2.mp4 第二段播放异常
>```

## 3.3 修改视频编码格式

>```bash
>ffmpeg -i 1.mp4 -acodec copy -vcodec libx265 1.ts
>ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out3.mp4
>```

3.4 修改视频分辨率

>```bash
>ffmpeg -i 1.mp4 -acodec copy -vcodec libx264 -s 800x472 -vbsf h264_mp4toannexb 1.ts
>ffmpeg -i "concat:1.ts|2.ts|3.ts" -codec copy out4.mp4
>```

# 4. 注意

* 把每个视频封装格式也统一为ts , 拼接输出的时候再输出你需要的封装格式 , 比如MP4
* 视频分辨率可以不同 , 但是编码格式需要统一
* 音频编码格式需要统一 , 音频参数(采样率/声道等)也需要统一

