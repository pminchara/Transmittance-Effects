#version 440
uniform samplerCube skybox;
uniform float f = 0.1;
uniform float power = 2.0;
uniform float bias = 0.3;
uniform float scale = 2.0;
uniform float eta = 0.20;

uniform mat4 model, proj, view;

uniform vec3 cameraPos = vec3(0.0f, 0.0f, 1.0f);

in vec3 Normal;
in vec3 Position;

out vec4 FragColor;

void main() {
vec3 eyeV = normalize(Position - cameraPos);
// normalize vectors
vec3 n = normalize(Normal);
vec3 e = normalize(eyeV);

// compute reflection vector
vec3 trefl = reflect(-e, n);

// compute refraction vector
vec3 trefr = refract(-e, n, eta);

vec3 F1 = -trefl * inverse(mat3(view));
vec3 Fr = -trefr * inverse(mat3(view));

// access cube map texture
vec3 refl = texture(skybox, F1).rgb;
vec3 refr = texture(skybox, Fr).rgb;
// Schlick approximation
float f = ((1.0 - eta) * (1.0 - eta)) / ((1.0 + eta) * (1.0 + eta));
float schlick = f + (1 - f) * pow(1 + dot(e,n), power);
FragColor = vec4(mix(refr, refl, f), 1);
}