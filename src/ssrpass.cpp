#include "ssrpass.h"
#include "deferredlighting.h"

const std::string SSRPass::name = "SSR";
const std::vector<std::string> SSRPass::ssr_output_textures = { "ssr" };
SSRPass::SSRPass(unsigned int width, unsigned int height, const std::shared_ptr<GBufferPass> gBuffer) : PreprocessPass(width, height, gBuffer, SSRPass::name, SSRPass::ssr_output_textures)
{
	// ssr
	glGenFramebuffers(1, &ssrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);

	glGenTextures(1, &output_textures[0].id);
	glBindTexture(GL_TEXTURE_2D, output_textures[0].id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_textures[0].id, 0);

	// set shader parameters
	ssrShader = std::make_unique<Shader>("src/shaders/ssr.vert", "src/shaders/ssr.frag");

	ssrShader->use();
	ssrShader->setInt("gPosition", 0);
	ssrShader->setInt("gNormal", 1);
	ssrShader->setInt("gAlbedoSpec", 2);

	// bind matrix uniform block
	ssrShader->bindUniformBlock("Matrices", 0);

}

SSRPass::~SSRPass()
{
	glDeleteTextures(1, &output_textures[0].id);
	glDeleteFramebuffers(1, &ssrFBO);
}

void SSRPass::Render()
{
	// ssr
	glBindFramebuffer(GL_FRAMEBUFFER, ssrFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer->gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer->gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gBuffer->gAlbedoSpec);

	ssrShader->use();
	RenderQuad();
}

void SSRPass::ResizeBuffers(unsigned int width, unsigned int height)
{
	TARGET_WIDTH = width; TARGET_HEIGHT = height;
	glBindTexture(GL_TEXTURE_2D, output_textures[0].id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
}
