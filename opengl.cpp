#include <SDL_opengl.h>
#include <SDL_opengl_glext.h>

#include "NewSCI.h"
extern SDL_Window* sdlWindow;
extern SDL_Texture* sdlTexture;
extern SDL_Renderer* sdlRenderer;
extern Pixels shownBuffer;

unsigned int programId = 0;
char* shaderFile = NULL;

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

void presentBackBuffer(SDL_Renderer *renderer, SDL_Window* win, SDL_Texture* backBuffer, GLuint programId)
{
	GLint oldProgramId;

	SDL_SetRenderTarget(renderer, NULL);
	SDL_RenderClear(renderer);

	SDL_GL_BindTexture(backBuffer, NULL, NULL);
	if (programId != 0)
	{
		glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgramId);
		glUseProgram(programId);
	}

	GLfloat minx, miny, maxx, maxy;
	GLfloat minu, maxu, minv, maxv;

	//minx = (windowWidth - screenWidth) * 0.5f;
	//miny = (windowHeight - screenHeight) * 0.5f;
	//maxx = minx + screenWidth;
	//maxy = miny + screenHeight;
	minx = 0;
	miny = 0;
	maxx = windowWidth;
	maxy = windowHeight;

	minu = 0.0f;
	maxu = 1.0f;
	minv = 0.0f;
	maxv = 1.0f;

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(minu, minv);
	glVertex2f(minx, miny);
	glTexCoord2f(maxu, minv);
	glVertex2f(maxx, miny);
	glTexCoord2f(minu, maxv);
	glVertex2f(minx, maxy);
	glTexCoord2f(maxu, maxv);
	glVertex2f(maxx, maxy);
	glEnd();
	SDL_GL_SwapWindow(win);

	if (programId != 0)
		glUseProgram(oldProgramId);
}

void OpenGL_Initialize()
{
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");

	if ((sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE)) == NULL)
	{
		SDL_Log("Could not create renderer: %s", SDL_GetError());
		return;
	}

	initGLExtensions();
	programId = compileProgram(shaderFile); //("crt.fragment");
}

void OpenGL_Present()
{
	SDL_SetRenderTarget(sdlRenderer, sdlTexture);
	SDL_UpdateTexture(sdlTexture, NULL, shownBuffer, screenPitch);
	presentBackBuffer(sdlRenderer, sdlWindow, sdlTexture, programId);
}
