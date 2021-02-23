#version 440
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos = vec3(0.0f, 0.0f, 1.0f);
uniform samplerCube skybox;

uniform mat4 model, proj, view;

void main()
{             
    vec3 I = normalize(Position - cameraPos);
	vec3 N = normalize(Normal);
    vec3 R = reflect(-I, N);
	vec3 F = R * inverse(mat3(view));
	FragColor = vec4(texture(skybox, F).rgb, 1);
}