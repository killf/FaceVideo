#include <stdio.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/file.h>
#include <libavutil/avutil.h>

#include "SDL.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define PIXEL_W 1920
#define PIXEL_H 1200
#define IMAGE_SIZE 3456000 // 1920 * 1200 * 1.5

struct buffer_data
{
    uint8_t *ptr;
    size_t size; ///< size left in the buffer
};

SDL_Window *sdlWindow;
SDL_Renderer *sdlRenderer;
SDL_Texture *sdlTexture;

AVFormatContext *fmt_ctx = NULL;
AVIOContext *avio_ctx = NULL;
uint8_t *buffer = NULL, *avio_ctx_buffer = NULL;
size_t buffer_size, avio_ctx_buffer_size = 4096;
const char *input_filename = "videos/tuhui11.mp4";
int ret = 0;
struct buffer_data bd = {0};

AVCodec *codec;
AVCodecContext *codec_ctx;
AVPacket avpkt;
AVFrame *frame;
int frame_count;

int init_sdl2()
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        printf("%s\n", SDL_GetError());
        return -1;
    }

    sdlWindow = SDL_CreateWindow("SDL example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, PIXEL_W, PIXEL_H, SDL_WINDOW_OPENGL);
    if (sdlWindow == NULL)
    {
        printf("%s\n", SDL_GetError());
        return -1;
    }

    sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (sdlRenderer == NULL)
    {
        printf("%s\n", SDL_GetError());
        return -1;
    }

    sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, PIXEL_W, PIXEL_H);
    if (sdlTexture == NULL)
    {
        printf("%s\n", SDL_GetError());
        return -1;
    }

    SDL_RenderClear(sdlRenderer);
    SDL_RenderPresent(sdlRenderer);
}

static int read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    struct buffer_data *bd = (struct buffer_data *)opaque;
    buf_size = FFMIN(buf_size, bd->size);

    printf("ptr:%p size:%zu\n", bd->ptr, bd->size);

    /* copy internal buffer data to buf */
    memcpy(buf, bd->ptr, buf_size);
    bd->ptr += buf_size;
    bd->size -= buf_size;

    return buf_size;
}

int main(int argc, char *argv[])
{
    init_sdl2();

    /* register codecs and formats and other lavf/lavc components*/
    av_register_all();

    /* slurp file content into buffer */
    ret = av_file_map(input_filename, &buffer, &buffer_size, 0, NULL);
    if (ret < 0)
        goto end;

    /* fill opaque structure used by the AVIOContext read callback */
    bd.ptr = buffer;
    bd.size = buffer_size;

    if (!(fmt_ctx = avformat_alloc_context()))
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    avio_ctx_buffer = (uint8_t *)av_malloc(avio_ctx_buffer_size);
    if (!avio_ctx_buffer)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    avio_ctx = avio_alloc_context(avio_ctx_buffer, avio_ctx_buffer_size,
                                  0, &bd, &read_packet, NULL, NULL);
    if (!avio_ctx)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    fmt_ctx->pb = avio_ctx;

    /* 打开文件 */
    ret = avformat_open_input(&fmt_ctx, NULL, NULL, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not open input\n");
        goto end;
    }

    /* 获取视频流信息 */
    ret = avformat_find_stream_info(fmt_ctx, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "Could not find stream information\n");
        goto end;
    }
    av_dump_format(fmt_ctx, 0, input_filename, 0);

    /*查找视频流*/
    int video_indx = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_indx = i;
            break;
        }
    }
    if (video_indx < 0)
    {
        fprintf(stderr, "Could not find video stream\n");
        goto end;
    }

    /* 获取解码器 */
    codec = avcodec_find_decoder(fmt_ctx->streams[video_indx]->codec->codec_id);
    if (!codec)
    {
        fprintf(stderr, "Codec not found\n");
        goto end;
    }
    printf("codec:%p\n", codec);

    /* 分配内存 */
    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        fprintf(stderr, "Could not allocate video codec context\n");
        goto end;
    }
    printf("codec_ctx=%p\n", codec_ctx);

    /* 设置解码器参数 */
    if (avcodec_parameters_to_context(codec_ctx, fmt_ctx->streams[video_indx]->codecpar) != 0)
    {
        fprintf(stderr, "Failed to copy codec parameters to decoder context.\n");
        goto end;
    }

    /* 打开解码器 */
    if (avcodec_open2(codec_ctx, codec, NULL) < 0)
    {
        fprintf(stderr, "Could not open codec\n");
        goto end;
    }

    /* 分配视频帧 */
    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, "Could not allocate video frame\n");
        goto end;
    }

    /* 开始读取视频帧 */
    av_init_packet(&avpkt);
    // av_read_frame(fmt_ctx, &avpkt);

    int err = 0;
    while (err >= 0)
    {
        if ((err = av_read_frame(fmt_ctx, &avpkt)) < 0)
        {
            fprintf(stderr, "av_read_frame : %d\n", err);
            break;
        }

        err = avcodec_send_packet(codec_ctx, &avpkt);
        if (err < 0)
        {
            fprintf(stderr, "avcodec_send_packet : %d, %d\n", err, avpkt.size);
            break;
        }

        while (1)
        {
            err = avcodec_receive_frame(codec_ctx, frame);
            if (err == AVERROR(EAGAIN) || err == AVERROR_EOF)
            {
                // fprintf(stderr, "avcodec_receive_frame1 : %d\n", err);
                err = 0;
                break;
            }
            else if (err < 0)
            {
                fprintf(stderr, "avcodec_receive_frame2 : %d\n", err);
                break;
            }

            /* TODO 显示一帧 */
            SDL_Rect sdlRect;
            sdlRect.x = 0;
            sdlRect.y = 0;
            sdlRect.w = PIXEL_W;
            sdlRect.h = PIXEL_H;

            SDL_UpdateYUVTexture(sdlTexture, NULL,
                                 frame->data[0], frame->linesize[0],
                                 frame->data[1], frame->linesize[1],
                                 frame->data[2], frame->linesize[2]);
            SDL_RenderClear(sdlRenderer);
            SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
            SDL_RenderPresent(sdlRenderer);

            printf("frame : %d\n", frame_count++);
        }

        av_packet_unref(&avpkt);
    }
    av_frame_free(frame);

end:
    avformat_close_input(&fmt_ctx);
    /* note: the internal buffer could have changed, and be != avio_ctx_buffer */
    if (avio_ctx)
    {
        av_freep(&avio_ctx->buffer);
        av_freep(&avio_ctx);
    }
    av_file_unmap(buffer, buffer_size);

    if (ret < 0)
    {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}