#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include "NewSCI.h"
extern SDL_Window* sdlWindow;
extern SDL_Texture* sdlTexture;
extern SDL_Renderer* sdlRenderer;
extern Pixels shownBuffer;

SDL_Texture* sdlShader = NULL;

//unsigned int programId = 0;
#define MAX_SHADERS 4
unsigned int programIds[MAX_SHADERS] = { 0 };
int numShaders = 1;
char* shaderFile = NULL;
int scale = 1, offsetX = 0, offsetY = 0;

bool sdl2oh10 = false;

PFNGLCREATESHADERPROC glCreateShader;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLVALIDATEPROGRAMPROC glValidateProgram;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLUSEPROGRAMPROC glUseProgram;

const char* vertexShader = "varying vec4 v_color;"
"varying vec2 v_texCoord;"
"void main() {"
"gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
"v_color = gl_Color;"
"v_texCoord = vec2(gl_MultiTexCoord0);"
"}";

bool initGLExtensions() {
	glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");
	glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
	glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
	glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)SDL_GL_GetProcAddress("glValidateProgram");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
	glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");

	return glCreateShader && glShaderSource && glCompileShader && glGetShaderiv &&
		glGetShaderInfoLog && glDeleteShader && glAttachShader && glCreateProgram &&
		glLinkProgram && glValidateProgram && glGetProgramiv && glGetProgramInfoLog &&
		glUseProgram;
}

GLuint compileShader(const char* source, GLuint shaderType)
{
	if (source == NULL)
	{
		SDL_Log("Cannot compile shader, source is null.");
		return 0;
	}
	//SDL_Log("Compiling shader: %s", source);
	// Create ID for shader
	GLuint result = glCreateShader(shaderType);
	// Define shader text
	glShaderSource(result, 1, &source, NULL);
	// Compile shader
	glCompileShader(result);

	//Check vertex shader for errors
	GLint shaderCompiled = GL_FALSE;
	glGetShaderiv(result, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled != GL_TRUE)
	{
		SDL_Log("Error compiling shader: %d", result);
		GLint logLength;
		glGetShaderiv(result, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0)
		{
			GLchar *log = (GLchar*)malloc(logLength);
			glGetShaderInfoLog(result, logLength, &logLength, log);
			SDL_Log("%s", log);
			free(log);
		}
		glDeleteShader(result);
		result = 0;
	}
	return result;
}

GLuint compileProgram(const char* fragFile)
{
	if (fragFile == NULL || fragFile[0] == 0)
		return 0;

	GLuint programId = 0;
	GLuint vtxShaderId, fragShaderId;

	programId = glCreateProgram();

	vtxShaderId = compileShader(vertexShader, GL_VERTEX_SHADER);

	auto source = Pack::Read(fragFile);
	fragShaderId = compileShader(source, GL_FRAGMENT_SHADER);
	free(source);

	if(vtxShaderId && fragShaderId)
	{
		// Associate shader with program
		glAttachShader(programId, vtxShaderId);
		glAttachShader(programId, fragShaderId);
		glLinkProgram(programId);
		glValidateProgram(programId);

		// Check the status of the compile/link
		GLint logLen;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 0)
		{
			char* log = (char*) malloc(logLen * sizeof(char));
			// Show any errors as appropriate
			glGetProgramInfoLog(programId, logLen, &logLen, log);
			SDL_Log("Prog Info Log: %s", log);
			free(log);
		}
	}
	if(vtxShaderId) glDeleteShader(vtxShaderId);
	if(fragShaderId) glDeleteShader(fragShaderId);
	return programId;
}

extern int windowWidth, windowHeight;

void presentBackBuffer(SDL_Renderer *renderer, SDL_Window* win)
{
	GLint oldProgramId;
	SDL_Texture* source = sdlTexture;
	SDL_Texture* target = sdlShader;

	int winWidth, winHeight, scrWidth, scrHeight;

	for (int i = 0; i < numShaders; i++)
	{
		if (i == numShaders - 1)
			SDL_SetRenderTarget(renderer, NULL);
		else
			SDL_SetRenderTarget(renderer, target);
		SDL_RenderClear(renderer);

		unsigned int programId = programIds[i];

		SDL_GL_BindTexture(source, NULL, NULL);
		if (programId != 0)
		{
			glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
			glUseProgram(programId);
		}

		if (i == numShaders - 1)
		{
			SDL_GetWindowSize(sdlWindow, &winWidth, &winHeight);
			if (winWidth < screenWidth)
			{
				winWidth = screenWidth;
				SDL_SetWindowSize(sdlWindow, screenWidth, winHeight);
			}
			if (winHeight < screenHeight)
			{
				winHeight = screenHeight;
				SDL_SetWindowSize(sdlWindow, winWidth, screenHeight);
			}

			auto maxScaleX = floorf(winWidth / (float)screenWidth);
			auto maxScaleY = floorf(winHeight / (float)screenHeight);
#if _MSC_VER
			scale = (int)__min(maxScaleX, maxScaleY);
#else
			scale = (int)fmin(maxScaleX, maxScaleY);
#endif
			scrWidth = screenWidth * scale;
			scrHeight = screenHeight * scale;
			offsetX = (int)floorf((winWidth - scrWidth) * 0.5f);
			offsetY = (int)floorf((winHeight - scrHeight) * 0.5f);

			if (sdl2oh10)
			{
				glViewport(offsetX, offsetY, scrWidth, scrHeight);

				glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex2f((float)screenWidth, 0.0f);
				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.0f, (float)screenHeight);
				glTexCoord2f(1.0f, 1.0f);
				glVertex2f((float)screenWidth, (float)screenHeight);
				glEnd();
			}
			else
			{
				glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f((GLfloat)offsetX, (GLfloat)offsetY);
				glTexCoord2f(1.0f, 0.0f);
				glVertex2f((GLfloat)offsetX + scrWidth, (GLfloat)offsetY);
				glTexCoord2f(0.0f, 1.0f);
				glVertex2f((GLfloat)offsetX, (GLfloat)offsetY + scrHeight);
				glTexCoord2f(1.0f, 1.0f);
				glVertex2f((GLfloat)offsetX + scrWidth, (GLfloat)offsetY + scrHeight);
				glEnd();
			}
		}
		else
		{
			glViewport(0, 0, screenWidth, screenHeight);

			if (sdl2oh10)
			{
				glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.0f, (float)screenHeight);
				glTexCoord2f(1.0f, 0.0f);
				glVertex2f((float)screenWidth, (float)screenHeight);
				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.0f, 0.0f);
				glTexCoord2f(1.0f, 1.0f);
				glVertex2f((float)screenWidth, 0.0f);
				glEnd();
			}
			else
			{
				glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0.0f, 0.0f);
				glTexCoord2f(1.0f, 0.0f);
				glVertex2f((float)screenWidth, 0.0f);
				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0.0f, (float)screenHeight);
				glTexCoord2f(1.0f, 1.0f);
				glVertex2f((float)screenWidth, (float)screenHeight);
				glEnd();
			}
		}

		if (source == sdlTexture)
		{
			source = sdlShader;
			target = sdlTexture;
		}
		else
		{
			source = sdlTexture;
			target = sdlShader;
		}
	}

	glUseProgram(oldProgramId);

	SDL_GL_SwapWindow(win);
}

void OpenGL_Initialize()
{
	SDL_version linked;
	SDL_GetVersion(&linked);
	if (linked.major == 2 && linked.minor == 0)
	{
		if (linked.patch > 7)
			sdl2oh10 = true;
		else if (linked.patch < 4)
		{
			//https://www.youtube.com/watch?v=5FjWe31S_0g
			char msg[512];
			sprintf_s(msg, 512, "You are trying to run with an outdated version of SDL.\n\nYou have version %d.%d.%d.\nYou need version 2.0.4 or later.", linked.major, linked.minor, linked.patch);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Clunibus", msg, NULL);
			return;
		}
	}

	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

	if ((sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) == NULL)
	{
		SDL_Log("Could not create renderer: %s", SDL_GetError());
		return;
	}

	initGLExtensions();
	//programId = compileProgram(shaderFile); //("crt.fragment");
	programIds[0] = compileProgram(shaderFile);
	numShaders = 1;
}

void OpenGL_Present()
{
	SDL_SetRenderTarget(sdlRenderer, sdlTexture);
	SDL_UpdateTexture(sdlTexture, NULL, shownBuffer, screenPitch);
	presentBackBuffer(sdlRenderer, sdlWindow);
}
