#ifndef GBUFFER_H
#define GBUFFER_H

#include "renderpass.h"
#include "scene.h"

class GBufferPass : public RenderPass
{
private:
	std::shared_ptr<Scene> scene;
	std::unique_ptr<Shader> gBufferShader;

public:
	unsigned int gBuffer;
	unsigned int gPosition, gNormal, gAlbedoSpec;
	unsigned int rboDepthGBuffer;

	GBufferPass(unsigned int width, unsigned int height, std::shared_ptr<Scene> scene);
	~GBufferPass();
	void Render() override;
	void ResizeBuffers(unsigned int width, unsigned int height) override;

private:
	void renderEntityToGBuffer(Entity& entity);
};
#endif