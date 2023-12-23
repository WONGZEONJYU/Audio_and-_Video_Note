# 1. ffplay简单过滤器

## 1.1 视频旋转  

* `ffplay -i test.mp4 -vf transpose=1`

## 1.2 视频反转  

* `ffplay test.mp4 -vf hflip`
* `ffplay test.mp4 -vf vflip`

## 1.3 视频旋转和反转

* `ffplay test.mp4 -vf hflip,transpose=1`

## 1.4 音频变速播放

* `ffplay -i test.mp4 -af atempo=2`

## 1.5 视频变速播放

* `ffplay -i test.mp4 -vf setpts=PTS/2`

## 1.6 音视频同时变速

* `ffplay -i test.mp4 -vf setpts=PTS/2 -af atempo=2`

[[更多参考]](http://www.ffmpeg.org/ffmpeg-filters.html  )

