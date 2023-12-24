# 1. ffmpeg命令视频录制(Windows)

# 1.1 先安装dshow软件 Screen Capturer Recorder

* [[项目地址链接]](https://sourceforge.net/projects/screencapturer/files/)
* 然后查看可用设备名字 : `ffmpeg -list_devices true -f dshow -i dummy`

# 2. 音视频录制

## 2.1 录制视频 (默认参数)

* 桌面 `ffmpeg -f dshow -i video="screen-capture-recorder" v-out.mp4`
* 摄像头 : `ffmpeg -f dshow -i video="Integrated Webcam" -y v-out2.flv` (要根据自己的摄像头名称)

## 2.2 录制声音 (默认参数)

* 系统声音 : `ffmpeg -f dshow -i audio="virtual-audio-capturer" a-out.aac`
* 系统+麦克风声音 : 

> ```bash
> ffmpeg -f dshow -i audio="麦克风 (Realtek Audio)" -f dshow -i audio="virtual-audio-capturer" -filter_complex amix=inputs=2:duration=first:dropout_transition=2 a-out2.aac
> ```

## 2.3 同时录制声音和视频

> ```bash
> ffmpeg -f dshow -i audio="麦克风 (Realtek Audio)" -f dshow -i audio="virtualaudio-capturer" -filter_complex amix=inputs=2:duration=first:dropout_transition=2 -f dshow -i video="screen-capture-recorder" -y av-out.flv
> ```

# 3. 查看视频录制的可选参数

> ```bash
> ffmpeg -f dshow -list_options true -i video="screen-capture-recorder"
> ```

# 4. 查看音频设备可选参数

> ```bash
> ffmpeg -f dshow -list_options true -i audio="virtual-audio-capturer“
> ffmpeg -f dshow -list_options true -i audio="麦克风 (Realtek Audio)"
> ```

# 5. 指定参数录制音视频

> ```bash
> ffmpeg -f dshow -i audio="麦克风 (Realtek Audio)" -f dshow -i audio="virtual-audio-capturer" -filter_complex amix=inputs=2:duration=first:dropout_transition=2 -f dshow -video_size 1920x1080 -framerate 15 -pixel_format yuv420p -i video="screen-capturerecorder" -vcodec h264_qsv -b:v 3M -y av-out.flv
> ```

> ```bash
> ffmpeg -f dshow -i audio="麦克风 (Realtek Audio)" -f dshow -i audio="virtual-audio-capturer" -filter_complex amix=inputs=2:duration=first:dropout_transition=2 -f dshow -i video="screen-capture-recorder" -vcodec h264_qsv -b:v 3M -r 15 -y avout2.mp4
> ```

> ```bash
> ffmpeg -f dshow -i audio="麦克风 (Realtek Audio)" -f dshow -i audio="virtual-audio-capturer" -filter_complex amix=inputs=2:duration=first:dropout_transition=2 -f dshow -framerate 15 -pixel_format yuv420p -i video="screen-capture-recorder" -vcodec h264_qsv -b:v 3M -r 15 -y av-out3.mp4
> ```



