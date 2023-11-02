#include "hdrpass.h"

#include "bloompass.h"

HDRPass::HDRPass(unsigned int width, unsigned int height, std::shared_ptr<DeferredLightingPass> lighting) : RenderPass(width, height)
{
	this->lighting = lighting;

	HDRShader = std::make_unique<Shader>("src/shaders/hdr.vert", "src/shaders/hdr.frag");

	HDRShader->use();
	HDRShader->setInt("hdrMode", hdrMode);
	HDRShader->setInt("hdrBuffer", 0);
	HDRShader->setFloat("exposure", exposureVal);
}

HDRPass::~HDRPass()
{
}

void HDRPass::Render()
{
	// render to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lighting->colorBuffers[0]);

	for (auto& p : postprocessPasses) {
		for (auto& output_texture : p->output_textures) {
			glActiveTexture(GL_TEXTURE0 + output_texture.texture_unit);
			glBindTexture(GL_TEXTURE_2D, output_texture.id);
		}
	}

	HDRShader->use();
	RenderQuad();
}

void HDRPass::ResizeBuffers(unsigned int width, unsigned int height)
{
	TARGET_WIDTH = width; TARGET_HEIGHT = height;
}

void HDRPass::AddPostprocessPass(std::shared_ptr<PostprocessPass> postprocessPass)
{
	postprocessPasses.push_back(postprocessPass);
	HDRShader->use();
	for (auto& output_texture : postprocessPass->output_textures) {
		HDRShader->setInt(output_texture.name, output_texture.texture_unit);
		HDRShader->setBool(output_texture.name + "On", true);
	}
}

void HDRPass::RemovePostprocessPass(std::shared_ptr<PostprocessPass> postprocessPass)
{
	HDRShader->use();
	for (auto& output_texture : postprocessPass->output_textures)
		HDRShader->setBool(output_texture.name + "On", false);

	postprocessPasses.erase(std::remove(postprocessPasses.begin(), postprocessPasses.end(), postprocessPass));
}

void HDRPass::updateHDRMode()
{
	HDRShader->use();
	HDRShader->setInt("hdrMode", hdrMode);
}

void HDRPass::updateExposure()
{
	HDRShader->use();
	HDRShader->setFloat("exposure", exposureVal);
}
