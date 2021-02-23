#version 440
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos = vec3(0.0f, 0.0f, 1.0f);
uniform samplerCube skybox;

uniform mat4 model, proj, view;

void main()
{             
    float ratio = 1.00 / 1.52;
    vec3 I = normalize(Position - cameraPos);
    vec3 R = refract(-I, normalize(Normal), ratio);
	vec3 F = -R * inverse(mat3(view));
    FragColor = vec4(texture(skybox, F).rgb, 1.0);
}  