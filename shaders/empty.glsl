///////////////////////////////////////////////////////////////////////////////
// shader inputs/outputs
///////////////////////////////////////////////////////////////////////////////
uniform float iGlobalTime; // in seconds
uniform vec2 iResolution; // viewport resolution (in pixels) (1080p or 720p)
uniform mat4 iMidiPad; // 16 buttons of midi controller
uniform float iMidiPadValue; // sum of all elements in iMidiPad/16

uniform float iFFT[8];
uniform float iFFTi[8];
uniform float iFFTs[8];
uniform float iFFTsi[8];


// all samplers have linear filtering applied, wraping set to repeat
//
uniform sampler1D iFFTTexture; // 1024
uniform sampler2D iNoise; // 512x512 noise
// predefined textures
//
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
	vec2 uv = vec2( gl_FragCoord.xy + vec2(iGlobalTime*10) ) / iResolution;
	vec4 t = texture( iTex1, uv );
	out_color = t;
}
