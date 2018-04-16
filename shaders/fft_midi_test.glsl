///////////////////////////////////////////////////////////////////////////////
// shader inputs/outputs
///////////////////////////////////////////////////////////////////////////////
uniform float iGlobalTime; // in seconds
uniform vec2 iResolution; // viewport resolution (in pixels) (1080p or 720p)
uniform mat4 iMidiPad; // 16 buttons of midi controller
uniform float iMidiPadValue; // sum of all elements in iMidiPad/16

uniform float iFFT[8];
uniform float iFFTs[8];

// all samplers have linear filtering applied, wraping set to repeat
//
uniform sampler1D iFFTTexture; // 1024
// predefined textures
//
uniform sampler2D iSplashScreen;
uniform sampler2D iNoise; // 512x512 noise

// out_color must be written in order to see anything
//
layout(location = 0) out vec4 out_color;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void main(void)
{
	float x = ( gl_FragCoord.x + 0.5 ) / iResolution.x;
	//float fft = sqrt( texture( iFFTTexture, x ).x ) * 1000;
	int xb = int( gl_FragCoord.x / 80 ) % 8;
	float fftb = iFFTs[xb] * 1000;

	float t = 1;
	if ( fftb < gl_FragCoord.y )
		t = 0;

	if ( x < 0.5 )
	{
		out_color = float4( t, t, t, 1 );
	}
	else
	{
		out_color = float4( iMidiPad[0][0], iMidiPad[0][1], iMidiPad[0][2], 1 );
	}
}
