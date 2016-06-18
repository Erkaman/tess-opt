// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vsPos;
layout(location = 1) in vec3 vsNormal;

out vec3 tcsPos;
out vec3 tcsNormal;

void main(){
	tcsPos = (vec4(vsPos,1)).xyz;
	tcsNormal = (vec4(vsNormal,0)).xyz;
}
