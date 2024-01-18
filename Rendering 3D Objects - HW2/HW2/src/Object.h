/*
 * Copyright 2023 University of Calgary, Visualization and Graphics Grpup
 */
#include <vulkan/vulkan.h>
#include "Camera.h"

void objectCreateGeometryAndBuffers();
void objectDestroyBuffers();
void objectDraw();

void objectDraw(VkPipeline pipeline);

VkBuffer objectGetVertexBuffer();
VkBuffer objectGetIndicesBuffer();
uint32_t objectGetNumIndices();

void objectCreatePipeline();
void objectUpdateConstants();

void updateViewAndProjectionMatrix(VklCameraHandle camera);
void scaleUp();
void scaleDown();
void rotateXExtrinsic(float angle);
void rotateYExtrinsic(float angle);
void rotateZExtrinsic(float angle);
void rotateXIntrinsic(float angle);
void rotateYIntrinsic(float angle);
void rotateZIntrinsic(float angle);
void drawObjects(std::string filePath);
void preProcessingObjects(std::vector<glm::vec3> &positions, std::vector<unsigned int> &indices, std::vector<glm::vec3> &vertexNormals);
void calculateNormalsAndNormalize(std::vector<glm::vec3> &positions, std::vector<unsigned int> &indices, std::vector<glm::vec3> &vertexNormals);

