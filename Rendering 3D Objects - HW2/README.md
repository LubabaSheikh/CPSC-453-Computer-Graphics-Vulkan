# How to compile and run HW2
1. Clone the 453VulkanStarter repository
2. Run: git checkout HW2
3. Copy and paste my Main.cpp, Object.cpp, Object.h,  starter.vert, starter.frag into HW2/src folder
4. Then follow the same build procedure as the starter code. This includes creating a build folder, running cmake .. and make -j10
5. Then run the program with this command: ./HW2Starter


# Switching Between Object Files
The program first loads the teapot. No command line arguments have to be passed. To switch between the object files in the models folder, press the following keys:
- Press 1 to load airplane.obj
- Press 2 to load diamond.obj
- Press 3 to load ducky.obj
- Press 4 to load hello.obj
- Press 5 to load mug.obj
- Press 6 to load Nefertiti.obj


# Model Transofrmations
Scaling: To make the object bigger press B key. To make the object smaller press S key.

Intrinsic Rotations: By default it does intrinsic rotations first. For Intrinsic rotations press I. 
- To rotate about the x-axis press the left and right arrow keys.
- To rotate about the y-axis, press the up and down arrow keys
- To rotate about the z-axis, press the A and D keys.

Extrinsic Rotations: For extrinsic rotations press E.
- To rotate about the x-axis press the left and right arrow keys.
- To rotate about the y-axis, press the up and down arrow keys
- To rotate about the z-axis, press the A and D keys.




# Lighting
To implement lighting, I calculated the ambient, diffuse and specular values in the fragment shader (starter.frag). My objectColor = vec3(0.8, 0.6, 1.0) and lightColor = vec3(1.0, 1.0, 1.0). My lightPosition = glm::vec3(20.0, 20.0, 5.0). I calculated the ambient, diffuse and specular values using the procedure stated in this website: https://learnopengl.com/Lighting/Basic-Lighting


# Explaining Code and Calculations
When a model is loaded into the positions and indices vectors, I run my preProcessingObjects() function that centers the vertices and scales them to fit the screen. I first find the max and min vertices in the x, y, and z directions to calculate the center of the object. I translate the objects in the opposite direction of the center of the object to bring the vertices back to the center. Then in-order to scale the object to fit the screen I use the max depth of the object to calculate the scaling factor to fit the object between the range -1 and 1.

I use the calculateNormalsAndNormalize() function that I created to calculate the vertex normals and normalize them for Phong shading. The vertex normals are used to interpolate normals across the surface of the triangles. For each pixel on the triangle, the interpolated normal is used for lighting calculations and smooth phong shading using ambient, diffuse and specular components. The normals are also normalized to ensure they are unit vectors which provides accurate results for lighting calculations in Phong shading.

