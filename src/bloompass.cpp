#include "bloompass.h"

const std::string BloomPass::name = "Bloom";
const std::vector<std::string> BloomPass::bloom_output_textures = { "bloom" };
BloomPass::BloomPass(unsigned int width, unsigned int height, const std::shared_ptr<DeferredLightingPass> lighting) : PostprocessPass(width, height, lighting, BloomPass::name, BloomPass::bloom_output_textures)
{
	glGenFramebuffers(2, &bloomPingPongFBO[0]);
	glGenTextures(2, &bloomPingPongBuffers[0]);
	output_textures[0].id = bloomPingPongBuffers[1];

	for (unsigned int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, bloomPingPongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, bloomPingPongBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomPingPongBuffers[i], 0);
	}

	bloomBlurShader = std::make_unique<Shader>("src/shaders/bloom.vert", "src/shaders/bloom.frag");
	bloomBlurShader->use();
	bloomBlurShader->setInt("image", 0);
}

BloomPass::~BloomPass()
{
	glDeleteTextures(2, &bloomPingPongBuffers[0]);
	glDeleteFramebuffers(2, &bloomPingPongFBO[0]);
}

void BloomPass::Render()
{
	bool horizontal = true, first_iteration = true;
	bloomBlurShader->use();
	glActiveTexture(GL_TEXTURE0);
	for (unsigned int i = 0; i < bloomBlurPasses; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, bloomPingPongFBO[horizontal]);
		bloomBlurShader->setInt("horizontal", horizontal);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? lighting->colorBuffers[1] : bloomPingPongBuffers[!horizontal]);
		RenderQuad();
		horizontal = !horizontal;
		if (first_iteration) first_iteration = false;
	}
}

void BloomPass::ResizeBuffers(unsigned int width, unsigned int height)
{
	TARGET_WIDTH = width; TARGET_HEIGHT = height;
	for (unsigned int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, bloomPingPongBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	}
}
