#version 120


uniform sampler2D tex;
varying vec2 coord;

void main() {
	//gl_FragColor = vec4(0.5, 0.5, 0.8, 1);

	gl_FragColor = texture2D(tex, coord);
}
