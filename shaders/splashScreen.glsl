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
// predefined textures
//
uniform sampler2D iSplashScreen;

// out_color must be written in order to see anything
//
layout(location = 0) out vec4 out_color;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void main(void)
{
	vec2 uv = vec2(gl_FragCoord.xy) / iResolution;
	vec4 t = texture( iSplashScreen, uv );
	out_color = t;
}
