#version 440
in vec3 vertex_position;
in vec3 vertex_normals;

out vec3 Normal;
out vec3 Position;

uniform mat4 model, proj, view;

void main()
{
	Normal = normalize(vec3(model * vec4(vertex_normals,1.0)));
    Position = vec3(model * vec4(vertex_position, 1.0));
    gl_Position = proj * view * vec4(Position, 1.0);
}