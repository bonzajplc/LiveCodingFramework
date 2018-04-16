///////////////////////////////////////////////////////////////////////////////
// shader inputs/outputs
///////////////////////////////////////////////////////////////////////////////
uniform float iGlobalTime; // in seconds
uniform vec2 iResolution; // viewport resolution (in pixels) (1080p or 720p)
uniform mat4 iMidiPad; // 16 buttons of midi controller
uniform float iMidiPadValue; // sum of all elements in iMidiPad/16

// all samplers have linear filtering applied, wraping set to repeat
//
uniform sampler1D iFFTTexture; // 1024
// copies of following textures for preview are located on desktop
//
uniform sampler2D iSplashScreen;
uniform sampler2D iChecker;
uniform sampler2D iTex1;
uniform sampler2D iTex2;
uniform sampler2D iTex3;
uniform sampler2D iTex4;

// out_color must be written in order to see anything
//
layout(location = 0) out vec4 out_color;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void main(void)
{
	vec2 uv = float2(gl_FragCoord.xy+float2(0.5,0.5)) / float2(iResolution);
	//vec4 t = texture( iSplashScreen, uv );
	//out_color = t;

	//out_color = float4( iMidiPad[0][0], iMidiPad[0][1], iMidiPad[0][2], 1 );
	out_color = float4( iMidiPadValue, iMidiPadValue, iMidiPadValue, 1 );

	float fft = sqrt(texture( iFFTTexture, uv.x ).x) * 1000;
	float t = 1;
	if ( fft < gl_FragCoord.y )
		t = 0;

	out_color *= float4( t, t, t, 1 );

	//const float BANDS = 20;
	//float x = gl_FragCoord.x / (iResolution.x/BANDS);
	////x /= BANDS;
	//int b0 = int( pow( 2, max(x-1, 0) * 10.0/(BANDS-1) ) );
	//int b1 = int( pow( 2, x * 10.0/(BANDS-1) ) );
	//b1 = min( b1, 1024 );
	////int b0 = 0;
	//b1 = max( b1, b0 + 1 );

	//float peak = 0;
	//for ( ; b0 < b1; ++b0 )
	//{
	//	float t = texture( iFFTTexture, b0 * (1.0/1024.0) ).x;
	//	peak = max( peak, t );
	//}

	//float t = 1;
	//if ( peak*1000 < gl_FragCoord.y )
	//	t = 0;
	//out_color = float4( t, t, t, 1 );

	//out_color = float4( x, x, x, 1 );

	//out_color = float4( iMidiPad[0][0], iMidiPad[0][1], iMidiPad[0][2], 1 );
}
