# 1.提取YUV

* 提取**3**秒数据 , 分辨率和源视频一致

> ```bash
> ffmpeg-itest_1280x720.mp4 -t 3 -pix_fmtyuv420pyuv420p_orig.yuv
> ```

* 提取**3**秒数据 , 分辨率转为320x240

> ```bash
> ffmpeg -i test_1280x720.mp4 -t 3 -pix_fmt yuv420p -s 320x240 yuv420p_320x240.yuv
> ```

## 2. 提取RGB

* 提取**3**秒数据 , 分辨率转为 320x240

> ```bash
> ffmpeg -i test.mp4 -t 3 -pix_fmt rgb24 -s 320x240 rgb24_320x240.rgb
> ```

# 3. **RGB**和**YUV**之间的转换

> ```bash
> ffmpeg -s 320x240 -pix_fmt yuv420p -i yuv420p_320x240.yuv -pix_fmt rgb24
> rgb24_320x240_2.rgb
> ```

# 4. 提取**PCM**

> ```bash
> ffmpeg -i buweishui.mp3 -ar 48000 -ac 2 -f s16le 48000_2_s16le.pcm
> ffmpeg -i buweishui.mp3 -ar 48000 -ac 2 -sample_fmt s16 out_s16.wav
> ffmpeg -i buweishui.mp3 -ar 48000 -ac 2 -codec:a pcm_s16le out2_s16le.wav
> ffmpeg -i buweishui.mp3 -ar 48000 -ac 2 -f f32le 48000_2_f32le.pcm
> ffmpeg -i test.mp4 -t 10 -vn -ar 48000 -ac 2 -f f32le 48000_2_f32le_2.pcm
> ```

