out vec3 color;
in vec3 outColor;

uniform int uDrawWireframe;

void main(){

    color = outColor;

    if(uDrawWireframe==1) {
	color = vec3(1.0);
    }
}
