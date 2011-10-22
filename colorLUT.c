#include <stdbool.h>

#include <SDL.h>
#include <GL/glew.h>

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

int main() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	
	if (SDL_SetVideoMode(800, 600, 0, SDL_OPENGL) == NULL) {
		fprintf(stderr, "Unable to create OpenGL screen: %s\n", SDL_GetError());
		SDL_Quit();
		exit(2);
	}
	
	SDL_WM_SetCaption("colorLUT", NULL);
	
	glewInit();
	printf("Using GLEW Version %s\n", glewGetString(GLEW_VERSION));
	
	const char* vs_src = file2string("colorLUT.vs");
	GLuint vs = glCreateShader(GL_VERTEX_SHADER_ARB);
	glShaderSource(vs, 1, &vs_src, NULL);
	glCompileShader(vs);
	shaderlog(vs);
	
	const char* fs_src = file2string("colorLUT.fs");
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER_ARB);
	glShaderSource(fs, 1, &fs_src, NULL);
	glCompileShader(fs);
	shaderlog(fs);
	
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	shaderlog(program);
	
	glUseProgram(program);
	
	bool done = false;
	SDL_Event event;
	while (!done && SDL_WaitEvent(&event)) {
		if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE) {
			done = true;
		}
	}
	SDL_Quit();
}
