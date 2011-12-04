#version 120

uniform vec2 resolution;
uniform sampler2D tex;
uniform sampler2D lut;

vec4 lookup(vec3 coord, int size) {
	float sliceSize = 1.0 / size;
	float slicePixelSize = sliceSize / size;
	float sliceInnerSize = slicePixelSize * (size - 1);
	float zSlice0 = min(floor(coord.z * size), size - 1);
	float zSlice1 = min(zSlice0 + 1, size - 1);
	float xOffset = slicePixelSize / 2.0 + coord.x * sliceInnerSize;
	float s0 = xOffset + (zSlice0 * sliceSize);
	float s1 = xOffset + (zSlice1 * sliceSize);
	vec4 slice0Color = texture2D(lut, vec2(s0, coord.y));
	vec4 slice1Color = texture2D(lut, vec2(s1, coord.y));
	float zOffset = mod(coord.z * size, 1);
	return mix(slice0Color, slice1Color, zOffset);
}

void main() {
	vec2 q = gl_FragCoord.xy / resolution;
	vec3 col = texture2D(tex, vec2(q.x, 1.0 - q.y)).xyz;
	gl_FragColor = lookup(col, 16);
}
