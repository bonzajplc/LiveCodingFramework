/*******************************************************************************
*   2005-2014, plastic demoscene group
*	authors: misz, bonzaj
*******************************************************************************/

#pragma once

unsigned int getTimeMS();

int loadFile( const char* filename, char** dstBuf, size_t* dstBufSize );
GLuint readTexture( const char* filename );
