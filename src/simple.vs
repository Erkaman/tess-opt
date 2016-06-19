layout(location = 0) in vec3 vsPos;
layout(location = 1) in vec3 vsNormal;

uniform mat4 uMvp;
uniform mat4 uView;

out vec3 fsPos;
out vec3 fsNormal;
out vec3 fsResult;

uniform int uDoVertexCalculation;
uniform int uRenderSpecular;

void main()
{
    fsPos = vsPos;
    fsNormal = vsNormal;

    if(uDoVertexCalculation==1) {
	if(uRenderSpecular == 1)
	    fsResult = doSpecularLight(vsNormal, vsPos, uView);
	else
	    fsResult = sampleTexture(vsPos);
    }


    gl_Position = uMvp * vec4(vsPos, 1.0);
}
