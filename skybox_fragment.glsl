#version 440

in vec3 texcoords;
uniform samplerCube skybox;
out vec4 frag_colour;

void main() {
  frag_colour = texture(skybox, texcoords);
}