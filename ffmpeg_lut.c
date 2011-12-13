#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>

#include <SDL.h>
#include "ffmpeg_lut.h"

//void check_surface(AVCodecContext *pCodecCtx)
//{
//				//if(surface != NULL)
//				//	SDL_FreeSurface(surface);
//
//				if(surface == NULL)
//				{
//					Uint32 rmask, gmask, bmask, amask;
//
//					/* SDL interprets each pixel as a 32-bit number, so our masks must depend
//					   on the endianness (byte order) of the machine */
//					#if SDL_BYTEORDER == SDL_BIG_ENDIAN
//					rmask = 0xff000000;
//					gmask = 0x00ff0000;
//					bmask = 0x0000ff00;
//					amask = 0x000000ff;
//					#else
//					rmask = 0x000000ff;
//					gmask = 0x0000ff00;
//					bmask = 0x00ff0000;
//					amask = 0xff000000;
//					#endif
//
//					surface = SDL_CreateRGBSurface(SDL_SWSURFACE, pCodecCtx->width, pCodecCtx->height, 32,
//												   rmask, gmask, bmask, amask);
//					if(surface == NULL)
//					{
//						fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
//						exit(1);
//					}
//				}
//}

int play(SDL_mutex *lock)
{
    av_register_all();

    AVFormatContext *pFormatCtx;

    // Open video file
    if(av_open_input_file(&pFormatCtx, fffile, NULL, 0, NULL)!=0)
        return -1; // Couldn't open file

    // Retrieve stream information
    if(av_find_stream_info(pFormatCtx)<0)
        return -1; // Couldn't find stream information

    // Dump information about file onto standard error
    dump_format(pFormatCtx, 0, fffile, 0);



    int i;
    AVCodecContext *pCodecCtx;

    // Find the first video stream
    int videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStream=i;
            break;
        }
    if(videoStream==-1)
        return -1; // Didn't find a video stream

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
    
    pCodecCtx_g = pCodecCtx;

//	check_surface(pCodecCtx);

    AVCodec *pCodec;

    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    // Open codec
    if(avcodec_open(pCodecCtx, pCodec)<0)
        return -1; // Could not open codec

    AVFrame *pFrame, *pFrameRGB;

    // Allocate video frame
    pFrame=avcodec_alloc_frame();

    // Allocate an AVFrame structure
    pFrameRGB=avcodec_alloc_frame();
    if(pFrameRGB==NULL)
        return -1;


    uint8_t *buffer;
    int numBytes;
    // Determine required buffer size and allocate buffer
    numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
                                pCodecCtx->height);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
                   pCodecCtx->width, pCodecCtx->height);


    int frameFinished;
    AVPacket packet;
	
    i=0;
    while(av_read_frame(pFormatCtx, &packet)>=0)
    {
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream)
        {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

            // Did we get a video frame?
            if(frameFinished)
            {


				SDL_mutexP(lock);	



//                pFrameRGB->data[0] = surface->pixels;
//                pFrameRGB->linesize[0] = surface->pitch;
                // Convert the image from its native format to RGB
//                img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24,
//                            (AVPicture*)pFrame, pCodecCtx->pix_fmt,
//                            pCodecCtx->width, pCodecCtx->height);

//img_convert((AVPicture *)pFrameRGB, PIX_FMT_RGB24, (AVPicture*)pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
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

                sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
                sws_freeContext(img_convert_ctx);
                
                
				pFrameRGB_g = pFrameRGB;
				
				SDL_mutexV(lock);
				
                SDL_Delay(pCodecCtx->time_base.den);
                
				SDL_Event ev;
				ev.type = SDL_USEREVENT;				
				SDL_PushEvent(&ev);

                
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }

    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    av_close_input_file(pFormatCtx);

    return 0;
}

//int play_file(void * dumb)
//{
//    if(play() < 0)
//        fprintf(stderr,"ERROR: FFMPEG FAIL!");
//        
//	return 0;
//}
