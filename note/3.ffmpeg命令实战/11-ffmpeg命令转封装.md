# 1. 保持编码格式

> ```bash
> ffmpeg -i test.mp4 -vcodec copy -acodec copy test_copy.ts 
> ffmpeg -i test.mp4 -codec copy test_copy2.ts
> ```

# 2. 改变编码格式

> ```bash
> ffmpeg -i test.mp4 -vcodec libx265 -acodec libmp3lame out_h265_mp3.mkv
> ```

# 3. 修改帧率

> ```bash
> ffmpeg -i test.mp4 -r 15 -codec copy output.mp4 (错误命令)
> ffmpeg -i test.mp4 -r 15 output2.mp4
> ```

# 4. 修改视频码率

> ```bash
> ffmpeg -i test.mp4 -b 400k output_b.mkv(此时音频也被重新编码)
> ```

# 5. 修改视频码率

> ```bash
> ffmpeg -i test.mp4 -b:v 400k output_bv.mkv
> ```

# 6. 修改音频码率

> ```bash
> ffmpeg -i test.mp4 -b:a 192k output_ba.mp4 
> #如果不想重新编码video，需要加上-vcodec copy
> ```

# 7. 修改音视频码率

> ```bash
> ffmpeg -i test.mp4 -b:v 400k -b:a 192k output_bva.mp4
> ```

# 8. 修改视频分辨率

> ```bash
> ffmpeg -i test.mp4 -s 480x270 output_480x270.mp4
> ```

# 9. 修改音频采样率

> ```bash
> ffmpeg -i test.mp4 -ar 44100 output_44100hz.mp4
> ```

