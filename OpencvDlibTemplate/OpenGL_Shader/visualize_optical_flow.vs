#version 440
precision highp float;
layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTextureCoord;
out vec2 fTextureCoord;
void main() {
	gl_Position = vec4(vPosition, 0.0, 1.0);
	fTextureCoord = vTextureCoord;
}