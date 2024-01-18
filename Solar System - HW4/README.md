
# How to compile and run HW4
1. Clone the 453VulkanStarter repository
2. Run: git checkout HW4
3. Copy and paste my starter.frag into 453VulkanStarter/assets/shaders/ folder
5. Then follow the same build procedure as the starter code. This includes creating a build folder in 453VulkanStarter directory
6. Then cd build, and run cmake .. and make -j10
7. Then cd HW4 and run make -j10
8. Then run the program with this command: 

        ./HW4

9. Zoom out using the mouse to see the sun, earth and moon on the window (use mouse scroll button to zoom out)

# Keyboard Controls
- Press Right Arrow Key to increase pc.time value
- Press Left Arrow Key to decrease pc.time value


# Explaining Code and Calculations
I used the fragment shader that is provided which intersects rays with a unit sphere located at the origin to develop the functionalities of the assignment. I created a struct Sphere to store the skybox, sun, earth and moon and their properties such as center, radius, t values etc. Then I moved the interset code logic to a seperate function called intersect. 

I implemented the axial rotations accoring to:
- The Sun and the Moon have the same axial rotation period (in reality, around 27 days).
- The orbital rotation period of the Moon is  27 days so that the same side of the Moon is always facing the Earth.
- The Earth's orbital rotation period 365 daysand its axial rotation period is about 1 day.

I also created a draw function that checks the lowest t values and draws that pixel onto the screen. I also created an earthMoonLighting() function that implements Phong shading for the celestial bodies. It puts the light source at the center (0, 0, 0) and applies ambient, diffuse and specular lighting to the earth and the moon. I drew the sun, earth, moon within proprotionated distances between each other and applied the corresponding texture to them. I used a big sphere with a the starry texture on it to create the skybox. 

After setting the center and radius of each sphere I apply the orbital roation of the earth to the earth.center coordinates to make it orbit around the sun. Then I apply the orbital rotation of the the moon so that it rotates around the earth. I add the earth's center to the moon so that it rotates my making its center point the earth's center instead of the sun's center. I also apply the orbital tilt by multiplying the the tilt matrix with 5.14 degrees to the center of the moon's position for bonus. In the draw() function I apply a rotation matrix to the normals to exhibit the axial rotation. I also multiply the normals with the axial tilt matrix for earth and moon as part of the bonus.

For shadows I created two functions castShadow() and sphereIntersection() to implement the shadow effect for bonus. This causes the moon to cast a shadow on the earth during solar eclipse and earth casts shadow on moon during lunar eclipse. In the castShadow() function I calculate the rayDirection  and ligth position to check if the t values intersects anywhere. In the sphereIntersection() function I shoot the ray back to the ligh source and check if it intersects with any other object. I return a float values depending on if there was an intersection or not. If ther eis an opject in the way I set diffuse and specualar to 0 in order to create a shadow. 

# Bonus
- Implemented Shadow Rays
- Implemented Orbital and Axial Tilts