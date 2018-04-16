/*******************************************************************************
*   2005-2014, plastic demoscene group
*	authors: misz, bonzaj
*******************************************************************************/

#include "stdafx.h"
#include "liveCoding.h"

#ifdef _DEBUG
#define new _DEBUG_NEW
#endif

liveCoding* g;

//u32 prevMS = 0;

// Display func for main window
void displayFunc()
{
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	g->draw();

	HDC hdc = wglGetCurrentDC();
	HWND hwnd = WindowFromDC( hdc );

	RECT wr;
	BOOL bres = GetWindowRect( hwnd, &wr );
	if ( bres )
	{
		g->windowPosX_ = wr.left;
		g->windowPosY_ = wr.top;
	}

	//u32 curMS = getTimeMS();
	//if ( curMS < prevMS )
	//	curMS = prevMS;

	//u32 deltaMS = curMS - prevMS;
	//prevMS = curMS;

	//	float fps = 1.0f / (deltaMS * 0.001f);
	//	printf( "FPS: %3.1f\n", fps );

	glutSwapBuffers();
}

void idleFunc()
{
	glutPostRedisplay();
}

void reshapeFunc(int w1,int h1) {

	if(h1 == 0)
		h1 = 1;

	printf( "reshapeFunc(%d, %d)\n", w1, h1 );

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho( 0, w1, 0, h1, 0.1f, 1 );

	glViewport(0, 0, w1, h1);

	glMatrixMode(GL_MODELVIEW);

	g->windowW_ = w1;
	g->windowH_ = h1;
}

void keyboardFunc(unsigned char /*key*/, int /*x*/, int /*y*/)
{
	//if ( key == 27 )
	//{
	//	glutLeaveMainLoop();
	//	return;
	//}

	//glutPostRedisplay();
}

void specialFunc(int key, int /*x*/, int /*y*/)
{
	if ( key == GLUT_KEY_F4 )
	{
		int mod = glutGetModifiers();
		if ( mod & GLUT_ACTIVE_ALT )
		{
			glutLeaveMainLoop();
			return;
		}
	}
	else if ( key == GLUT_KEY_F2 )
	{
		g->fullscreen_ = !g->fullscreen_;
		glutFullScreenToggle();
		//glutGameModeString("1920x1080:32@60");
		//glutEnterGameMode();
	}
}

//void popupMenuFunc( int val )
//{
//	if ( val == 1 )
//	{
//		glutFullScreenToggle();
//	}
//};

int main(int argc, char* argv[])
{
#ifdef _DEBUG
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	flag |= _CRTDBG_LEAK_CHECK_DF;
	flag |= _CRTDBG_ALLOC_MEM_DF;
	flag &= ~_CRTDBG_CHECK_ALWAYS_DF;
	_CrtSetDbgFlag(flag);
#endif

	// init GLUT and create main window
	glutInit(&argc, argv);
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS );
	glutInitDisplayMode(/*GLUT_DEPTH |*/ GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(1280,720);
	glutCreateWindow("Live Coding Preview");

	//glutCreateMenu( popupMenuFunc );
	//glutAddMenuEntry( "Toggle Fullscreen", 1 );
	//glutAttachMenu( GLUT_RIGHT_BUTTON );

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit( -1 );
	}

	const char* glRenderer = reinterpret_cast<const char*>( glGetString(GL_RENDERER) );
	const char* glVendor = reinterpret_cast<const char*>( glGetString(GL_VENDOR) );
	GLint majorVer = 1;
	glGetIntegerv( GL_MAJOR_VERSION, &majorVer );
	GLint minorVer = 0;
	glGetIntegerv( GL_MINOR_VERSION, &minorVer );

	printf( "GL_RENDERER: %s\nGL_VENDOR: %s\nGL version: %d.%d\n\n", glRenderer, glVendor, majorVer, minorVer );

	glEnable( GL_FRAMEBUFFER_SRGB );
	wglSwapIntervalEXT( 1 );

	// callbacks for main window
	glutDisplayFunc(displayFunc);
	glutReshapeFunc(reshapeFunc);
	glutKeyboardFunc(keyboardFunc);
	glutSpecialFunc(specialFunc);
	glutIdleFunc(idleFunc);

	g = new liveCoding();
	int ires = g->startUp( argc, argv );
	if ( ires )
	{
		fprintf( stderr, "liveCoding::startUp failed! Err=%d\n", ires );
		g->shutDown();
		delete g;
		return -1;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	g->shutDown();
	delete g;
	g = NULL;

	return 0;
}


int liveCoding::startUp( int argc, char* argv[] )
{
	if ( argc < 2 )
	{
		fprintf( stderr, "expecting path to data directory as a first parameter!" );
		return -30;
	}

	liveCodingDir_ = argv[1];
	if ( liveCodingDir_.back() != '\\' && liveCodingDir_.back() != '/' )
		liveCodingDir_.append( 1, '/' );

	char cwd[MAX_PATH];
	_getcwd( cwd, MAX_PATH );
	printf( "CWD: %s\n\n", cwd );

	config_init( &config_ );
	readConfig();

	{
		HDC hdc = wglGetCurrentDC();
		HWND hwnd = WindowFromDC( hdc );

		LONG style = GetWindowLong( hwnd, GWL_STYLE );

		RECT cr;
		cr.left = windowPosX_;
		cr.right = windowPosX_ + windowW_;
		cr.top = windowPosY_;
		cr.bottom = windowPosY_ + windowH_;

		BOOL bres = AdjustWindowRect( &cr, style, false );
		if ( bres )
		{
			int w = cr.right - cr.left;
			int h = cr.bottom - cr.top;
			SetWindowPos( hwnd, HWND_TOPMOST, windowPosX_, windowPosY_, w, h, 0 );

			if ( fullscreen_ )
			{
				glutFullScreenToggle();
			}
		}
	}

	// create default vertex shader
	//
	const char* vp =
		"in float4 position;\n" \
		"void main()\n" \
		"{\n" \
		"	gl_Position = position;\n" \
		"}\n"
		;

	vertexShader_ = createGLSLShaderFromBuf( vp, strlen(vp), GL_VERTEX_SHADER );
	if ( ! vertexShader_ )
	{
		return -10;
	}

	const char* fp = 
		"layout(location = 0) out float4 out_color;\n" \
		"void main()\n" \
		"{\n" \
		"	out_color = float4( 1, 0, 0, 1 );\n" \
		"}\n"
		;

	reloadShaderSource( fp, strlen(fp) );

	{
		glGenTextures( 1, &iFFTTexture_ );
		glBindTexture( GL_TEXTURE_1D, iFFTTexture_ );

		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		float fftData[FFT_SIZE];
		for ( int i = 0; i < FFT_SIZE; ++i )
			fftData[i] = 1.0f;

		glTexImage1D( GL_TEXTURE_1D, 0, GL_R32F, FFT_SIZE, 0, GL_RED, GL_FLOAT, fftData );
		glBindTexture( GL_TEXTURE_1D, 0 );
	}

	{
		glGenTextures( 1, &iFFTsHistoryTexture_ );
		glBindTexture( GL_TEXTURE_2D, iFFTsHistoryTexture_ );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		iFFTsHistoryData_ = (float*)_aligned_malloc( 4 * FFT_BINS * FFT_HISTORY_LENGTH, 64 );
		memset( iFFTsHistoryData_, 0, 4 * FFT_BINS * FFT_HISTORY_LENGTH );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, FFT_BINS, FFT_HISTORY_LENGTH, 0, GL_RED, GL_FLOAT, iFFTsHistoryData_ );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}

	setupShaderInputs();

	const size_t nTextures = textures_.size();
	for ( size_t itex = 0; itex < nTextures; ++itex )
	{
		Tex& t = textures_[itex];
		if ( ! t.filename_.empty() )
		{
			t.glTexID_ = readTexture( (liveCodingDir_ + t.filename_).c_str() );
			if ( ! t.glTexID_ )
			{
				fprintf( stderr, "readTexture(%s) failed!\n", t.filename_.c_str() );
			}
		}
	}

	startTimeMS_ = getTimeMS();

	memset( &pipeMutex_, 0, sizeof(pipeMutex_) );
	InitializeCriticalSection( &pipeMutex_ );

	int ires = startUpPipe();
	if ( ires )
	{
		fprintf( stderr, "startUpPipe failed. Err=%d", ires );
		return ires;
	}

	ires = bass_startUp();
	if ( ires )
	{
		fprintf( stderr, "bass_startUp failed. Err=%d", ires );
		//return ires;
	}
	
	if ( ! ires )
		bass_startCapture();

	// midi
	//
	{
		midiController_CME_U2MIDI* mc = new midiController_CME_U2MIDI();
		ires = mc->startUp();
		if ( ires )
		{
			delete mc;
		}
		else
			midiController_ = mc;
	}

	//if ( ! midiController_ )
	//{
	//	midiController_Akai_MPD26* mc = new midiController_Akai_MPD26();
	//	ires = mc->startUp();
	//	if ( ires )
	//	{
	//		delete mc;
	//	}
	//	else
	//		midiController_ = mc;
	//}

	return 0;
}

void liveCoding::shutDown()
{
	if ( midiController_ )
	{
		midiController_->shutDown();
		delete midiController_;
		midiController_ = NULL;
	}

	bass_stopCapture();
	bass_shutDown();

	shutDownPipe();

	delete[] freshBinary_;
	freshBinary_ = NULL;
	freshBinarySize_ = 0;
	freshBinaryFormat_ = 0;

	delete[] freshSourceCode_;
	freshSourceCode_ = NULL;
	freshSourceCodeSize_ = 0;

	_aligned_free( iFFTsHistoryData_ );
	iFFTsHistoryData_ = NULL;

	if ( iFFTsHistoryTexture_ )
	{
		glDeleteTextures( 1, &iFFTsHistoryTexture_ );
		iFFTsHistoryTexture_ = 0;
	}

	if ( iFFTTexture_ )
	{
		glDeleteTextures( 1, &iFFTTexture_ );
		iFFTTexture_ = 0;
	}

	const size_t nTextures = textures_.size();
	for ( size_t itex = 0; itex < nTextures; ++itex )
	{
		Tex& t = textures_[itex];
		if ( t.glTexID_ )
		{
			glDeleteTextures( 1, &t.glTexID_ );
		}
	}

	textures_.clear();

	writeConfig();
	config_destroy( &config_ );
}

void liveCoding::readConfig()
{
	std::string filename = liveCodingDir_ + "config.cfg";
	int ires = config_read_file( &config_, filename.c_str() );
	if ( ! ires )
	{
		fprintf( stderr, "config_read_file '%s' failed. Creating default one.\n", filename.c_str() );
	}

	setupDefaultConfig();

	config_lookup_int( &config_, "windowPosX", &windowPosX_ );
	config_lookup_int( &config_, "windowPosY", &windowPosY_ );
	config_lookup_int( &config_, "windowW", &windowW_ );
	config_lookup_int( &config_, "windowH", &windowH_ );
	config_lookup_bool( &config_, "fullscreen", &fullscreen_ );
}

void liveCoding::writeConfig()
{
	{
		// store current window pos and fullscreen state
		//
		config_setting_set_int( config_lookup( &config_, "windowPosX" ), windowPosX_ );
		config_setting_set_int( config_lookup( &config_, "windowPosY" ), windowPosY_ );
		config_setting_set_int( config_lookup( &config_, "windowW" ), windowW_ );
		config_setting_set_int( config_lookup( &config_, "windowH" ), windowH_ );
		config_setting_set_bool( config_lookup( &config_, "fullscreen" ), fullscreen_ );
	}

	std::string filename = liveCodingDir_ + "config.cfg";

	int ires = config_write_file( &config_, filename.c_str() );
	if ( ! ires )
	{
		fprintf( stderr, "config_write_file '%s' failed.\n", filename.c_str() );
	}
}

void liveCoding::setupDefaultConfig()
{
	// fill missing settings
	//
	{
		config_setting_t* s = config_lookup( &config_, "windowPosX" );
		if ( ! s )
		{
			s = config_setting_add( config_root_setting( &config_ ), "windowPosX", CONFIG_TYPE_INT );
			config_setting_set_int( s, 0 );
		}
	}
	{
		config_setting_t* s = config_lookup( &config_, "windowPosY" );
		if ( ! s )
		{
			s = config_setting_add( config_root_setting( &config_ ), "windowPosY", CONFIG_TYPE_INT );
			config_setting_set_int( s, 0 );
		}
	}
	{
		config_setting_t* s = config_lookup( &config_, "windowW" );
		if ( ! s )
		{
			s = config_setting_add( config_root_setting( &config_ ), "windowW", CONFIG_TYPE_INT );
			config_setting_set_int( s, 800 );
		}
	}
	{
		config_setting_t* s = config_lookup( &config_, "windowH" );
		if ( ! s )
		{
			s = config_setting_add( config_root_setting( &config_ ), "windowH", CONFIG_TYPE_INT );
			config_setting_set_int( s, 600 );
		}
	}
	{
		config_setting_t* s = config_lookup( &config_, "fullscreen" );
		if ( ! s )
		{
			s = config_setting_add( config_root_setting( &config_ ), "fullscreen", CONFIG_TYPE_BOOL );
			config_setting_set_bool( s, false );
		}
	}
}

void liveCoding::draw()
{
	reloadShader();

	float midiPad[midiController::ePadCount];
	memset( midiPad, 0, sizeof(midiPad) );
	if ( midiController_ )
	{
		midiController_->update();
		midiController_->getData( midiPad );
	}

	if ( program_ )
	{
		{
			GLint location = glGetUniformLocation( program_, "iMidiPad" );
			if ( location != -1 )
			{
				glProgramUniformMatrix4fv( program_, location, 1, false, midiPad );
			}
		}

		{
			GLint location = glGetUniformLocation( program_, "iMidiPadValue" );
			if ( location != -1 )
			{
				float sum = 0;
				for ( int i = 0; i < midiController::ePadCount; ++i )
					sum += midiPad[i];

				sum /= midiController::ePadCount;

				glProgramUniform1f( program_, location, sum );
			}
		}

		if ( hRecord_ )
		{
			unsigned len = 0;

			switch( FFT_SIZE*2 ) // for 256 fft, only 128 values will contain DC in our case
			{
			case 256:
				len = BASS_DATA_FFT256;
				break;
			case 512:
				len = BASS_DATA_FFT512;
				break;
			case 1024:
				len = BASS_DATA_FFT1024;
				break;
			case 2048:
				len = BASS_DATA_FFT2048;
				break;
			case 4096:
				len = BASS_DATA_FFT4096;
				break;
			case 8192:
				len = BASS_DATA_FFT8192;
				break;
			case 16384:
				len = BASS_DATA_FFT16384;
				break;
			default:
				fprintf( stderr, "BASS invalid fft window size\n" );
				break;
			}

			if ( len )
			{
				//memset( fftBuf, 0, sizeof(fftBuf) );

				const int numBytes = BASS_ChannelGetData( hRecord_, fftBassBuf_, len | BASS_DATA_FFT_REMOVEDC );
				if( numBytes == -1 )
				{
					fprintf( stderr, "BASS_ChannelGetData failed. Err=%d\n", BASS_ErrorGetCode() );
				}
				else if ( numBytes > 0 )
				{
					memset( fft_, 0, sizeof(fft_) );

					int b0=0;

					for (int x=0;x<FFT_BINS;x++)
					{
						float peak = 0;
						int b1 = (int)pow( 2, x*10.0/(FFT_BINS-1) );
						if ( b1 > FFT_SIZE-1 )
							b1 = FFT_SIZE-1;

						if ( b1 <= b0 )
							b1 = b0 + 1; // make sure it uses at least 1 FFT bin

						float sum = 0;

						for ( ; b0 < b1; b0++ )
						{
							if (peak < fftBassBuf_[1+b0])
								peak = fftBassBuf_[1+b0];
							sum += /*(1.0f / FFT_SIZE) **/ fftBassBuf_[1+b0];
						}

						fft_[x] = peak;
						//fft_[x] = sum;
					}

					const float t = 0.9f;
					for ( int i = 0; i < FFT_BINS; ++i )
					{
						ffts_[i] = ffts_[i]*t + (1 - t)*fft_[i];
						ffti_[i] += fft_[i];
						fftsi_[i] += ffts_[i];
					}

					for ( int h = FFT_HISTORY_LENGTH-1; h > 0; --h )
					{
						float* srcRow = iFFTsHistoryData_ + (h-1) * FFT_BINS;
						float* dstRow = srcRow + FFT_BINS;
						memcpy( dstRow, srcRow, 4 * FFT_BINS );
					}

					memcpy( iFFTsHistoryData_, ffts_, 4 * FFT_BINS );

					if ( iFFTTextureUnit_ >= 0 )
					{
						glActiveTexture( GL_TEXTURE0 + iFFTTextureUnit_ );
 						glBindTexture( GL_TEXTURE_1D, iFFTTexture_ );
 						glTexSubImage1D( GL_TEXTURE_1D, 0, 0, FFT_SIZE, GL_RED, GL_FLOAT, fftBassBuf_ );
					}

					if ( iFFTsHistoryTextureUnit_ >= 0 )
					{
						glActiveTexture( GL_TEXTURE0 + iFFTsHistoryTextureUnit_ );
						glBindTexture( GL_TEXTURE_2D, iFFTsHistoryTexture_ );
						glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, FFT_BINS, FFT_HISTORY_LENGTH, GL_RED, GL_FLOAT, iFFTsHistoryData_ );
					}

					GLint location;

					location = glGetUniformLocation( program_, "iFFT" );
					if ( location != -1 )
						glProgramUniform1fv( program_, location, FFT_BINS, fft_ );

					location = glGetUniformLocation( program_, "iFFTi" );
					if ( location != -1 )
						glProgramUniform1fv( program_, location, FFT_BINS, ffti_ );

					location = glGetUniformLocation( program_, "iFFTs" );
					if ( location != -1 )
						glProgramUniform1fv( program_, location, FFT_BINS, ffts_ );

					location = glGetUniformLocation( program_, "iFFTsi" );
					if ( location != -1 )
						glProgramUniform1fv( program_, location, FFT_BINS, fftsi_ );
				}
			}
		}

		{
			GLint location = glGetUniformLocation( program_, "iGlobalTime" );
			if ( location != -1 )
			{
				u32 deltaTimeMS = getTimeMS() - startTimeMS_;
				float t = deltaTimeMS * 0.001f;
				glProgramUniform1f( program_, location, t );
			}
		}

		{
			GLint location = glGetUniformLocation( program_, "iResolution" );
			if ( location != -1 )
			{
				glProgramUniform2f( program_, location, (float)windowW_, (float)windowH_ );
			}
		}

		setShaderInputs();

		glUseProgram( program_ );

		glBegin( GL_TRIANGLES );

		glVertex2f( -1, -1 );
		glVertex2f( 3, -1 );
		glVertex2f( -1, 3 );

		glEnd();
	}
}

void liveCoding::reloadShader()
{
	EnterCriticalSection( &pipeMutex_ );

	if ( ! reloadShaderRequested_ )
	{
		LeaveCriticalSection( &pipeMutex_ );
		return;
	}

	reloadShaderRequested_ = false;

	if ( freshBinary_ )
	{
		u8* buf = freshBinary_;
		size_t bufSize = freshBinarySize_;
		GLenum format = freshBinaryFormat_;
		freshBinary_ = NULL;
		freshBinarySize_ = 0;
		freshBinaryFormat_ = 0;

		LeaveCriticalSection( &pipeMutex_ );

		reloadShaderBinary( buf, bufSize, format );

		delete[] buf;
	}
	else
	{
		char* buf = NULL;
		size_t bufSize = 0;

		if ( freshSourceCode_ )
		{
			buf = freshSourceCode_;
			bufSize = freshSourceCodeSize_;
			freshSourceCode_ = NULL;
			freshSourceCodeSize_ = 0;

			LeaveCriticalSection( &pipeMutex_ );
		}
		else
		{
			LeaveCriticalSection( &pipeMutex_ );

			//int ires = loadFile( (liveCodingDir_ + "shaders/splashScreen.glsl").c_str(), &buf, &bufSize );
			int ires = loadFile( (liveCodingDir_ + "shaders/fft_test.glsl").c_str(), &buf, &bufSize );
			if ( ires )
			{
				return;
			}
		}

		reloadShaderSource( buf, bufSize );

		delete[] buf;
	}

	const size_t nTextures = textures_.size();
	int unit = 1; // use unit 0 for management/creation
	for ( size_t itex = 0; itex < nTextures; ++itex )
	{
		const Tex& t = textures_[itex];
		if ( ! t.glTexID_ )
			continue;

		GLint location = glGetUniformLocation( program_, t.samplerName_.c_str() );
		if ( location != -1 )
		{
			glProgramUniform1i( program_, location, unit );
			glActiveTexture( GL_TEXTURE0 + unit );
			glBindTexture( GL_TEXTURE_2D, t.glTexID_ );
			++ unit;
		}
	}

	{
		GLint location = glGetUniformLocation( program_, "iFFTTexture" );
		if ( location != -1 )
		{
			glProgramUniform1i( program_, location, unit );
			glActiveTexture( GL_TEXTURE0 + unit );
			glBindTexture( GL_TEXTURE_1D, iFFTTexture_ );
			iFFTTextureUnit_ = unit;
			++ unit;
		}
		else
		{
			iFFTTextureUnit_ = -1;
		}
	}

	{
		GLint location = glGetUniformLocation( program_, "iFFTsHistory" );
		if ( location != -1 )
		{
			glProgramUniform1i( program_, location, unit );
			glActiveTexture( GL_TEXTURE0 + unit );
			glBindTexture( GL_TEXTURE_2D, iFFTsHistoryTexture_ );
			iFFTsHistoryTextureUnit_ = unit;
			++ unit;
		}
		else
		{
			iFFTsHistoryTextureUnit_ = -1;
		}
	}
}

int liveCoding::reloadShaderSource( const char* buf, size_t bufSize )
{
	GLuint newFragmentShader = createGLSLShaderFromBuf( buf, bufSize, GL_FRAGMENT_SHADER );
	if ( ! newFragmentShader )
	{
		return -1;
	}

	GLuint newProgram = glCreateProgram();
	glAttachShader( newProgram, vertexShader_ );
	glAttachShader( newProgram, newFragmentShader );

	glLinkProgram( newProgram );

	GLint result = GL_FALSE;
	glGetProgramiv( newProgram, GL_LINK_STATUS, &result );

	if ( result == GL_FALSE )
	{
		// error during link
		//

		int infoLogLength = 0;
		glGetProgramiv( newProgram, GL_INFO_LOG_LENGTH, &infoLogLength );

		if ( infoLogLength )
		{
			std::string errString;
			errString.resize( infoLogLength );
			glGetProgramInfoLog( newProgram, infoLogLength, NULL, &errString[0] );
			const char* text = errString.c_str();
			fprintf( stderr, "glLinkProgram failed: Info log:\n%s\n", text );
		}

		GLenum err = glGetError();
		while ( err )
		{
			err = glGetError();
		}

		glDeleteProgram( newProgram );
		glDeleteShader( newFragmentShader );

		return -20;
	}

	if ( program_ )
	{
		glDeleteProgram( program_ );
	}

	if ( fragmentShader_ )
	{
		glDeleteShader( fragmentShader_ );
	}

	program_ = newProgram;
	fragmentShader_ = newFragmentShader;

	return 0;
}

int liveCoding::reloadShaderBinary( const u8* buf, size_t bufSize, GLenum format )
{
	GLuint newProgram = glCreateProgram();
	glProgramBinary( newProgram, format, buf, (GLsizei)bufSize );

	GLint result = GL_FALSE;
	glGetProgramiv( newProgram, GL_LINK_STATUS, &result );

	if ( result == GL_FALSE )
	{
		int infoLogLength = 0;
		glGetProgramiv( newProgram, GL_INFO_LOG_LENGTH, &infoLogLength );

		if ( infoLogLength )
		{
			std::string errString;
			errString.resize( infoLogLength );
			glGetProgramInfoLog( newProgram, infoLogLength, NULL, &errString[0] );
			const char* text = errString.c_str();
			fprintf( stderr, "glProgramBinary failed: Info log:\n%s\n", text );
		}

		GLenum err = glGetError();
		while ( err )
		{
			err = glGetError();
		}

		glDeleteProgram( newProgram );

		return -20;
	}

	if ( program_ )
	{
		glDeleteProgram( program_ );
	}

	if ( fragmentShader_ )
	{
		glDeleteShader( fragmentShader_ );
	}

	program_ = newProgram;
	fragmentShader_ = NULL;

	return 0;
}

GLuint liveCoding::createGLSLShaderFromBuf( const char* buf, size_t /*bufSize*/, GLenum profile )
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
		fprintf( stderr, "glsl shader compilation failed!\n" );

		int infoLogLength;
		glGetShaderiv( glsh, GL_INFO_LOG_LENGTH, &infoLogLength );

		if ( infoLogLength )
		{
			std::string errString;
			errString.resize( infoLogLength );
			glGetShaderInfoLog( glsh, infoLogLength, NULL, &errString[0] );
			const char* text = reinterpret_cast<const char*>( &errString[0] );

			fprintf( stderr, "glsl error:\n%s\n", text );
		}

		GLenum err = glGetError();
		while ( err )
		{
			err = glGetError();
		}

		return 0;
	}

	return glsh;
}

int liveCoding::startUpPipe()
{
	threadHandle_ = _beginthreadex( NULL, 0, &thread_funcStatic, this, 0, &threadId_ );
	if ( ! threadHandle_ )
	{
		fprintf( stderr, "Couldn't start thread for pipe!" );
		return -1;
	}

	return 0;
}

void liveCoding::shutDownPipe()
{
	if ( threadHandle_ )
	{
		{
			EnterCriticalSection( &pipeMutex_ );
			shutDownThread_ = true;
			LeaveCriticalSection( &pipeMutex_ );
		}

		WaitForSingleObject( (HANDLE)threadHandle_, INFINITE );
		CloseHandle( (HANDLE)threadHandle_ );

		threadHandle_ = NULL;
		threadId_ = 0;
	}
}

BOOL liveCoding::tryConnect()
{
	OVERLAPPED ol;
	memset( &ol, 0, sizeof(ol) );
	BOOL ret = 0;

	for ( ; ; )
	{
		{
			EnterCriticalSection( &pipeMutex_ );
			if ( shutDownThread_ )
			{
				LeaveCriticalSection( &pipeMutex_ );
				return FALSE;
			}
			LeaveCriticalSection( &pipeMutex_ );
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 

		ol.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		BOOL fConnected = ConnectNamedPipe( hPipe_, &ol );

		if ( fConnected )
		{
			printf( "Shader pipe connected\n" );
			return TRUE;
		}
		else
		{
			DWORD err = GetLastError();

			if ( err == ERROR_PIPE_CONNECTED )
			{
				CloseHandle( ol.hEvent );
				return TRUE;
			}
			else if ( err == ERROR_PIPE_LISTENING )
			{
				Sleep( 100 );
			}
			else if ( err == ERROR_IO_PENDING )
			{
				if( WaitForSingleObject( ol.hEvent, 1000 ) == WAIT_OBJECT_0 )
				{
					DWORD dwIgnore;
					ret = GetOverlappedResult( hPipe_, &ol, &dwIgnore, FALSE );
					if ( ret )
					{
						CloseHandle( ol.hEvent );
						return TRUE;
					}

				} else
				{
					CancelIo( hPipe_ );
				}
			}
			else
			{
				fprintf( stderr,  "ConnectNamedPipe failed! Err=%d\n", err );
				break;
			}
		}
	}

	return FALSE;
}

void liveCoding::handlePipe()
{
	BOOL fSuccess = FALSE;

	u8* inBuffer = new u8[ e_PipeBUFSIZE ];

	for ( ; ; )
	{
		{
			EnterCriticalSection( &pipeMutex_ );
			if ( shutDownThread_ )
			{
				LeaveCriticalSection( &pipeMutex_ );
				break;
			}
			LeaveCriticalSection( &pipeMutex_ );
		}

		DWORD cbBytesRead = 0;

		memset( inBuffer, 0, e_PipeBUFSIZE );

		fSuccess = ReadFile( 
			hPipe_,       // handle to pipe 
			inBuffer,     // buffer to receive data 
			e_PipeBUFSIZE-1,  // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL);        // not overlapped I/O 

		if ( !fSuccess || cbBytesRead == 0 )
		{   
			DWORD err = GetLastError();

			if ( err == ERROR_NO_DATA )
			{
				Sleep( 100 );
				continue;
			}
			else if (err == ERROR_BROKEN_PIPE)
			{
				//fprintf( stderr, "ReadFile failed! Broken pipe\n" );
				break;
			}
			else
			{
				fprintf( stderr, "ReadFile failed!\n Err=%d", err );
				break;
			}
		}

		// process message
		//

		const char* message = reinterpret_cast<const char*>( inBuffer );
		const size_t messageLen = strlen(message);

		if ( messageLen >= strlen("srcCode") && !strcmp(message, "srcCode") )
		{
			const char* payload = message + messageLen + 1;

			//printf( "newSourceCode:\n%s\n", payload );

			size_t payLoadSize = strlen(payload);
			char* src = new char[payLoadSize+1];

			memcpy( src, payload, payLoadSize );
			src[payLoadSize] = 0;

			char* oldSourceCode = NULL;
			size_t oldSourceCodeSize = 0;

			EnterCriticalSection( &pipeMutex_ );

			reloadShaderRequested_ = true;
			oldSourceCode = freshSourceCode_;
			oldSourceCodeSize = freshSourceCodeSize_;

			freshSourceCode_ = src;
			freshSourceCodeSize_ = payLoadSize;

			LeaveCriticalSection( &pipeMutex_ );

			delete[] oldSourceCode;		
		}
		else if ( messageLen >= strlen("programBinary") && !strcmp(message, "programBinary") )
		{
			const char* payload = message + messageLen + 1;
			u32 binaryLen = 0;
			GLenum binaryFormat = 0;
			memcpy( &binaryLen, payload, 4 );
			memcpy( &binaryFormat, payload + 4, 4 );

			u8* bin = new u8[binaryLen];
			memcpy( bin, payload + 8, binaryLen );

			u8* oldBinary = NULL;

			EnterCriticalSection( &pipeMutex_ );

			reloadShaderRequested_ = true;

			oldBinary = freshBinary_;

			freshBinary_ = bin;
			freshBinarySize_ = binaryLen;
			freshBinaryFormat_ = binaryFormat;

			LeaveCriticalSection( &pipeMutex_ );

			delete[] oldBinary;
		}
	}

	printf( "  Shader pipe DISconnected\n" );

	delete[] inBuffer;

	DisconnectNamedPipe( hPipe_ );
}

unsigned int __stdcall liveCoding::thread_funcStatic( void *arg )
{
	liveCoding* nod = reinterpret_cast<liveCoding*>( arg );
	return nod->thread_func( NULL );
}

unsigned int liveCoding::thread_func( void * /*arg*/ )
{
	hPipe_ = CreateNamedPipe( 
		"\\\\.\\pipe\\LiveCodingShaderPipe",        // pipe name 
		FILE_FLAG_OVERLAPPED |
		PIPE_ACCESS_INBOUND,      // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		1,						  // max. instances  
		0,                        // output buffer size 
		e_PipeBUFSIZE,                // input buffer size 
		1000,                     // client time-out 
		NULL );                   // default security attribute 

	if ( hPipe_ == INVALID_HANDLE_VALUE )
	{
		fprintf( stderr, "Couldn't open named pipe! Err=%u\n", GetLastError() );
		return 1;
	}

	BOOL fConnected = FALSE;

	for ( ; ; )
	{
		{
			EnterCriticalSection( &pipeMutex_ );
			if ( shutDownThread_ )
			{
				LeaveCriticalSection( &pipeMutex_ );
				break;
			}
			LeaveCriticalSection( &pipeMutex_ );
		}

		fConnected = tryConnect();

		if ( fConnected )
		{
			handlePipe();
		}
	}

	if ( hPipe_ != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hPipe_ );
		hPipe_ = 0;
	}

	return 0;
}

int liveCoding::bass_startUp()
{
	int device = -1;
	config_lookup_int( &config_, "bassRecordDevice", &device );

	const int freq = 44100;

	int ires = 0;
	ires = BASS_Init( device, freq, 0, 0, 0 );
	if( !ires )
	{
		fprintf( stderr, "BASS_Init failed! Err=%d\n", BASS_ErrorGetCode() );
		return -1;
	}

	int a, count = 0;
	BASS_DEVICEINFO allInfo[16];
	memset( allInfo, 0, sizeof(allInfo) );
	BASS_DEVICEINFO info;
	for ( a = 0; BASS_RecordGetDeviceInfo(a, &info); a++ )
	{
		if ( info.flags & BASS_DEVICE_ENABLED ) // device is enabled
		{
			printf( "BASS_RecordDevice: %d : %s\n", count, info.name );

			if ( count < 16 )
				allInfo[count] = info;

			count++; // count it
		}
	}

	//printf( "n", count, info.name );

	ires = BASS_RecordInit( device );
	if( !ires )
	{
		fprintf( stderr, "BASS_RecordInit failed! Err=%d\n", BASS_ErrorGetCode() );
		return -2;
	}

	DWORD usedDevice = BASS_RecordGetDevice();
	(void)usedDevice;

	return 0;
}

void liveCoding::bass_shutDown()
{
	BASS_RecordFree();
	BASS_Free();
}

void liveCoding::bass_startCapture()
{
	bass_stopCapture();

	//const int freq = 44100;
	//const int channels = 2;

	//hRecord_ = BASS_RecordStart( freq, channels, BASS_SAMPLE_8BITS, 0, 0 );
	//if( !hRecord_ )
	//{
	//	fprintf( stderr, "BASS_RecordStart failed! Err=%d\n", BASS_ErrorGetCode() );
	//}

	//std::string file = liveCodingDir_ + "sine.wav";
	//std::string file = liveCodingDir_ + "catzilla.ogg";
	std::string file = liveCodingDir_ + "wilhelm.wav";

	if ( ! (hRecord_ = BASS_StreamCreateFile(FALSE,file.c_str(),0,0,BASS_SAMPLE_LOOP))
		&& ! ( hRecord_ = BASS_MusicLoad(FALSE,file.c_str(),0,0,BASS_MUSIC_RAMP|BASS_SAMPLE_LOOP,1)))
	{
		assert( false );
		return;
	}

	BASS_ChannelPlay(hRecord_,FALSE);
}

void liveCoding::bass_stopCapture()
{
	if ( hRecord_ )
	{
		BASS_ChannelStop( hRecord_ );
		hRecord_ = 0;
	}
}
