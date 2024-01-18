#version 450

#define PI 3.1415926535897932384626433832795
#define MAX_TEXTURES 4		// save some space in the push constants by hard-wiring this

layout(location = 0) out vec4 color;

// interpolated position and direction of ray in world space
layout(location = 0) in vec3 p; // position
layout(location = 1) in vec3 d; // direction of ray?

// For each ray see where there is an intersection and color that

//push constants block
layout( push_constant ) uniform constants
{
	mat4 invView; // camera-to-world
	vec4 proj; // (near, far, aspect, fov)
	float time; 
} pc;

struct Sphere {
    vec3 center;
    float radius;
    vec3 dir;
    float prod; // compute for the shift here too by doing p-c
    float normp; // do p-c where c is the where you want the object to be in
    float discriminant;
    vec3 normal;
    float t;
};

layout(binding = 0) uniform sampler2D textures[ MAX_TEXTURES ];

// Material properties
vec3 bg_color = vec3(0.00,0.00,0.05);

// formulas in slides

Sphere intersect (Sphere s, int textureIndex, int factor);
mat4 rotateY(float angle);
mat4 rotateX(float angle);
mat4 rotateZ(float angle);
vec3 earthMoonLighting(vec4 textureCoordinates, vec3 normal, vec3 intersectionPoint, Sphere[3] scene);
void draw(Sphere s, int textureIndex, int factor, Sphere[3] scene);
int castShadow(vec3 intersectionPoint, Sphere[3] scene);
float sphereIntersection(Sphere s, vec3 rayOrigin, vec3 rayDirection);
mat4 rotateWithAxialTilt(float tiltAngle);

void main() {

    Sphere skyBox;
    skyBox.center = vec3(0, 0, 0);
    skyBox.radius = 3;
    skyBox.normal = normalize(d);
    float phi = acos(skyBox.normal.z); // normal.z/r incorporates the radius
    float theta = atan(skyBox.normal.x, skyBox.normal.y); 
    color = texture(textures[0],
	            vec2(skyBox.radius + 0.5 * theta/PI, phi/PI )); 

    Sphere sun;
    sun.center = vec3(0, 0, 0);
    sun.radius = 1;
    Sphere earth;
    earth.center = vec3(4, 0, 0);
    earth.radius = 0.23;
    Sphere moon;
    moon.center = vec3(1, 0, 0);
    moon.radius = 0.026;

    // Orbital rotation of Earth around the Sun
    float earthOrbitalRotationTime = pc.time / 365.0; // Adjust the division factor for the desired speed
    earth.center = vec3(rotateY(earthOrbitalRotationTime) * vec4(earth.center, 1.0));

    // Orbital rotation of Moon around the Earth
    float moonOrbitalRotationTime = pc.time / 27.0; // Adjust the division factor for the desired speed
    // Moon's orbital tilt
    mat4 moonOrbitalTilt = rotateZ(radians(5.14));
    moon.center = vec3(rotateY(moonOrbitalRotationTime) * moonOrbitalTilt) + earth.center;

    // Get t values to check intersetions
    sun = intersect(sun, 1, 27);
    earth = intersect(earth, 2, 1);
    moon = intersect(moon, 3, 27);

    Sphere[3] scene = {sun, earth, moon};

    // draw celestial body with lowest t value
    if (sun.t < earth.t && sun.t < moon.t)
    {
        draw(sun, 1, 27, scene);
    }
    else if (earth.t < sun.t && earth.t < moon.t)
    {
        draw(earth, 2, 12, scene);
    }
    else if (moon.t < earth.t && moon.t < sun.t)
    {
        draw(moon, 3, 27, scene);
    }
  
}

// Check if the camera view intersects with any sphere
Sphere intersect(Sphere s, int textureIndex, int factor)
{
    float t = 10000;
    // intersect against sphere of radius 1 centered at the origin
    s.dir = normalize(d);

    s.prod = 2.0 * dot(p - s.center, s.dir); // compute for the shift here too by doing p-c
    s.normp = length(p - s.center); // do p-c where c is the where you want the object to be in

    s.discriminant = s.prod * s.prod - 4.0 * (-s.radius + s.normp * s.normp); // tells us whether there is an intersection or not
    // the -1 in the discriminant is the radius so use it to incorporate different radius sized sphere
    // the c in the formula shifts the object to a specific location
    // antyhing below this overrirdes the backgorund and puts whatever we want on the spehres
    if( s.discriminant >= 0.0) 
    {
        // determine intersection point
        float t1 = 0.5 * (-s.prod - sqrt(s.discriminant));
        float t2 = 0.5 * (-s.prod + sqrt(s.discriminant));
        float tmin, tmax;
        if(t1 < t2) {
            tmin = t1;
            tmax = t2;
        } else {
            tmin = t2;
            tmax = t1;
        }
        if(tmax > 0.0) {
            s.t = (tmin > 0) ? tmin : tmax; // tells us the exact intersection point?
        } 
        else
        {
            s.t = t;
        }
    }
    return s;
}

void draw(Sphere s, int textureIndex, int factor, Sphere[3] scene)
{
    vec3 ipoint = p + s.t * (s.dir);
    // Axial rotation of Sphere by multiplying the normal with rotation matrix and axial tilt rotation matrix
    mat4 axialTilt = mat4(1.0);
    if(textureIndex == 2)
    {
        axialTilt = rotateWithAxialTilt(radians(23.44));
    } 
    if(textureIndex == 3)
    {
        axialTilt = rotateWithAxialTilt(radians(6.68));
    } 
    s.normal = normalize(ipoint - s.center) * mat3(rotateY(pc.time/factor) * axialTilt); // BONUS: use the normal to do the axial tilt calculations
    // BONUS: For planar tilt add an extra tilt to...?
    // determine texture coordinates in spherical coordinates
    
    vec3 originalNormal = normalize(ipoint - s.center);

    // First rotate about x through 90 degrees so that y is up.
    s.normal.z = -s.normal.z;
    s.normal = s.normal.xzy;

    float phi = acos(s.normal.z); // normal.z/r incorporates the radius
    float theta;
    if(abs(s.normal.x) < 0.001) {
        theta = sign(s.normal.y)*PI*0.5; 
    } else {
        theta = atan(s.normal.y, s.normal.x); 
    }
    
    vec2 texCoords = vec2(s.radius + 0.5 * theta/PI, phi/PI );

    // Top-left of texture is (0,0) in Vulkan, so we can stick to spherical coordinates
    color = texture(textures[textureIndex], texCoords);
    if(textureIndex == 2 || textureIndex == 3)
    {
        // adds phong shading to earth and moon
        vec3 result = earthMoonLighting(texture(textures[textureIndex], texCoords), originalNormal, ipoint, scene);
        color = vec4(result, 1.0);
    } 

}

// Phong shading implementation with ambient, diffuse and specular
vec3 earthMoonLighting(vec4 texture, vec3 normal, vec3 intersectionPoint, Sphere[3] scene)
{
    // Lighting calculations
    vec3 lightPos = vec3(0.0, 0.0, 0.0);  
    vec3 lightColor = vec3(1.0, 1.0, 1.0); 

    // Implement Ambient Lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * texture.xyz;

    // Shadow calculations
    vec3 diffuse;
    vec3 specular;
    vec3 lightDir = normalize(lightPos - intersectionPoint);

    if(castShadow(intersectionPoint, scene) == 1)
    {
        diffuse = vec3(0, 0, 0);
        specular = vec3(0, 0, 0);
    }
    else
    {
        // Implement Diffuse Lighting
        vec3 norm = normalize(normal);
        float diff = max(dot(norm, lightDir), 0.0);
        diffuse = diff * texture.xyz;

        // Implement Specular Lighting
        float specularStrength = 4.0;
        float shininess = 32.0;  // Adjust shininess based on your needs
        vec3 viewDir = normalize(p - intersectionPoint);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        specular = specularStrength * spec * texture.xyz;
    }

    vec3 result = (ambient + diffuse + specular) * lightColor;

    return result;
}


// Take the ray from the earth and shoot it at the sun. If that ray intersects with any ther sphere then its a shawdow
// If it doesnt intersect with any other sphere then it should be lit up
int castShadow(vec3 intersectionPoint, Sphere[3] scene)
{
    vec3 lightPos = vec3(0.0, 0.0, 0.0);
    vec3 rayDirection = (lightPos - intersectionPoint);

    // Use a small epsilon to prevent self-shadowing
    float epsilon = 0.01;

    // Move the shadow ray's starting point slightly away from the surface to prevent self-shadowing
    vec3 shadowStartPoint = intersectionPoint + rayDirection * epsilon;

    float earthIntersectionPoint = sphereIntersection(scene[1], shadowStartPoint, rayDirection);

    if (earthIntersectionPoint > 0 && earthIntersectionPoint < 1.0)
    {
        return 1; // There is an intersection with Earth along the shadow ray
    }

    float moonIntersectionPoint = sphereIntersection(scene[2], shadowStartPoint, rayDirection);

    if (moonIntersectionPoint > 0 && moonIntersectionPoint < 1.0)
    {
        return 1; // There is an intersection with Moon along the shadow ray
    }

    return 0; // No intersection with any sphere along the shadow ray
}

// Rotation matrix Y axis
mat4 rotateY(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat4(
        c, 0.0, -s, 0.0,
        0.0, 1.0, 0.0, 0.0,
        s, 0.0, c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

// Rotation matrix X axis
mat4 rotateX(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, c, -s, 0.0,
        0.0, s, c, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

// Rotation matrix Z axis
mat4 rotateZ(float angle) {
    float c = cos(angle);
    float s = sin(angle);
    return mat4(
        c, -s, 0.0, 0.0,
        s, c, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

// Rotation matrix for axial tilt
mat4 rotateWithAxialTilt(float tiltAngle) {
    mat4 tiltRotation = rotateZ(tiltAngle); // Rotate around Z for axial tilt
    return tiltRotation;
}

// Take the ray from the earth and shoot it at the sun. If that ray intersects with any ther sphere then its a shawdow
// Simillar implementation as intersect for camera and spheres but using rays and light position
float sphereIntersection(Sphere s, vec3 rayOrigin, vec3 rayDirection)
{
    vec3 oc = rayOrigin - s.center;
    float a = dot(rayDirection, rayDirection);
    float b = 2.0 * dot(oc, rayDirection);
    float c = dot(oc, oc) - s.radius * s.radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
    {
        // No intersection
        return -1.0;
    }
    else
    {
        // Find the closest intersection point along the ray
        float t1 = (-b - sqrt(discriminant)) / (2.0 * a);
        float t2 = (-b + sqrt(discriminant)) / (2.0 * a);

        // Choose the closer intersection point
        float t = (t1 < t2) ? t1 : t2;

        // Check if the intersection point is in the positive direction of the ray
        if (t > 0.0)
        {
            return t;
        }
        else
        {
            // The intersection point is behind the ray
            return -1.0;
        }
    }
}
