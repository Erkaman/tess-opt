#pragma once

/*
  GLFW
 */
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>

#include <string>

#define _DEBUG

inline void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    //  const GLubyte* sError = gluErrorString(err);

    if (err != GL_NO_ERROR)
	{
	    printf("OpenGL error %08x, at %s:%i - for %s. Error Message\n", err, fname, line, stmt);
	    exit(1);
	}
}


// GL Check.
#ifdef _DEBUG
#define GL_C(stmt) do {					\
	stmt;						\
	CheckOpenGLError(#stmt, __FILE__, __LINE__);	\
    } while (0)
#else
#define GL_C(stmt) stmt
#endif



/*
  This utility function loads the contents of a file into a std::string.
 */
std::string LoadFile(const char* path) {

    //  Open file

    FILE* fp = fopen(path, "rb" );

    if(!fp) {
	printf("Could not open %s", path );
	exit(1);
    }

    // Get file size.

    // seek to end of file.
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    // reset file pointer
    fseek(fp, 0L, SEEK_SET);

    // Read file.

    char* buffer = new char[fileSize];
    fread(buffer, sizeof(char), (size_t)fileSize, fp);
    std::string str(buffer, fileSize);
    free(buffer);

    return str;
}


char* GetShaderLogInfo(GLuint shader) {

    GLint len;
    GLsizei actualLen;

    GL_C(glGetShaderiv(shader,  GL_INFO_LOG_LENGTH, &len));
    char* infoLog = new char[len];

    GL_C(glGetShaderInfoLog(shader, len, &actualLen, infoLog));

    return  infoLog;

}

GLuint CreateShaderFromString(const std::string& shaderSource, const GLenum shaderType) {

    // before the shader source code, we append the contents of the file "shader_common"
    // this contains important functions that are common between some shaders.
    std::string src = LoadFile("shader_common") + shaderSource;//LoadFile("shader_common")

    GLuint shader;

    GL_C(shader = glCreateShader(shaderType));
    const char *c_str = src.c_str();
    GL_C(glShaderSource(shader, 1, &c_str, NULL ));
    GL_C(glCompileShader(shader));

    GLint compileStatus;
    GL_C(glGetShaderiv(shader,  GL_COMPILE_STATUS, &compileStatus));

    if (compileStatus != GL_TRUE) {
	printf("Could not compile shader\n\n%s \n\n%s\n",  src.c_str(),
	       GetShaderLogInfo(shader) );
	exit(1);
    }

    return shader;
}

/*
  Load shader with vertex shader, fragment shader, TC-shader, TE-shader.
 */
GLuint LoadTessShader(
    const std::string& vsSource,
    const std::string& fsShader,
    const std::string& tcsSource,
    const std::string& tesSource){

    // Create the shaders
    GLuint vs = CreateShaderFromString(vsSource, GL_VERTEX_SHADER);
    GLuint fs =CreateShaderFromString(fsShader, GL_FRAGMENT_SHADER);
    GLuint tcs =CreateShaderFromString(tcsSource, GL_TESS_CONTROL_SHADER);
    GLuint tes =CreateShaderFromString(tesSource, GL_TESS_EVALUATION_SHADER);

    // Link the program
    GLuint shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glAttachShader(shader, tcs);
    glAttachShader(shader, tes);
    glLinkProgram(shader);


    // Check the program
    GLint linkStatus;
    glGetProgramiv(shader, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE) {
	printf("Could not link shader \n\n%s\n",   GetShaderLogInfo(shader)  );
	exit(1);
    }

    // clean up
    glDetachShader(shader, vs);
    glDetachShader(shader, fs);
    glDetachShader(shader, tes);
    glDetachShader(shader, tcs);

    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteShader(tes);
    glDeleteShader(tcs);

    return shader;
}

/*
  Load shader with only vertex and fragment shader.
 */
GLuint LoadNormalShader(const std::string& vsSource, const std::string& fsShader){


    // Create the shaders
    GLuint vs = CreateShaderFromString(vsSource, GL_VERTEX_SHADER);
    GLuint fs =CreateShaderFromString(fsShader, GL_FRAGMENT_SHADER);

    // Link the program
    GLuint shader = glCreateProgram();
    glAttachShader(shader, vs);
    glAttachShader(shader, fs);
    glLinkProgram(shader);



    GLint Result;
    glGetProgramiv(shader, GL_LINK_STATUS, &Result);
    if(Result == GL_FALSE) {
	printf("Could not link shader \n\n%s\n",   GetShaderLogInfo(shader)  );
	exit(1);
    }

    glDetachShader(shader, vs);
    glDetachShader(shader, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return shader;
}
