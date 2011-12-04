#version 120

uniform vec2 resolution;
uniform sampler2D tex;
uniform sampler2D lut;	//TODO

void main() {
	vec2 q = gl_FragCoord.xy / resolution;
	vec3 col = texture2D(tex, vec2(q.x, 1.0 - q.y)).xyz;
	gl_FragColor = vec4(col, 1);
}
