#include <stdlib.h>
#include <psp2/kernel/clib.h> 
#include <psp2/kernel/modulemgr.h>
#include <psp2/sysmodule.h>
#include <taihen.h>
#include "PSMGLES/psmgles.h"
#include "GLES2/gl2.h"
#include "EGL/egl.h"

GLfloat red = 0.0;
GLfloat blue = 1.0;

SceUID psmID, shacccgID, npID, monoID, monoBridgeID, cID;

GLuint programObject;

EGLDisplay display = NULL;
EGLSurface surface;

GLsizei width = 960;
GLsizei height = 544;


int loadModules(void)
{
    int ret;
    memset(&ret, 0, 4);
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);
    sceSysmoduleLoadModule(SCE_SYSMODULE_NP);
    sceSysmoduleLoadModule(SCE_SYSMODULE_SSL);
    sceSysmoduleLoadModule(SCE_SYSMODULE_LOCATION);
    sceSysmoduleLoadModule(SCE_SYSMODULE_SHUTTER_SOUND);

    int statusPSM = 0;
    psmID = sceKernelLoadStartModule("app0:modules/libpsm.suprx", 0, SCE_NULL, 0, SCE_NULL, &statusPSM);
    printf("libpsm() 0x%08x\n", psmID);
    if (psmID < 0)
        return 1;
    int statusMonoBridge = 0;
    monoBridgeID = sceKernelLoadStartModule("app0:modules/libmono_bridge.suprx", 0, SCE_NULL, 0, SCE_NULL, &statusMonoBridge);
    printf("libmono_bridge() 0x%08x\n", monoBridgeID);
    if (monoBridgeID < 0)
        return 1;
    int statusshaq = 0;
    shacccgID = sceKernelLoadStartModule("app0:modules/libshacccg.suprx", 0, SCE_NULL, 0, SCE_NULL, &statusshaq);
    printf("libshacccg() 0x%08x\n", shacccgID);
    if (shacccgID < 0)
        return 1;
    sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_NP);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_SSL);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_LOCATION);
    sceSysmoduleUnloadModule(SCE_SYSMODULE_SHUTTER_SOUND);
    return 0;
}

GLuint LoadShader(const GLchar *shaderSrc, GLenum type)
{
    GLuint shader;
    GLint compiled;

    printf("Creating Shader...\n");

    shader = glCreateShader(type);

    if(shader == 0)
    {
        printf("Failed to Create Shader\n");
        return 0;
    }

    glShaderSource(shader, 1, &shaderSrc, NULL);

    printf("Compiling Shader...\n");
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if(!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char* infoLog = malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            printf("Error compiling shader:\n%s\n", infoLog);
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    printf("Shader Compiled!\n");
    return shader;
}

int Init()
{
    const GLchar vShaderStr[] =
        "void main(\n"
	    "float4 aPosition,\n"
	    "float4 out gl_Position: POSITION)\n"
	    "{\n"
	    "	gl_Position = aPosition;\n"
	    "}";

    const GLchar fShaderStr[] =
        "void main(\n"
        "uniform float4 aColor,\n"
        "float4 out gl_FragColor: COLOR)\n"
	    "{\n"
	    "	gl_FragColor = aColor;\n"
	    "}";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;  
    
    vertexShader = LoadShader(vShaderStr, GL_VERTEX_SHADER);
    fragmentShader = LoadShader(fShaderStr, GL_FRAGMENT_SHADER);
    printf("Creating program\n");
    programObject = glCreateProgram();

    if(programObject == 0)
        return 0;
    printf("Created program\n");

    printf("Attaching Shaders...\n");
    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);
    printf("Attached Shaders\n");

    printf("Binding Attrib Location...\n");
    glBindAttribLocation(programObject, 0, "aPosition");
    printf("Binded Attrib Location\n");

    printf("Linking Program...\n");
    glLinkProgram(programObject);
    printf("Linked Program\n");

    printf("Getting Program...\n");
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if(!linked)
    {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if(infoLen > 1)
        {
            char* infoLog = malloc(sizeof(char) * infoLen);

            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            printf("Error linking program:\n%s\n", infoLog);

            free(infoLog);
        }

        glDeleteProgram(programObject);
        return 0;
    }
    printf("Got Program\n");

    printf("Setting Clear Color...\n");
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    printf("Set Clear Color\n");
    return 1;
}

void Draw()
{
    GLfloat vVertices[] = {0.0f,  0.5f, 0.0f,
                            -0.5f, -0.5f, 0.0f,
                            0.5f, -0.5f,  0.0f};

    glViewport(0, 0, width, height);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(programObject);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
    glEnableVertexAttribArray(0);

    if (red == 0.0)
        red = 1.0;
    else
        red = 0.0;
    if (blue == 0.0)
        blue = 1.0;
    else
        blue = 0.0;

    glUniform4f( glGetUniformLocation( programObject, "aColor" ), red, 0.0, blue, 1.0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    eglSwapBuffers(display, surface);
}

int main(void)
{

    pglInit();
    printf("Initialization Status %d\n", pglIsInit());
    display = eglGetDisplay(0);
    printf("Got Display %p\n", display);
    EGLint majorVersion;
    EGLint minorVersion;
    EGLBoolean initialized = eglInitialize(display, &majorVersion, &minorVersion);
    if (!initialized) {
        printf("Failed to initialize EGL\n");
        sceKernelExitProcess(0);
        return -1;
    }
    EGLBoolean api = eglBindAPI(0x30a0);
    if (!api) {
        printf("Failed to bind API\n");
        sceKernelExitProcess(0);
        return -1;
    }
    EGLint attribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_SAMPLES, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE};
    EGLint numConfigs = 0;
    EGLBoolean gotConfigs = eglGetConfigs(display, 0, 0, &numConfigs);
    if (!gotConfigs) {
        printf("Failed to get configs\n");
        sceKernelExitProcess(0);
        return -1;
    }
    printf("Configs %d\n", numConfigs);
    EGLConfig config;
    EGLBoolean choseConfig = eglChooseConfig(display, attribs, &config, 1, &numConfigs);
    if (!choseConfig || numConfigs < 1) {
        printf("Failed to choose config\n");
        sceKernelExitProcess(0);
        return -1;
    }
    printf("Configs %d\n", numConfigs);
    EGLint value;
    eglGetConfigAttrib(display, config, EGL_RED_SIZE, &value);
    EGLint redSize = value;

    surface = eglCreateWindowSurface(display, config, 1, NULL);
    printf("Red Size %d\n", redSize);
    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (!context) {
        printf("Failed to create context\n");
        sceKernelExitProcess(0);
        return -1;
    }
    EGLBoolean madeCurrent = eglMakeCurrent(display, surface, surface, context);
    if (!madeCurrent ) {
        printf("Failed to make current\n");
        sceKernelExitProcess(0);
        return -1;
    }
    if(!Init())
        return 0;
    while (1)
    {
        Draw();
        sceKernelDelayThread(500*1000);  //2 fps
    }
    sceKernelDelayThread(1*1000*1000);
    sceKernelExitProcess(0);
    return 0;
}

void _start(unsigned int args, void *argp)
{
    main();
}