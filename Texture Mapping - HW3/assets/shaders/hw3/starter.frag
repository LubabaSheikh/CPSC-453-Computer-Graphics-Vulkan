#version 450

float ambient_strength = 0.1;
vec3 specular_strength = vec3(0.5, 0.5, 0.5);	// allow some diffuse through
float specular_power = 76.8;

vec3 lightDir = vec3(1.0, 2.0, 1.0);	// view space vector, will be normalized later!
vec3 lightCol = vec3(1.0, 1.0, 1.0);	// overall light colour

// data layout
layout(location = 0) out vec4 colour;

layout(location = 0) in vec3 normal;	// view space vector
layout(location = 1) in vec3 viewDir;	// view space vector
layout(location = 2) in vec2 tex;	// texture space

layout(location = 3) flat in int aoConfig;
layout(location = 4) flat in int pnConfig;
layout(location = 5) flat in int texConfig;
layout(location = 6) flat in int mConfig;

// layout(binding = 0) uniform config; // NEED THIS
layout(binding = 1) uniform sampler2D texSampler; // NEED THIS
layout(binding = 2) uniform sampler2D AOSampler; // NEED THIS
layout(binding = 3) uniform sampler2D PNSampler; // NEED THIS

// Noise function using linear interpolation
float noise(float x, float y) {
    return texture(PNSampler, vec2(x, y)).r;
}

// Turbulence function
float turbulence(float x, float y) {
    float result = 0.0f;
	// T(x, y) = Sum(i=0, 4)(Noise(2^i *x, 2^i *y))/2^i
    // Use five noise levels to define the turbulence function
    for (int i = 0; i < 5; ++i) {
        float frequency = pow(2.0f, i);
        float amplitude = 1.0f / frequency;

        // Add the scaled noise to the result
        result += noise(frequency * x, frequency * y) * amplitude;
    }

    return result;
}

// Procedural texture function
float perlinNoise(vec2 tex, float m) {
    float turbulenceValue = turbulence(tex.x, tex.y);
    float pi = 3.14159265358979323846f;
    float value = 0.5f * (1.0f + sin(m * pi * (tex.x + tex.y + turbulenceValue)));
    return value;
}

void main() {
    // Normalize N, L and V vectors
    vec3 N = normalize(normal);
    vec3 L = normalize(lightDir);
    vec3 V = normalize(viewDir);

    // Calculate R locally
    vec3 R = reflect(-L, N);
    
    vec3 basecol = texture(texSampler, tex).xyz;
    
    if(pnConfig == 1)
    {
        float proceduralValue = perlinNoise(tex, mConfig);
        basecol = vec3(proceduralValue);
    }
    
    vec3 diffuse = max(dot(N, L), 0.0) * basecol;
    vec3 ambient = ambient_strength * basecol;
    vec3 specular = pow(max(dot(R, V), 0.0), specular_power) * specular_strength;

    if(texConfig == 1)
    {
        vec3 textureSampler = texture(texSampler, tex).xyz; // NEED THIS
        diffuse = diffuse * textureSampler;
    }
    if(aoConfig == 1)
    {
        vec3 aoSampler = texture(AOSampler, tex).xyz; // NEED THIS
        ambient = ambient * aoSampler;
    }
    
    // Write final colour to the framebuffer
    colour = vec4((ambient + diffuse + specular)*lightCol, 1.0);
    
}

