# How to compile and run HW3
1. Clone the 453VulkanStarter repository
2. Run: git checkout HW3
3. Copy and paste my Main.cpp, Object.cpp, Object.h, Texture.cpp, Texture.h and CMakeLists.txt in the 453VulkanStarter/HW3 folder
4. Copy and paste my starter.vert, starter.frag into 453VulkanStarter/assets/shaders/hw3 folder
5. Then follow the same build procedure as the starter code. This includes creating a build folder in 453VulkanStarter directory
6. Then cd build, and run cmake .. and make -j10
7. Then cd HW3 and run make -j10
8. Then run the program with this command: 

        ./HW3 arg1 arg2 arg3
        
    where arg1 is the path to the .obj file, arg2 is the path to the colour png file and arg3 is the path to the ambient occlusion png file

For example: 

    ./HW3 "../../assets/models/chess_bishop/bishop.obj" "../../assets/models/chess_bishop/bishop.colour.black.png" "../../assets/models/chess_bishop/bishop.ao.png"

    ./HW3 "../../assets/models/timmy_cup/timmy_cup.obj" "../../assets/models/timmy_cup/timmy_cup.colour.png" "../../assets/models/timmy_cup/timmy_cup.ao.png"


# Keyboard Controls
By default all configurations are turned off. The base color is set to the texture. 

Texture Mapping with Lighting: Press T to turn texture mapping with diffuse on and off. 

AO Mapping: Press A to turn ambient occlusion on and off

Perlin Noise (Procedural Texturing): Press P to turn Perlin Noise turn on and off
- Press U to increase the m value at runtime
- Press D to decrease the m value at runtime

Press the up, down, left, right and Z keys to do rotations

# Vulkan Setup for Textures
To implement the setup for textures in vulkan, I followed the tutorials provided in the vulkan website: 
https://vulkan-tutorial.com/Texture_mapping/Images

https://vulkan-tutorial.com/Texture_mapping/Image_view_and_sampler

https://vulkan-tutorial.com/Texture_mapping/Combined_image_sampler

https://vulkan-tutorial.com/Uniform_buffers/Descriptor_layout_and_buffer

https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets

I also used this git repository as guidance for the setup code: https://github.com/SaschaWillems/Vulkan/blob/master/examples/texture/texture.cpp


# Explaining Code and Calculations
I followed the vulkan tutorials in the websites cited above to do the setup. This consists of using the path to the obj, color and ao files to created the texture samples. First a textureImage, textureImageView and textureSampler is created for each model using the png files provided. Then a descriptorSetLayout is created for all textures. The descriptorSetLayout is used to create all the bindings. A descriptorPool is also created for each texture. Then descriptorSetLayout, textureImageView, textureSampler and descriptorPool are used to create 1 descriptorSet that will be passed to the GPU. Descriptor writes are alo used to map the bindings and locations of each texture in the fragment shader. 

The fragment shader then recieves all the texture samplers in the appropriate binding locations that is used to configure all the different settings such as ambient occlusion, texture mapping with lighting in diffuse and perlin noise. I used the texture() function in the fragment shader to convert all the x,y,z values with the texture sampler. These texture samplers are then used to multiply with the basecolor, ambient and diffuse constants to achieve the desired effects on the model.

For Perlin Noise I used a 16x16 2D integer vector grid and populated it with random values. Then I convert the grid into a texture by flattening it to a 1D vector to obtain the pixels. I used the same procedure to create the texture sampler using the vulkan tutorial steps by using descriptorSetLayout, textureImageView, textureSampler and descriptorPools. In the fragment shader I created perlin noise, turbulance and noise functions using the T(x,y) and S(u,v) formulas provided in the assignment. I used these functions to calculate the perlin noise sampler and multiplied it with the basecolor to display the perlin noise effect on the models.