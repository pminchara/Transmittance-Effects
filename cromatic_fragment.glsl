#version 440

uniform samplerCube skybox;
uniform float f = 0.05;
uniform float power = 2.0;
uniform float bias = 0.3;
uniform float scale = 2.0;
uniform float eta = 0.75;
uniform vec3 cameraPos = vec3(0.0f, 0.0f, 1.0f);

uniform mat4 model, proj, view;

in vec3 Normal;
in vec3 Position;

out vec4 FragColor;

void main() {
// normalize vectors
vec3 eyeV = normalize(Position - cameraPos);
vec3 n = normalize(Normal);
vec3 e = normalize(eyeV);

// compute reflection vector
vec3 trefl = reflect(-e, n);

trefl = -trefl * inverse(mat3(view));

// compute refraction vector
vec3 trefrRED = refract(-e, n, eta);
vec3 trefrGREEN = refract(-e, n, eta + 0.1);
vec3 trefrBLUE = refract(-e, n, eta + 0.2);

trefrRED = -trefrRED * inverse(mat3(view));
trefrGREEN = -trefrGREEN * inverse(mat3(view));
trefrBLUE = -trefrBLUE * inverse(mat3(view));

// access cube map texture
vec3 refl = texture(skybox, trefl).rgb;
vec3 refr;
refr.r = texture(skybox, trefrRED).r;
refr.g = texture(skybox, trefrGREEN).g;
refr.b = texture(skybox, trefrBLUE).b;

// simple version
float fresnelApprox = 1 - pow(dot(e,n), power);

// schlick approximation
float f = ((1.0 - eta) * (1.0 - eta)) /
((1.0 + eta) * (1.0 + eta));
float schlick = f + (1 - f) * pow(1 - dot(e,n), power);

// cgTut empirical approximation
float R = max(0, min(1, bias + scale * pow(1.0 - dot(n,e), power)));
FragColor = vec4(mix(refr, refl, R), 1);
}