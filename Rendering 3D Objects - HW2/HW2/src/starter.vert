#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inNormal; 

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragPosition;  
layout(location = 2) out vec3 cameraPosition;  
layout(location = 3) out vec3 lightPos;  

//push constants block
layout( push_constant ) uniform constants
{
	mat4 model;
	mat4 view;
    mat4 proj;

    float scale;
    float angleX;
    float angleY;
    float angleZ;
    
    vec3 cameraPosition; 
    vec3 lightPos; 

} PushConstants;

void main() {
    cameraPosition = PushConstants.cameraPosition;
    lightPos = PushConstants.lightPos;

    gl_Position = PushConstants.proj * 
    PushConstants.view * 
    PushConstants.model * 
    vec4(position.x, position.y, position.z, 1);

    fragPosition = vec3(PushConstants.model * vec4(position.x, position.y, position.z, 1.0));
    fragNormal = normalize(mat3(PushConstants.model) * inNormal); 

}
