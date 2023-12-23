# 1.保留封装格式

* `ffmpeg -i test.mp4-acodec copy -vn audio.mp4`
* `ffmpeg -i test.mp4 -vcodec copy -an video.mp4`

# 2. 提取视频

保留编码格式 : `ffmpeg -i test.mp4 -vcodec copy -an test_copy.h264`

强制格式 : `ffmpeg -i test.mp4 -vcodec libx264 -an test.h264`

# 3. 提取音频

保留编码格式 : `ffmpeg -i test.mp4 -acodec copy -vn test.aac`

强制格式 : `ffmpeg -i test.mp4 -acodec libmp3lame -vn test.mp3`

