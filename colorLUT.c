#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_image.h>
#include <GL/glew.h>

//Potrebne pro FFMPEG
#include <unistd.h>
#include "SDL_thread.h"
#include "ffmpeg_lut.h"
#include "SDL_mutex.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

GLuint program, texture_img;
bool generated = false;
bool ffplayer = false;

//Struktura slouzi k ulozeni obrazku (cestu) lookup tabulek a jejich velikosti
typedef struct luts {
	char * lut_name;
	int lut_size;
} TLuts;

SDL_Thread *thread = NULL;
SDL_mutex  *lock;

//Spustime prehravani videa v novem vlakne
static int runThread(void * u)
{
	play(lock);

	UNUSED(u);
	return 0;
}

//start noveho vlakna
void start_video_thread()
{
	lock = SDL_CreateMutex();
	thread = SDL_CreateThread(runThread, NULL);
}

//Korektni ukonceni
void end_video_thread()
{
	int stat;
	SDL_WaitThread(thread, &stat);

	if(stat != 0)
		SDL_KillThread(thread);

	SDL_DestroyMutex(lock);
}


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

	if(!ffplayer)
	{
		//Texture
		GLuint format;
		if (surface->format->BytesPerPixel == 4) {
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
			exit(EXIT_FAILURE);
		}

		if(!generated)
		{
			glGenTextures(1, &texture_img);
		}

		glBindTexture(GL_TEXTURE_2D, texture_img);
		glActiveTexture(GL_TEXTURE0);

		if(!generated)
			glTexImage2D(GL_TEXTURE_2D, 0, surface->format->BytesPerPixel, surface->w, surface->h, 0, format, GL_UNSIGNED_BYTE, surface->pixels);
		else
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h, GL_RGB, GL_UNSIGNED_BYTE, surface->pixels );

		glGenerateMipmap(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);


		if(!generated)
			generated = true;
	}
	else
	{	//FFPLAYER
		SDL_mutexP(lock);

		if(!generated)
		{
			glGenTextures(1, &texture_img);
		}

		glBindTexture(GL_TEXTURE_2D, texture_img);
		glActiveTexture(GL_TEXTURE0);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		if(!generated)
			glTexImage2D( GL_TEXTURE_2D, 0, 3, pCodecCtx_g->width, pCodecCtx_g->height, 0, GL_RGB, GL_UNSIGNED_BYTE, pFrameRGB_g->data[0] );
		else
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pCodecCtx_g->width, pCodecCtx_g->height, GL_RGB, GL_UNSIGNED_BYTE, pFrameRGB_g->data[0] );

		glGenerateMipmap(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);

		if(!generated)
			generated = true;

		SDL_mutexV(lock);

		UNUSED(surface);
	}

}

int main(int argc, char **argv) {
	//Help
	if (argc < 3) {
			printf("Usage: colorLUT <input> [-s n] <LUTs>...\n");
			printf("--------------------------------------\n");
			printf("<input> - input image, *.avi or *.mpg file\n");
			printf("-s n - 'n' = size of lookup table (default 16)\n");
			printf("<LUTs> - images of lookup table\n\n");
			printf("Example: colorLUT input.png -s 2 lut1.png lut2.png -s 8 lut3.png lut4.png\n");
			printf("Example: colorLUT input.avi lut.jpg\n");

		return EXIT_FAILURE;
	}

	//SDL init
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVERYTHING) < 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_Surface *surface = NULL;

	char *input_name = argv[1];

	int input_luts_size = 0;
	TLuts input_luts[argc - 2];

	int lsize = 16; //default lookup table size

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

	char *pch = strrchr(input_name,'.') + 1;
	int len = strlen(pch);
	char ext[len];

	for(int i = 0; i < len; i++)
		ext[i] = (char) tolower(pch[i]);

	if(!strcmp(ext,"avi") || !strcmp(ext,"mpg") || !strcmp(ext,"mpeg"))
		ffplayer = true;


	if(ffplayer)
	{
		input_movie = input_name;
		start_video_thread();

		while(pFrameRGB_g == NULL)
			usleep(50*1000);
	}


	if(!ffplayer)
	{
		//Load image
		surface = IMG_Load(input_name);
		if (surface == NULL) {
			fprintf(stderr, "Unable to load image %s!\n", input_name);
			SDL_Quit();
			return EXIT_FAILURE;
		}

		//Window and OpenGL init
		if (SDL_SetVideoMode(surface->w, surface->h, 0, SDL_OPENGL) == NULL) {
			fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
			SDL_Quit();
			return EXIT_FAILURE;
		}
	}
	else
	{//FFPLAYER
		if (SDL_SetVideoMode(pCodecCtx_g->width-10, pCodecCtx_g->height, 0, SDL_OPENGL) == NULL) {
			fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
			SDL_Quit();
			return EXIT_FAILURE;
		}
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

	if(ffplayer)
		glUniform2f(glGetUniformLocation(program, "resolution"), pCodecCtx_g->width, pCodecCtx_g->height);
	else
		glUniform2f(glGetUniformLocation(program, "resolution"), surface->w, surface->h);


	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(program, "tex"), 0);

	glActiveTexture(GL_TEXTURE1);
	glUniform1i(glGetUniformLocation(program, "lut"), 1);

	//Geometry
	glEnable(GL_VERTEX_ARRAY);
	const short vertices[] = {-1,1, 1,1, 1,-1, -1,-1};
	glVertexPointer(2, GL_SHORT, 0, vertices);

	int lutindex = 0;
	printf("LUT: %s\tsize: %d\n", input_luts[lutindex].lut_name,input_luts[lutindex].lut_size);
	SDL_Event event;
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
	//Event loop
	do {
		if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
		{
			SDL_mutexP(lock);
				loop = false;
			SDL_mutexV(lock);
			break;
		} else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
			lutindex++;
			if (lutindex >= input_luts_size) {
				lutindex = 0;
			}

			printf("LUT: %s\tsize: %d\n", input_luts[lutindex].lut_name,input_luts[lutindex].lut_size);
		}

		if (event.type == SDL_USEREVENT)
			get_texture(surface);

		lut(input_luts[lutindex]);

		GLint lsize_loc = glGetUniformLocation(program,"lut_size");
		glUniform1i(lsize_loc,input_luts[lutindex].lut_size);


		glActiveTexture(GL_TEXTURE1);
		glDrawArrays(GL_QUADS, 0, 8);
		SDL_GL_SwapBuffers();
	} while (SDL_WaitEvent(&event));


	if(ffplayer)
		end_video_thread();

	if(surface != NULL)
		SDL_FreeSurface(surface);

	SDL_Quit();
}
