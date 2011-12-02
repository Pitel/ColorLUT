#version 120

//in vec2 position;

void main() {
    gl_Position = vec4(gl_Vertex.xy,0,1);
}
