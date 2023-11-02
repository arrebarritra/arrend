#ifndef RENDERPASS_H
#define RENDERPASS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <random>

#include "shader.h"



class RenderPass
{
public:
	RenderPass(unsigned int width, unsigned int height) : TARGET_WIDTH(width), TARGET_HEIGHT(height) {};
	virtual ~RenderPass() {};
	virtual void Render() = 0;
	virtual void ResizeBuffers(unsigned int width, unsigned int height) = 0;

protected:
	unsigned int TARGET_WIDTH, TARGET_HEIGHT;

	static unsigned int quadVAO; static unsigned int quadVBO;
	static unsigned int cubeVAO; static unsigned int cubeVBO;
	static void RenderQuad();
	static void RenderCube();
};
#endif
