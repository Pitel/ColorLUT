#version 120


varying vec2 coord;

void main()
{
    gl_Position = ftransform();
    coord = vec2(gl_MultiTexCoord0);
}
