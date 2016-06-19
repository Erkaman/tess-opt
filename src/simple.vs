layout(location = 0) in vec3 vsPos;
layout(location = 1) in vec3 vsNormal;

out vec3 fsPos;
out vec3 fsNormal;
out vec3 fsResult;

uniform mat4 uMvp;
uniform mat4 uView;
uniform int uDoVertexCalculation;
uniform int uRenderSpecular;
uniform int uNoiseOctaves;
uniform float uNoiseScale;
uniform float uNoisePersistence;

void main()
{
    fsPos = vsPos;
    fsNormal = vsNormal;

    if(uDoVertexCalculation==1) {
	if(uRenderSpecular == 1)
	    fsResult = doSpecularLight(vsNormal, vsPos, uView);
	else
	    fsResult = sampleTexture(vsPos, uNoiseScale,
	uNoiseOctaves, uNoisePersistence);
    }


    gl_Position = uMvp * vec4(vsPos, 1.0);
}
