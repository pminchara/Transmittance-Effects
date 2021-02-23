#version 440

in vec3 vertex_position;
uniform mat4 proj, view;
out vec3 texcoords;

void main() {
  texcoords = vertex_position;
  gl_Position = proj * view * vec4(vertex_position, 1.0);
}