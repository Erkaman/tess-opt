out vec3 color;
in vec3 fsColor;

uniform int uDrawWireframe;

void main(){

    color = fsColor;

    if(uDrawWireframe==1) {
	color = vec3(1.0);
    }
}
