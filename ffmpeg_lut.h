#ifndef FFMPEG_LUT_H
#define FFMPEG_LUT_H

#include "SDL_mutex.h"

#include <math.h>
#include <stdbool.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>


char *input_movie;
bool loop;

AVCodecContext *pCodecCtx_g;
AVFrame *pFrameRGB_g;

int play(SDL_mutex *lock);


#endif
