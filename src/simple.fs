out vec3 color;

in vec3 fsPos;
in vec3 fsNormal;
in vec3 fsResult;

uniform mat4 uView;
uniform int uDrawWireframe;
uniform int uRenderSpecular;
uniform int uDoVertexCalculation;


void main()
{
    if(uDoVertexCalculation==1) {
	color = fsResult;

    } else {
	if(uRenderSpecular == 1)
	    color = doSpecularLight(fsNormal, fsPos, uView);
	else
	    color = sampleTexture(fsPos);

    }

    if(uDrawWireframe==1) {
	color = vec3(1.0);
    }
}
