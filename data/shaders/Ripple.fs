#version 330

uniform float       rate;
uniform float       time;   // time in seconds
uniform sampler2D   tex;    // scene buffer

smooth  in  vec2 fs_texc;
smooth  in  vec4 fs_color;
        out vec4 out_color;

void main(void)
{
  vec2 pos  = -1.0 + 2.0 * fs_texc;//gl_FragCoord.xy / res;
  float len = length(pos * max(0.2, time * rate));
  vec2 uv   = fs_texc + (pos / len) * cos(len * 12.0 - time * 4.0) * 0.03;

  out_color = texture2D(tex, uv) * fs_color;
}
