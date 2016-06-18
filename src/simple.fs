out vec3 color;

in vec3 fsPos;
in vec3 fsNormal;

uniform mat4 uView;

void main()
{
//    vec3 diff = vec3( abs(fsNormal) );

//    color = vec4(diff, 1.0);

    vec3 pos = fsPos;
    vec3 normal = fsNormal;

    color = doSpecularLight(normal, pos, uView);
}
