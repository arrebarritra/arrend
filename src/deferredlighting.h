#ifndef DEFERRED_LIGHTING_H
#define DEFERRED_LIGHTING_H

#include "renderpass.h"
#include "preprocesspass.h"
#include "gbuffer.h"

class DeferredLightingPass : public RenderPass
{
private:
	std::unique_ptr<Shader> lightingPassShader;
	std::shared_ptr<GBufferPass> gBuffer;
public:
	unsigned int hdrFBO;
	unsigned int colorBuffers[2];
	unsigned int rboDepthLighting;

	std::vector<std::shared_ptr<PreprocessPass>> preprocessPasses;

	DeferredLightingPass(unsigned int width, unsigned int height, std::shared_ptr<GBufferPass> gBuffer);
	~DeferredLightingPass();
	void Render() override;
	void ResizeBuffers(unsigned int width, unsigned int height) override;

	void AddPreprocessPass(std::shared_ptr<PreprocessPass> preprocessPass);
	void RemovePreprocessPass(std::shared_ptr<PreprocessPass> preprocessPass);
};

#endif
