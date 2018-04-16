///////////////////////////////////////////////////////////////////////////////
// shader inputs
///////////////////////////////////////////////////////////////////////////////

// all samplers have linear filtering applied, wraping set to repeat
//

uniform float iGlobalTime; // in seconds
uniform vec2 iResolution; // viewport resolution (in pixels) (1080p or 720p)
uniform mat4 iMidiPad; // 16 buttons of midi controller
uniform float iMidiPadValue; // sum of all elements in iMidiPad/16

uniform float iFFT[8]; // latest frame
uniform float iFFTs[8]; // smoothed latest frame
uniform sampler2D iFFTsHistory; // smoothed fft history, 8x1024, x coord = bin, y coord n-frames earlier, y=0 is latest frame
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
	int xb = int( gl_FragCoord.x / 160 ) % 8;

	int xScreen = xb * 160 + 80;

	//// 1
	////
	//float fft = texelFetch( iFFTsHistory, ivec2(xb, 0), 0 ) * 1000;
	//float t = 1;
	//if ( fft < gl_FragCoord.y || abs((xb*160+80)-gl_FragCoord.x) > 40 )
	//	t = 0;

	//// 2, the same as 1
	////
	//float fft = iFFTs[xb] * 1000;
	//float t = 1;
	//if ( fft < gl_FragCoord.y || abs((xb*160+80)-gl_FragCoord.x) > 40 )
	//	t = 0;

	// 3, history
	//
	float fft = texture( iFFTsHistory, gl_FragCoord.xy / iResolution ) * 50;
	float t = 0;
	if ( abs(xScreen-gl_FragCoord.x) < fft /*&& abs(yScreen-gl_FragCoord.y) < scale*/ )
		t = 1;

	out_color = float4( t, t, t, 1 ) * texture( iNoise, gl_FragCoord.xy / iResolution );
}
