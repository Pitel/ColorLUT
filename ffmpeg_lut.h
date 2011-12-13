#ifndef FFMPEG_LUT_H
#define FFMPEG_LUT_H
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>

char *fffile;
SDL_Surface *surface;

 AVCodecContext *pCodecCtx_g;
 AVFrame *pFrameRGB_g;

int play(SDL_mutex *lock);


#endif
