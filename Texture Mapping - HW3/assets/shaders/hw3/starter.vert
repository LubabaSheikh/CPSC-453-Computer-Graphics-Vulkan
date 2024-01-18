#version 450

layout(location = 0) in vec3 position;	// local space
layout(location = 1) in vec3 inNormal;  // local space
layout(location = 2) in vec2 inTex;	// texture space
// layout(location = 3) in vec2 inAoTex;	// ambient occlusion space

// normal, view vector and tex coords. to be passed to fragment shader
layout(location=0) out vec3 N;		// view space vector
layout(location=1) out vec3 V;		// view space vector
layout(location=2) out vec2 T;		// texture space

layout(location=3) out int aoStateConfig;
layout(location=4) out int pnStateConfig;
layout(location=5) out int texStateConfig;
layout(location=6) out int mStateConfig;


//push constants block
layout( push_constant ) uniform constants
{
	mat4 model;
	mat4 view;
	mat4 proj;
    int aoState;
	int pnState;
	int texState;
    int mState;

} pushConstants;

void main() {

    mat4 mv = pushConstants.view * pushConstants.model;

    // Update normal
    N = mat3(mv) * inNormal;	// view space minus translation, assumes uniform scaling
    // N = transpose(inverse( mat3(mv) ) ) * inNormal;		// handles non-uniform scaling case
    // but also see: https://lxjk.github.io/2017/10/01/Stop-Using-Normal-Matrix.html

    // Calculate view-vector
    vec4 P = mv * vec4(position.xyz, 1);
    V = -P.xyz / P.w;		// view space

    // texture coordinates are passed through
    T = inTex;
	// Ao coordinates passed through 
    // A = inAoTex;
    // hack to simplify the depth buffer, otherwise ignored (homogenized camera space)
    gl_Position = pushConstants.proj * mv * vec4(position.xyz, 1);	

    aoStateConfig = pushConstants.aoState;
    pnStateConfig = pushConstants.pnState;
    texStateConfig = pushConstants.texState;
    mStateConfig = pushConstants.mState;
}
