# 1. ffmpeg拉流

## 1.1 直播拉流

>```bash
>ffplay rtmp://server/live/streamName 
>ffmpeg -i rtmp://server/live/streamName -c copy dump.flv
>```

> ```tex
> 对于不是rtmp的协议 -c copy要谨慎使用
> ```

## 1.2 可用地址

> ```tex
> HKS：rtmp://live.hkstv.hk.lxdns.com/live/hks2
> 大熊兔(点播)：rtsp://184.72.239.149/vod/mp4://BigBuckBunny_175k.mov
> CCTV1高清：http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8
> 
> ffmpeg -i http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8 -c copy cctv1.ts
> 
> ffmpeg -i http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8 cctv1.flv
> 
> ffmpeg -i http://ivi.bupt.edu.cn/hls/cctv1hd.m3u8 -acodec aac -vcodec libx264 cctv1-2.flv
> CCTV3高清：http://ivi.bupt.edu.cn/hls/cctv3hd.m3u8
> CCTV5高清：http://ivi.bupt.edu.cn/hls/cctv5hd.m3u8
> CCTV5+高清：http://ivi.bupt.edu.cn/hls/cctv5phd.m3u8
> CCTV6高清：http://ivi.bupt.edu.cn/hls/cctv6hd.m3u8
> ```

> ```tex
> 对于不是rtmp的协议 -c copy要谨慎使用
> ```

# 2. ffmpeg推流

## 2.1直播推流

> ```bash
> ffmpeg -re -i out.mp4 -c copy flv rtmp://server/live/streamName -re
> #表示按时间戳读取文件
> ```

[[Nginx搭建rtmp流媒体服务器(Ubuntu 16.04)]](https://www.jianshu.com/p/16741e363a77)