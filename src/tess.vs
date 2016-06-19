layout(location = 0) in vec3 vsPos;
layout(location = 1) in vec3 vsNormal;

out vec3 tcsPos;
out vec3 tcsNormal;

void main(){
	tcsPos = vsPos;
	tcsNormal = vsNormal;
}
