#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdbool.h>
#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>

#define FFPLAYER

#ifdef FFPLAYER
	#include <unistd.h>
	#include "SDL_thread.h"
	#include "ffmpeg_lut.h"
	#include "SDL_mutex.h"
	
SDL_Thread *thread = NULL;
SDL_Event ev;
SDL_mutex  *lock;

static int runThread(void *data)
{
	//SDL_mutex *lock = (SDL_mutex *)data;
	play(lock);
}
    
void start_video_thread()
{
	lock = SDL_CreateMutex();
	thread = SDL_CreateThread(runThread, NULL);
}

void end_video_thread()
{
	//SDL_DestroyMutex(lock);
	SDL_KillThread(thread);
}
#endif

GLuint program;

typedef struct luts {
	char * lut_name;
	int lut_size;
} TLuts;

char *file2string(const char *path) {
	FILE *fd;
	long len, r;
	char *str;
	if (!(fd = fopen(path, "r"))) {
		fprintf(stderr, "Can't open file '%s' for reading\n", path);
		return NULL;
	}
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	printf("File '%s' is %ld long\n", path, len);
	fseek(fd, 0, SEEK_SET);
	if (!(str = malloc(len * sizeof(char)))) {
		fprintf(stderr, "Can't malloc space for '%s'\n", path);
		return NULL;
	}
	r = fread(str, sizeof(char), len, fd);
	str[r - 1] = '\0'; /* Shader sources have to term with null */
	fclose(fd);
	return str;
}

void shaderlog(GLuint obj) {
	int infologLength = 0;
	int maxLength;

	if(glIsShader(obj)) {
		glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	} else {
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	}
	char infoLog[maxLength];

	if (glIsShader(obj)) {
		glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
	} else {
		glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);
	}

	if (infologLength > 0) {
		printf("%s\n", infoLog);
	}
}

void lut(TLuts lut_table) {
	GLuint texture, format;

	//LUT
	SDL_Surface * lutimage = IMG_Load(lut_table.lut_name);
	if (lutimage == NULL) {
		fprintf(stderr, "Unable to load LUT %s!\n", lut_table.lut_name);
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	SDL_Surface * lut = SDL_CreateRGBSurface(lutimage->flags, lut_table.lut_size*lut_table.lut_size, lut_table.lut_size, lutimage->format->BitsPerPixel, lutimage->format->Rmask, lutimage->format->Gmask, lutimage->format->Bmask, lutimage->format->Amask);
	SDL_Rect rect = {0, 0, lut_table.lut_size*lut_table.lut_size, lut_table.lut_size};
	SDL_BlitSurface(lutimage, &rect, lut, NULL);
	SDL_FreeSurface(lutimage);
	if (lut->format->BytesPerPixel == 4) {
		if (lut->format->Rmask == 0x000000ff) {
			format = GL_RGBA;
		} else {
			format = GL_BGRA;
		}
	} else if (lut->format->BytesPerPixel == 3) {
		if (lut->format->Rmask == 0x000000ff) {
			format = GL_RGB;
		} else {
			format = GL_BGR;
		}
	} else {
		fprintf(stderr, "The image is not truecolor!\n");
		SDL_Quit();
		exit(EXIT_FAILURE);
	}
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, lut->format->BytesPerPixel, lut->w, lut->h, 0, format, GL_UNSIGNED_BYTE, lut->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(lut);
}


void get_texture(SDL_Surface *surface)
{
	SDL_mutexP(lock);
	
//	SDL_Surface *surf = SDL_CreateRGBSurfaceFrom((*bmp_pointer)->pixels, (*bmp_pointer)->w, (*bmp_pointer)->h, 32,(*bmp_pointer)->pitches, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
	
	//Texture
	GLuint texture, format;
	/*if (surface->format->BytesPerPixel == 4) {
		if (surface->format->Rmask == 0x000000ff) {
			format = GL_RGBA;
		} else {
			format = GL_BGRA;
		}
	} else if (surface->format->BytesPerPixel == 3) {
		if (surface->format->Rmask == 0x000000ff) {
			format = GL_RGB;
		} else {
			format = GL_BGR;
		}
	} else {
		fprintf(stderr, "The image is not truecolor!\n");
		SDL_Quit();
		#ifdef FFPLAYER 
			end_video_thread();
		#endif
		exit(EXIT_FAILURE);
	}
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	
	*/
	
	glGenTextures(1, &texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	
	glTexImage2D( GL_TEXTURE_2D, 0, 3, pCodecCtx_g->width, pCodecCtx_g->height, 0, GL_RGB, GL_UNSIGNED_BYTE, pFrameRGB_g->data[0] );
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pCodecCtx_g->width, pCodecCtx_g->height, GL_RGB, GL_UNSIGNED_BYTE, pFrameRGB_g->data[0] );
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pCodecCtx_g->width, pCodecCtx_g->height, GL_RGB, GL_UNSIGNED_BYTE, pFrameRGB_g->data[0] );
	//glTexImage2D(GL_TEXTURE_2D, 0, 4 , pCodecCtx_g->width, pCodecCtx_g->height, 0, GL_RGB, GL_UNSIGNED_INT, pFrameRGB_g->data[0]);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, 512, 256, GL_RGB, GL_UNSIGNED_BYTE, pFrameRGB_g->data[0]);
	//glGenerateMipmap(GL_TEXTURE_2D);
	
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	
	
	SDL_mutexV(lock);
	
}


int main(int argc, char **argv) {
	//Help
	if (argc < 3) {
		printf("Usage: colorLUT <image> [-s n] <LUTs>...\n");
		printf("--------------------------------------\n");
		printf("<image> - input image\n");
		printf("<LUTs> - images of lookup table\n");
		printf("-s n - 'n' = size of lookup table (default 16)\n\n");
		printf("Example: colorLUT input.png -s 2 lut1.png lut2.png -s 16 lut3.png lut4.png\n");
		return EXIT_FAILURE;
	}
	
	
	#ifndef FFPLAYER	
	char *input_name = argv[1];
	#endif
	
	int input_luts_size = 0;
	TLuts input_luts[argc - 2];	
	int lsize = 16; //defaul lookup table size
	
	for(int i = 2; i < argc; i++)
	{
		if (!strcmp(argv[i],"-s"))
		{
			lsize = atoi(argv[++i]);
			i++;
		}
		
		if(i > argc)
		{
			printf("ERROR: No lookup table image?\n");
			return EXIT_FAILURE;
		}
		
		input_luts[input_luts_size].lut_name = argv[i];
		input_luts[input_luts_size].lut_size = lsize;
		input_luts_size++;
	}

	//SDL init
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}
	
	#ifdef FFPLAYER	
		fffile = argv[1];
		start_video_thread();
		
		while(pFrameRGB_g == NULL)
			usleep(50*1000);
	#endif

	
	#ifndef FFPLAYER
	//Load image
	SDL_Surface * surface = IMG_Load(input_name);
	if (surface == NULL) {
		fprintf(stderr, "Unable to load image %s!\n", input_name);
		SDL_Quit();
		return EXIT_FAILURE;
	}
	#endif

	//Window and OpenGL init
	if (SDL_SetVideoMode(pCodecCtx_g->width, pCodecCtx_g->height, 0, SDL_OPENGL) == NULL) {
		fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}
	


	SDL_WM_SetCaption("colorLUT", NULL);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		SDL_Quit();
		return EXIT_FAILURE;
	}

	printf("Using GLEW Version %s\n", glewGetString(GLEW_VERSION));

	if (!GLEW_VERSION_2_0) {
		fprintf(stderr, "This machine sucks and does not support OpenGL 2.0!\n");
		SDL_Quit();
		return EXIT_FAILURE;
	}

	//Building shader
	const char* fs_src = file2string("colorLUT.fs");
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER_ARB);
	glShaderSource(fs, 1, &fs_src, NULL);
	glCompileShader(fs);
	shaderlog(fs);

	program = glCreateProgram();
	glAttachShader(program, fs);
	glLinkProgram(program);
	shaderlog(program);

	glUseProgram(program);

	get_texture(surface);
	
	glUniform2f(glGetUniformLocation(program, "resolution"), pCodecCtx_g->width, pCodecCtx_g->height);	
	glUniform1i(glGetUniformLocation(program, "tex"), 0);
	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(program, "lut"), 1);
	
	//Geometry
	glEnable(GL_VERTEX_ARRAY);
	const short vertices[] = {-1,1, 1,1, 1,-1, -1,-1};
	glVertexPointer(2, GL_SHORT, 0, vertices);

	//Event loop
	int lutindex = 0;	
	printf("LUT: %s\tsize: %d\n", input_luts[lutindex].lut_name,input_luts[lutindex].lut_size);	
	SDL_Event event;
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	do {
		if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
			break;
		} else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
			lutindex++;
			if (lutindex >= input_luts_size) {
				lutindex = 0;
			}
			
			printf("LUT: %s\tsize: %d\n", input_luts[lutindex].lut_name,input_luts[lutindex].lut_size);			
		}

		#ifdef FFPLAYER 
		if (event.type == SDL_USEREVENT)
			get_texture(surface);
		#endif
		
		lut(input_luts[lutindex]);
		
		GLint lsize_loc = glGetUniformLocation(program,"lut_size");
		glUniform1i(lsize_loc,input_luts[lutindex].lut_size);
		
		glActiveTexture(GL_TEXTURE1);
		glDrawArrays(GL_QUADS, 0, 8);
		SDL_GL_SwapBuffers();
	} while (SDL_WaitEvent(&event));
	
	#ifdef FFPLAYER 
		end_video_thread();
	#endif
	
	if(surface != NULL)
		SDL_FreeSurface(surface);
	
	SDL_Quit();
	

}
