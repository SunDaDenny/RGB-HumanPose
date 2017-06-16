#version 440
precision highp float;
uniform sampler2D resultFrame;

in vec2 fTextureCoord;

out vec3 fragColor;


void main(void)
{
	const float scale = 25.0;
	
	vec3 data = texture(resultFrame, fTextureCoord).rgb;
	
	fragColor = vec3(data.rg / scale, data.b);
	
	if(data.r > 0)
	{
		fragColor.r = data.r / scale;
	}
	else
	{
		fragColor.b = -data.r / scale;
	}
	if(data.g > 0)
	{
		fragColor.g = data.g / scale;
	}
	else
	{
		fragColor.g = -data.g / scale;
	}	
	
}