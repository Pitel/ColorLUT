#include <SDL.h>
#include "ffmpeg_lut.h"

int play(SDL_mutex *lock)
{
    av_register_all();

    AVFormatContext *pFormatCtx;

    if(av_open_input_file(&pFormatCtx, input_movie, NULL, 0,NULL)!=0)
        return -1;

    if(av_find_stream_info(pFormatCtx)<0)
        return -1;

    unsigned int i;
    AVCodecContext *pCodecCtx;

    int videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStream=i;
            break;
        }

    if(videoStream==-1)
        return -1;

    pCodecCtx=pFormatCtx->streams[videoStream]->codec;

    pCodecCtx_g = pCodecCtx;

    AVCodec *pCodec;

    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }
    if(avcodec_open(pCodecCtx, pCodec)<0)
        return -1;

    AVFrame *pFrame, *pFrameRGB;

    pFrame=avcodec_alloc_frame();

    pFrameRGB=avcodec_alloc_frame();
    if(pFrameRGB==NULL)
        return -1;


    uint8_t *buffer;
    int numBytes;
    numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
                                pCodecCtx->height);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));


    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
                   pCodecCtx->width, pCodecCtx->height);



	loop = true;

	while(loop)
	{
		av_seek_frame(pFormatCtx,videoStream, 0, AVSEEK_FLAG_FRAME);

		int frameFinished;
		AVPacket packet;

		float sync = 0.0;
		int frame_plus = 0;

		i=0;
		while(av_read_frame(pFormatCtx, &packet)>=0)
		{
			SDL_mutexP(lock);
			if(!loop)
			{
				SDL_mutexV(lock);
				break;
			}
			SDL_mutexV(lock);

			if(packet.stream_index==videoStream)
			{
				avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

				if(frameFinished)
				{

					SDL_mutexP(lock);

					static int sws_flags = SWS_BICUBIC;
					struct SwsContext *img_convert_ctx;
					img_convert_ctx = sws_getContext(
										  pCodecCtx->width,
										  pCodecCtx->height,
										  pCodecCtx->pix_fmt,
										  pCodecCtx->width,
										  pCodecCtx->height,
										  PIX_FMT_RGB24,
										  sws_flags, NULL, NULL, NULL);

					if(img_convert_ctx == NULL) {
							fprintf(stderr, "Cannot initialize the conversion context!\n");
					}

					sws_scale(img_convert_ctx, (const uint8_t * const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
					sws_freeContext(img_convert_ctx);


					pFrameRGB_g = pFrameRGB;

					SDL_mutexV(lock);

					sync += (float)1000/pCodecCtx->time_base.den - floor((float)1000/pCodecCtx->time_base.den);

					if(sync > 1.0)
					{
						sync = sync - floor(sync);
						frame_plus = 1;
					}

					SDL_Delay(1000/(pCodecCtx->time_base.den+frame_plus));
					frame_plus = 0;

					SDL_Event ev;
					ev.type = SDL_USEREVENT;
					SDL_PushEvent(&ev);

				}
			}
			av_free_packet(&packet);
		}
	}

    av_free(buffer);
    av_free(pFrameRGB);

    av_free(pFrame);
    avcodec_close(pCodecCtx);

    av_close_input_file(pFormatCtx);


    return 0;
}
