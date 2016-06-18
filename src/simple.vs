layout(location = 0) in vec3 vsPos;
layout(location = 1) in vec3 vsNormal;

uniform mat4 uMvp;

out vec3 fsPos;
out vec3 fsNormal;

void main()
{
    fsPos = vsPos;
    fsNormal = vsNormal;

    gl_Position = uMvp * vec4(vsPos, 1.0);
}
