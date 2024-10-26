#if !defined(WIN32_OPENGL_H)
#if 0
#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013
#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_FULL_ACCELERATION_ARB               0x2027
#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9
#define WGL_RED_BITS_ARB                        0x2015
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_DEPTH_BITS_ARB                      0x2022
#define WGL_STENCIL_BITS_ARB                    0x2023
#define WGL_SAMPLES_ARB							0x2042

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

//#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB		0x0002
//#define WGL_CONTEXT_DEBUG_BIT_ARB					0x0001
//<F7>#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB	0x00000002

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext, const int *attribList);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_iv_arb(HDC hdc,
    int iPixelFormat,
    int iLayerPlane,
    UINT nAttributes,
    const int *piAttributes,
    int *piValues);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_fv_arb(HDC hdc,
    int iPixelFormat,
    int iLayerPlane,
    UINT nAttributes,
    const int *piAttributes,
    FLOAT *pfValues);

typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc,
    const int *piAttribIList,
    const FLOAT *pfAttribFList,
    UINT nMaxFormats,
    int *piFormats,
    UINT *nNumFormats);

typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
typedef const char * WINAPI wgl_get_extensions_string_ext(void);

typedef char GLchar;
typedef uintptr_t GLsizeiptr;

typedef void 	WINAPI gl_attach_shader(GLuint program, GLuint shader);
typedef void 	WINAPI gl_compile_shader(GLuint shader);
typedef GLuint	WINAPI gl_create_program(void);
typedef GLuint	WINAPI gl_create_shader(GLenum type);
typedef void	WINAPI gl_delete_program(GLuint program);
typedef void 	WINAPI gl_delete_shader(GLuint shader);
typedef void 	WINAPI gl_enable_vertex_attrib_array(GLuint index);
typedef void 	WINAPI gl_get_active_attrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
typedef void 	WINAPI gl_get_programiv(GLuint program, GLenum pname, GLint *params);
typedef void 	WINAPI gl_get_program_info_log(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void 	WINAPI gl_get_shaderiv(GLuint shader, GLenum pname, GLint *params);
typedef void 	WINAPI gl_get_shader_info_log(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLint	WINAPI gl_get_uniform_location(GLuint program, const GLchar *name);
typedef void	WINAPI gl_bind_vertex_array(GLuint array);
typedef void 	WINAPI gl_gen_vertex_arrays(GLsizei n, GLuint *arrays);
typedef void 	WINAPI gl_bind_buffer(GLenum target, GLuint buffer);
typedef void 	WINAPI gl_gen_buffers(GLsizei n, GLuint *buffers);
typedef void 	WINAPI gl_vertex_attrib_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void 	WINAPI gl_vertex_attribi_pointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void 	WINAPI gl_buffer_data(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void 	WINAPI gl_link_program(GLuint program);
typedef void 	WINAPI gl_shader_source(GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void 	WINAPI gl_use_program(GLuint program);
typedef void 	WINAPI gl_validate_program(GLuint program);
typedef void 	WINAPI gl_uniform_1ui(GLint location, GLuint v0);
typedef void 	WINAPI gl_uniform_1f(GLint location, GLfloat v0);
typedef void 	WINAPI gl_uniform_3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void 	WINAPI gl_uniform_4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void 	WINAPI gl_uniform_matrix_4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void 	WINAPI gl_uniform_matrix_3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void	WINAPI gl_get_shader_info_log(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void	WINAPI gl_active_texture(GLenum texture);
typedef void	WINAPI gl_uniform_1i(GLint location, GLint v0);
typedef void	WINAPI gl_generate_mipmap(GLenum target);
typedef void	WINAPI gl_tex_image3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void * data);
typedef void	WINAPI gl_tex_sub_image3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);

#define GL_DEBUG_CALLBACK(Name) void WINAPI Name(GLenum Source, GLenum Type, GLuint ID, GLenum Severity, GLsizei Length, const GLchar *Message, const void *UserParam)
typedef GL_DEBUG_CALLBACK(GLDEBUGPROC);
typedef void WINAPI gl_debug_message_callback(GLDEBUGPROC *callback, const void *userParam);

#define global_variable static

global_varible gl_attach_shader                 *glAttachShader;
global_varible gl_compile_shader                *glCompileShader;
global_varible gl_create_program                *glCreateProgram;
global_varible gl_create_shader                 *glCreateShader;
global_varible gl_delete_program                *glDeleteProgram;
global_varible gl_delete_shader                 *glDeleteShader;
global_varible gl_enable_vertex_attrib_array    *glEnableVertexAttribArray;
global_varible gl_get_active_attrib             *glGetActiveAttrib;
global_varible gl_get_programiv                 *glGetProgramiv;
global_varible gl_get_program_info_log          *glGetProgramInfoLog;
global_varible gl_get_shaderiv                  *glGetShaderiv;
global_varible gl_get_shader_info_log           *glGetShaderInfoLog;
global_varible gl_get_uniform_location          *glGetUniformLocation;

global_varible gl_bind_vertex_array				*glBindVertexArray;
global_varible gl_gen_vertex_arrays				*glGenVertexArrays;
global_varible gl_bind_buffer					*glBindBuffer;
global_varible gl_gen_buffers					*glGenBuffers;
global_varible gl_buffer_data					*glBufferData;

global_varible gl_vertex_attrib_pointer			*glVertexAttribPointer;
global_varible gl_vertex_attribi_pointer		*glVertexAttribIPointer;

global_varible gl_link_program					*glLinkProgram;
global_varible gl_shader_source					*glShaderSource;
global_varible gl_use_program					*glUseProgram;
global_varible gl_validate_program				*glValidateProgram;

global_varible gl_uniform_1ui					*glUniform1ui;
global_varible gl_uniform_1f					*glUniform1f;
global_varible gl_uniform_3fv					*glUniform3fv;
global_varible gl_uniform_4fv					*glUniform4fv;
global_varible gl_uniform_matrix_4fv			*glUniformMatrix4fv;
global_varible gl_uniform_matrix_3fv			*glUniformMatrix3fv;

global_variable gl_active_texture				*glActiveTexture;
global_variable gl_uniform_1i					*glUniform1i;
global_variable gl_generate_mipmap				*glGenerateMipmap;
global_variable gl_tex_image3D					*glTexImage3D;
global_variable gl_tex_sub_image3D				*glTexSubImage3D;

global_varible wgl_create_context_attribs_arb	*wglCreateContextAttribsARB;
global_varible wgl_choose_pixel_format_arb		*wglChoosePixelFormatARB;
global_varible wgl_swap_interval_ext			*wglSwapIntervalEXT;
global_varible wgl_get_extensions_string_ext	*wglGetExtensionsStringEXT;

global_variable gl_debug_message_callback *glDebugMessageCallback;

#define WIN32_OPENGL_H
#endif
#endif
