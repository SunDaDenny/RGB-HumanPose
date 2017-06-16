#version 440
precision highp float;
uniform sampler2D currentFrame, previousFrame;
uniform vec2 textureSize;
uniform float threshold;

in vec2 fTextureCoord;

out vec3 fragColor;

mat2 inverseMat2(mat2 M){
float a=M[0][0];
float b=M[0][1];
float c=M[1][0];
float d=M[1][1];
float den=(a*d-b*c);
if (den==0.0) return mat2(0.0, 0.0, 0.0, 0.0);
return (1.0/den)*mat2(d,-b,-c,a);
}

float J(vec2 xyarg){
return texture(previousFrame, fTextureCoord+xyarg).r;
}

float I(vec2 xyarg){
return texture(currentFrame, fTextureCoord+xyarg).r;
}

float Ix(vec2 xy, float epsilon) {
return (I(xy+vec2(epsilon, 0.0))-I(xy-vec2(epsilon, 0.0)))/(2.0*epsilon);
}

float Iy(vec2 xy, float epsilon) {
return (I(xy+vec2(0.0,epsilon))-I(xy-vec2(0.0,epsilon)))/(2.0*epsilon);
}

float dIk(vec2 xy, vec2 v) {
return I(xy)-J(xy+v);
}

void main(void) {

vec3 cameraRGB1 = texture(previousFrame, fTextureCoord).rgb;
vec3 cameraRGB2 = texture(currentFrame, fTextureCoord).rgb;
   if(all(equal(cameraRGB1, vec3(0.0, 0.0, 0.0))))
   {
	   fragColor = vec3(0.0, 0.0, 0.0);
	   return;
   }
      if(all(equal(cameraRGB2, vec3(0.0, 0.0, 0.0))))
   {
		fragColor = vec3(0.0, 0.0, 0.0);
	   return;
   }

vec2 epsilon=2.0/textureSize; //distance between 2 pixels
mat2 G=mat2(0.0, 0.0, 0.0, 0.0);
vec2 xy,bk=vec2(0.0, 0.0);
float Ixx, Iyy;
const float RADIUS = 4.5;

//loop on a square pixel patch : 
// for (float x=-RADIUS; x<=RADIUS; x+=1.0){
	// for (float y=-RADIUS; y<=RADIUS; y+=1.0){
		// xy=vec2(x,y)/textureSize;
		// Ixx=Ix(xy, epsilon.x);
		// Iyy=Iy(xy, epsilon.y);
		// G+=mat2(Ixx*Ixx, Ixx*Iyy, Ixx*Iyy, Iyy*Iyy);
	// }
// }

// mat2 Ginv=inverseMat2(G);




vec2 vk=vec2(0.0, 0.0);
for (float k=0.0; k<3.0; k+=1.0) {
	bk=vec2(0.0, 0.0);
	for (float x=-RADIUS; x<=RADIUS; x+=1.0){
		for (float y=-RADIUS; y<=RADIUS; y+=1.0){
			xy=vec2(x,y)/textureSize;
			Ixx=Ix(xy, epsilon.x);
			Iyy=Iy(xy, epsilon.y);
			bk+=dIk(xy,vk)*vec2(Ixx,Iyy);
		}
	}
}


vec2 nuk = bk /*/ dt*/;
nuk = nuk / 5000.0;

float dt = 30.0;
nuk = nuk * dt;

//==================================
const float PI = 3.1415926;
float isDispaly;
if(length(nuk) >= 0.1)
{
	isDispaly = 1.0;
}
else
{
	isDispaly = 0.0;
}
float angle = atan(-nuk.y, nuk.x) * 180.0 / PI + 180;
float hi = floor(angle / 60.0);
float f = angle / 60.0 - hi;
float v = 1.0;
float p = 0.0;
float q = 1.0 - f;
float t = 1.0 - (1.0 - f);
vec3 RGB = vec3(0.0, 0.0, 0.0);
// RGB += vec3(v, t, p) * (1.0 - abs(sign(hi - 0.0)));
// RGB += vec3(q, v, p) * (1.0 - abs(sign(hi - 1.0)));
// RGB += vec3(p, v, t) * (1.0 - abs(sign(hi - 2.0)));
// RGB += vec3(p, q, v) * (1.0 - abs(sign(hi - 3.0)));
// RGB += vec3(t, p, v) * (1.0 - abs(sign(hi - 4.0)));
// RGB += vec3(v, p, q) * (1.0 - abs(sign(hi - 5.0)));
if(int(hi) == 3) RGB = vec3(v, t, p);
else if(int(hi) == 4) RGB = vec3(q, v, p);
else if(int(hi) == 5) RGB = vec3(p, v, t);
else if(int(hi) == 0) RGB = vec3(p, q, v);
else if(int(hi) == 1) RGB = vec3(t, p, v);
else if(int(hi) == 2) RGB = vec3(v, p, q);
//==================================

// fragColor = vec4(nuk / dt, 0.0, 1.0);
float finalScale = 10.0;

vec3 color = vec3(0.0, 0.0, 0.0);
if(nuk.x > 0.1) color.r = nuk.x * finalScale;
else if(nuk.x < -0.1) color.g = nuk.x * (-1.0) * finalScale;
if(nuk.y > 0.1) color.b = nuk.y * finalScale;
// else if(nuk.y < -0.1) color.g = nuk.y * (-1.0) * finalScale;

fragColor = color;
fragColor = RGB * isDispaly;


// fragColor = texture(currentFrame, fTextureCoord).rgb;
// float r = length(nuk) * 100.0;
// float r = length(nuk) * finalScale;
// if(r > 0.20) r * 100.0;
// else r = 0.0;
/*if(r>0.078125*3.) r = 1.;
else r = 0.;*/
// fragColor = vec3(r, r, r);

// if(fTextureCoord.x < 0.5 && fTextureCoord.y < 0.5)
	// fragColor = vec4(1.0, 0.0, 0.0, 1.0);
// else
	// fragColor = vec4(0.0, 0.0, 0.0, 1.0);

}