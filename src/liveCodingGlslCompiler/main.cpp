/*******************************************************************************
*   2005-2014, plastic demoscene group
*	authors: misz, bonzaj
*******************************************************************************/
#include "stdafx.h"

HGLRC hglrc;
HDC hdc;
HWND hPFwnd;
HINSTANCE hInst;

bool extraLogMessages = false;

std::string gVendor;

static int _StartUpOpenGL(int args, char** argv)
{
	// fake window
	//
	PIXELFORMATDESCRIPTOR pfd = {
		sizeof (PIXELFORMATDESCRIPTOR),	1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE,
		PFD_TYPE_RGBA,
		(BYTE)32,
		0, 0, 0, 0, 0, 0, 0, 0, (BYTE)0, 0, 0, 0, 0,
		(BYTE)0, (BYTE)0,
		0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	hInst = GetModuleHandle(NULL);
	WNDCLASS wincl;
	wincl.hInstance = hInst;
	wincl.lpszClassName = "PFrmt";
	wincl.lpfnWndProc = DefWindowProc;
	wincl.style = 0;
	wincl.hIcon = NULL;
	wincl.hCursor = NULL;
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground = NULL;
	RegisterClass(&wincl);

	// Create a dummy window to get our extension entry points
	hPFwnd = CreateWindow("PFrmt", "PFormat", WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 8, 8, HWND_DESKTOP, NULL, hInst, NULL);

	hdc = GetDC(hPFwnd);
	SetPixelFormat(hdc,ChoosePixelFormat(hdc,&pfd), &pfd);
	hglrc = wglCreateContext(hdc);

	if( ! hglrc )
	{
		//free( fileMem );
		std::cerr << argv[0] << "Error while creating opengl context!" << std::endl;

		UnregisterClass( "PFrmt", hInst );
		DestroyWindow( hPFwnd );

		return 101;
	}

	wglMakeCurrent(hdc,hglrc);

	const GLubyte* renderer = glGetString( GL_RENDERER );
	const GLubyte* vendor = glGetString( GL_VENDOR );
	GLint majorVer = 1;
	glGetIntegerv( GL_MAJOR_VERSION, &majorVer );
	GLint minorVer = 0;
	glGetIntegerv( GL_MINOR_VERSION, &minorVer );

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit( -1 );
	}


	std::string rendererVendorInfo = (const char*)glGetString(GL_VENDOR);
	std::string rendererVendorInfoLowerCase = rendererVendorInfo;
	std::transform( rendererVendorInfo.begin(), rendererVendorInfo.end(), rendererVendorInfoLowerCase.begin(), std::tolower );

	if ( strstr(rendererVendorInfoLowerCase.c_str(), "nvidia") )
		gVendor = "nvidia";
	else if ( strstr(rendererVendorInfoLowerCase.c_str(), "ati") )
		gVendor = "amd";
	else if ( strstr(rendererVendorInfoLowerCase.c_str(), "amd") )
		gVendor = "amd";
	else
	{
		std::cerr << argv[0] << ": Unknown OpenGL vendor!" << std::endl;
	}

	return 0;
}

static void _ShutDownOpenGL()
{
	// restore previous context
	//
	wglMakeCurrent( NULL, NULL );

	// delete fake one
	//
	wglDeleteContext( hglrc );
	ReleaseDC( hPFwnd, hdc );

	SendMessage(hPFwnd,WM_CLOSE, 0, 0);
	UnregisterClass( "PFrmt", hInst );
}

int loadFile( const char* filename, char** dstBuf, size_t* dstBufSize )
{
	FILE* f = fopen( filename, "rb" );
	if ( !f )
	{
		fprintf( stderr, "fopen failed! (%s)", filename );
		return -1;
	}

	fseek( f, 0, SEEK_END );
	size_t sizeInBytes = ftell( f );
	fseek( f, 0, SEEK_SET );

	char* buf = new char[sizeInBytes+1];
	size_t readBytes = fread( buf, 1, sizeInBytes, f );
	(void)readBytes;
	assert( readBytes == sizeInBytes );
	int ret = fclose( f );
	(void)ret;
	assert( ret != EOF );

	buf[sizeInBytes] = NULL;

	*dstBuf = buf;
	*dstBufSize = sizeInBytes;

	return 0;
}


GLuint createGLSLShaderFromBuf( const char* filename, const char* buf, size_t bufSize, GLenum profile )
{
	std::stringstream ssLine0;
	ssLine0 <<
		"#version 430 core" << std::endl <<
		"#define float2 vec2" << std::endl <<
		"#define float3 vec3" << std::endl <<
		"#define float4 vec4" << std::endl <<
		"#define half2 vec2" << std::endl <<
		"#define half3 vec3" << std::endl <<
		"#define half4 vec4" << std::endl <<
		"#define int2 ivec2" << std::endl <<
		"#define int3 ivec3" << std::endl <<
		"#define int4 ivec4" << std::endl <<
		"#define bool4 bvec4" << std::endl <<

		"#define float4x4 mat4" << std::endl <<
		"#define float3x3 mat3" << std::endl <<

		"#define lerp mix" << std::endl <<
		"#define saturate(x) clamp(x, 0, 1)" << std::endl <<
		"#define mul(mtxLeft, vecRight) (mtxLeft * vecRight)" << std::endl

		<< buf << std::endl;
	;

	std::string line0 = ssLine0.str();

	GLuint glsh = glCreateShader( profile );
	const GLchar* lines[1] = { line0.c_str() };
	glShaderSource( glsh, 1, lines, NULL );
	glCompileShader( glsh );

	GLint result = GL_FALSE;
	glGetShaderiv( glsh, GL_COMPILE_STATUS, &result );

	if ( result == GL_FALSE )
	{
		if ( extraLogMessages )
			fprintf( stderr, "glsl shader compilation failed!\n" );

		int infoLogLength;
		glGetShaderiv( glsh, GL_INFO_LOG_LENGTH, &infoLogLength );

		if ( infoLogLength )
		{
			std::string errString;
			errString.resize( infoLogLength );
			glGetShaderInfoLog( glsh, infoLogLength, NULL, &errString[0] );
			const char* text = reinterpret_cast<const char*>( &errString[0] );

			std::stringstream src;
			src.str( errString );

			std::string line;

			int nExtraLines = 16;

			while ( getline(src, line) )
			{
				if ( gVendor == "nvidia" )
				{
					std::string::size_type errPos = line.find( ") : error" );
					if ( errPos != std::string::npos )
					{
						std::string::size_type nextBracket = errPos;
						if ( nextBracket != std::string::npos )
						{
							std::string lineNoStr = line.substr( 2, nextBracket-2 );
							int lineNo = atoi( lineNoStr.c_str() );
							lineNo -= (int)nExtraLines;

							std::string errText = line.substr( errPos + 1 );
							fprintf( stderr, "%s(%d)%s\n", filename ? filename : "unknown", lineNo, errText.c_str() );
						}
					}
				}
				else if ( gVendor == "amd" )
				{
					const char* token = "ERROR: 0:";
					std::string::size_type tokenPos = line.find( token );
					if ( tokenPos != std::string::npos )
					{
						std::string::size_type colonPos = tokenPos + strlen(token);
						std::string::size_type nextColonPos = line.find( ':', colonPos + 1 );

						if ( nextColonPos != std::string::npos )
						{
							std::string lineNoStr = line.substr( colonPos, nextColonPos-colonPos );
							int lineNo = atoi( lineNoStr.c_str() );
							lineNo -= (int)nExtraLines;

							std::string errText = line.substr( nextColonPos + 1 );
							fprintf( stderr, "%s(%d):%s\n", filename ? filename : "unknown", lineNo, errText.c_str() );
						}
					}
					else
					{
						fprintf( stderr, "%s\n", line.c_str() );
					}
				}
				else
				{
					fprintf( stderr, "%s\n", line.c_str() );
				}
			}
		}

		fprintf( stderr, "\n\n" );

		GLenum err = glGetError();
		while ( err )
		{
			err = glGetError();
		}

		return 0;
	}

	return glsh;
}

HANDLE _OpenPipe()
{
	HANDLE hPipe = CreateFile( 
		"\\\\.\\pipe\\LiveCodingShaderPipe",   // pipe name 
		/*GENERIC_READ |*/  // read and write access 
		GENERIC_WRITE, 
		0,              // no sharing 
		NULL,           // default security attributes
		OPEN_EXISTING,  // opens existing pipe 
		0,              // default attributes 
		NULL);          // no template file 

	if ( hPipe != INVALID_HANDLE_VALUE ) 
	{
		return hPipe;
	}

	return 0;
}

void _ClosePipe( HANDLE h )
{
	BOOL bres = FlushFileBuffers( h );
	(void)bres;
	assert( bres );
	CloseHandle( h );
}

void sendProgramSource( const char* sourceCode, size_t sourceCodeLen )
{
	HANDLE hPipe = _OpenPipe();
	if ( ! hPipe )
	{
		fprintf( stderr, "_OpenPipe failed! Shader compilation succeeded but new program wasn't sent for preview. liveCodingGlslCompiler and liveCodingPreview are intended to be used together.\n" );
		return;
	}

	const char* message = "srcCode";
	const size_t messageLen = strlen( message );

	size_t bufSize = messageLen + 1 + sourceCodeLen + 1;
	char* buf = new char[bufSize];
	char* tmpBuf = buf;
	memcpy( tmpBuf, message, messageLen + 1 );
	tmpBuf += messageLen + 1;
	memcpy( tmpBuf, sourceCode, sourceCodeLen + 1 );
	tmpBuf += sourceCodeLen + 1;

	DWORD cbWritten = 0;

	BOOL fSuccess = WriteFile( 
		hPipe,                  // pipe handle 
		buf,		            // message 
		(DWORD)bufSize,                // message length 
		&cbWritten,             // bytes written 
		NULL);                  // not overlapped 

	delete[] buf;

	if ( ! fSuccess) 
	{
		fprintf( stderr, "WriteFile to pipe failed. Err=%d\n", GetLastError() ); 
	}

	//Sleep( 1000 );

	_ClosePipe( hPipe );
}

void sendProgramBinary( GLuint glprog )
{
	GLint programBinaryLength = 0;
	glGetProgramiv( glprog, GL_PROGRAM_BINARY_LENGTH, &programBinaryLength );

	const char* message = "programBinary";
	const size_t messageLen = strlen( message );

	size_t bufSize = messageLen + 1 + 8 + programBinaryLength;
	u8* buf = new u8[bufSize];

	GLenum binaryFormat = 0;
	glGetProgramBinary( glprog, programBinaryLength, NULL, &binaryFormat, buf + messageLen + 1 + 8 );

	u8* tmpBuf = buf;
	memcpy( tmpBuf, message, messageLen + 1 );
	tmpBuf += messageLen + 1;
	memcpy( tmpBuf, &programBinaryLength, 4 );
	tmpBuf += 4;
	memcpy( tmpBuf, &binaryFormat, 4 );
	tmpBuf += 4;

	HANDLE hPipe = _OpenPipe();
	if ( ! hPipe )
	{
		delete[] buf;
		fprintf( stderr, "_OpenPipe failed! Shader compilation succeeded but new program wasn't sent for preview. liveCodingGlslCompiler and liveCodingPreview are intended to be used together.\n" );
		return;
	}

	DWORD cbWritten = 0;

	BOOL fSuccess = WriteFile( 
		hPipe,                  // pipe handle 
		buf,		            // message 
		(DWORD)bufSize,                // message length 
		&cbWritten,             // bytes written 
		NULL);                  // not overlapped 

	delete[] buf;

	if ( ! fSuccess) 
	{
		fprintf( stderr, "WriteFile to pipe failed. Err=%d\n", GetLastError() ); 
	}

	//Sleep( 1000 );

	_ClosePipe( hPipe );
}

int doFile( const std::string& srcFile )
{
	if ( extraLogMessages )
		fprintf( stderr, "doFile: %s\n", srcFile.c_str() );

	// create default vertex shader
	//
	const char* vp =
		"in float4 position;\n" \
		"void main()\n" \
		"{\n" \
		"	gl_Position = position;\n" \
		"}\n"
		;

	GLuint vertexShader_ = createGLSLShaderFromBuf( NULL, vp, strlen(vp), GL_VERTEX_SHADER );
	if ( ! vertexShader_ )
	{
		return -10;
	}

	char cwd[MAX_PATH];
	_getcwd( cwd, MAX_PATH );

	char* buf = NULL;
	size_t bufSize = 0;
	int ires = loadFile( srcFile.c_str(), &buf, &bufSize );
	if ( ires )
		return ires;

	GLuint fragmentShader_ = createGLSLShaderFromBuf( srcFile.c_str(), buf, bufSize, GL_FRAGMENT_SHADER );
	if ( ! fragmentShader_ )
	{
		delete[] buf;
		return -11;
	}

	GLuint program_ = glCreateProgram();
	glProgramParameteri( program_, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE );

	glAttachShader( program_, vertexShader_ );
	glAttachShader( program_, fragmentShader_ );

	glLinkProgram( program_ );

	GLint result = GL_FALSE;
	glGetProgramiv( program_, GL_LINK_STATUS, &result );

	if ( result == GL_FALSE )
	{
		// error during link
		//

		int infoLogLength = 0;
		glGetProgramiv( program_, GL_INFO_LOG_LENGTH, &infoLogLength );

		if ( infoLogLength )
		{
			std::string errString;
			errString.resize( infoLogLength );
			glGetProgramInfoLog( program_, infoLogLength, NULL, &errString[0] );
			const char* text = errString.c_str();
			fprintf( stderr, "glLinkProgram failed: Info log:\n%s", text );
		}

		GLenum err = glGetError();
		while ( err )
		{
			err = glGetError();
		}

		glDeleteProgram( program_ );
		glDeleteShader( fragmentShader_ );
		glDeleteShader( vertexShader_ );

		delete[] buf;
		return -20;
	}

	sendProgramBinary( program_ );

	glDeleteProgram( program_ );
	glDeleteShader( fragmentShader_ );
	glDeleteShader( vertexShader_ );

	return 0;
}

int main(int argc, char* argv[])
{
	if ( argc < 2 )
	{
		std::cerr << argv[0] << ": Expecting source file as first parameter." << std::endl;
		return -1;
	}

	if ( extraLogMessages )
		fprintf( stderr, "glslOfflineCompiler: %s\n", argv[1] );

	int ires = _StartUpOpenGL( argc, argv );
	if ( ires )
		return 0;

	std::string srcFile = argv[1];
	ires = doFile( srcFile );
	if ( ires )
	{
		return ires;
	}
	
	_ShutDownOpenGL();

	return 0;
}
