#include "recordthread.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}
#include <QDebug>
#include <QFile>
#ifdef Q_OS_MAC
#define FORMAT_NAME "avfoundation"
#define PCM_FILE "/Users/caitou/Desktop/out.yuv"
#define FORMAT_INPUT_URL "0:"
#else
#define FORMAT_INPUT_URL "video=FaceTime高清摄像头（内建）"
#define FORMAT_NAME "dshow"
#define PCM_FILE "C:\\Workspaces\\out.yuv"
#endif
#define ERROR_BUFFER(res) \
    char errbuf[1024];\
    av_strerror(res, errbuf, sizeof(errbuf));

RecordThread::RecordThread(QObject *parent) : QThread(parent)
{
    connect(this, &QThread::finished, this, &QThread::deleteLater);
}

RecordThread::~RecordThread() {
    disconnect();
    requestInterruption();
    quit();
    wait();

    qDebug() << "RecordThread::~RecordThread()";
}

void RecordThread::run() {
#ifdef Q_OS_MAC
    const AVInputFormat *fmt = av_find_input_format(FORMAT_NAME);
#else
    AVInputFormat *fmt = av_find_input_format(FORMAT_NAME);
#endif
    if (!fmt) {
        qDebug() << "av_find_input_format error";
        return;
    }

    AVFormatContext *context = nullptr;
    AVDictionary *options = nullptr;
    av_dict_set(&options, "video_size", "1280x720", 0);
    av_dict_set(&options, "pixel_format", "uyvy422", 0);
#ifdef Q_OS_MAC
    av_dict_set(&options, "framerate", "ntsc", 0);  //https://ffmpeg.org/ffmpeg-devices.html#Options-19
#else
    av_dict_set(&options, "framerate", "30", 0);
#endif

    int res = avformat_open_input(&context, FORMAT_INPUT_URL, fmt, &options);
    if (res != 0) {
        ERROR_BUFFER(res);
        qDebug() << "avformat_open_input error" << errbuf;
        return;
    }

    QFile file(PCM_FILE);
    res = file.open(QFile::WriteOnly);
    if (res == 0) {
        qDebug() << "open file error" << res;
        avformat_close_input(&context);
        return;
    }

    // 计算每一帧的大小（在 Mac 平台从 packet 中读取到的 size 和 真实的 image_buffer_size 不一致）
    AVStream *stream = context->streams[0];
    int pixel_width = stream->codecpar->width;
    int pixel_height = stream->codecpar->height;
    qDebug() << "pixel_width" << pixel_width << "pixel_height" << pixel_height;
    AVPixelFormat pixel_fmt = AV_PIX_FMT_YUYV422;
    int image_buffer_size = av_image_get_buffer_size(pixel_fmt, pixel_width, pixel_height, 1);

    AVPacket packet;
    while(!isInterruptionRequested()) {
        res = av_read_frame(context, &packet);
        if (res == 0) {
            qDebug() << "frame packet size" << packet.size << "image buffer size" << image_buffer_size;
            file.write((char * )packet.data, image_buffer_size);
        } else if (res == AVERROR(EAGAIN)) { // 资源临时不可用
            continue;
        } else {
            ERROR_BUFFER(res);
            qDebug() << "av_read_frame error" << errbuf;
            break;
        }

    }

    file.close();
    avformat_close_input(&context);
}
