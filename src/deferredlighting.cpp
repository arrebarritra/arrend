#include "deferredlighting.h"

DeferredLightingPass::DeferredLightingPass(unsigned int width, unsigned int height, std::shared_ptr<GBufferPass> gBuffer) : RenderPass(width, height)
{
	this->gBuffer = gBuffer;

	// output buffer
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

	glGenTextures(2, &colorBuffers[0]);
	for (unsigned int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	unsigned int attachmentsColor[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachmentsColor);
	// create and attach depth buffer (renderbuffer)
	glGenRenderbuffers(1, &rboDepthLighting);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthLighting);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TARGET_WIDTH, TARGET_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthLighting);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;

	// set shader parameters
	lightingPassShader = std::make_unique<Shader>("src/shaders/glighting.vert", "src/shaders/glighting.frag");

	lightingPassShader->use();
	lightingPassShader->setInt("gPosition", 0);
	lightingPassShader->setInt("gNormal", 1);
	lightingPassShader->setInt("gAlbedoSpec", 2);

	// bind matrix uniform block
	lightingPassShader->bindUniformBlock("Matrices", 0);
	lightingPassShader->bindUniformBlock("Lights", 1);
}

DeferredLightingPass::~DeferredLightingPass()
{
	glDeleteTextures(2, &colorBuffers[0]);
	glDeleteRenderbuffers(1, &rboDepthLighting);
	glDeleteFramebuffers(1, &hdrFBO);
}

void DeferredLightingPass::Render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer->gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer->gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer->gAlbedoSpec);

	for (auto& p : preprocessPasses) {
		for (auto& output_texture : p->output_textures) {
			glActiveTexture(GL_TEXTURE0 + output_texture.texture_unit);
			glBindTexture(GL_TEXTURE_2D, output_texture.id);
		}
	}

	lightingPassShader->use();
	RenderQuad();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer->gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, hdrFBO); // write to default framebuffer
	glBlitFramebuffer(0, 0, TARGET_WIDTH, TARGET_HEIGHT, 0, 0, TARGET_WIDTH, TARGET_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void DeferredLightingPass::ResizeBuffers(unsigned int width, unsigned int height)
{
	TARGET_WIDTH = width; TARGET_HEIGHT = height;
	for (unsigned int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthLighting);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TARGET_WIDTH, TARGET_HEIGHT);
}

void DeferredLightingPass::AddPreprocessPass(std::shared_ptr<PreprocessPass> preprocessPass)
{
	preprocessPasses.push_back(preprocessPass);
	lightingPassShader->use();
	for (auto& output_texture : preprocessPass->output_textures) {
		lightingPassShader->setInt(output_texture.name, output_texture.texture_unit);
		lightingPassShader->setBool(output_texture.name + "On", true);
	}
}

void DeferredLightingPass::RemovePreprocessPass(std::shared_ptr<PreprocessPass> preprocessPass)
{
	lightingPassShader->use();
	for (auto& output_texture : preprocessPass->output_textures)
		lightingPassShader->setBool(output_texture.name + "On", false);

	preprocessPasses.erase(std::remove(preprocessPasses.begin(), preprocessPasses.end(), preprocessPass));
}
