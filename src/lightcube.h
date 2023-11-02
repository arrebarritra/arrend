#ifndef  LIGHTCUBE_H
#define LIGHTCUBE_H

#include "renderpass.h"
#include "scene.h"

class LightCubePass : public RenderPass
{
private:
	std::unique_ptr<Shader> lightCubeShader;
	std::shared_ptr<Scene> scene;

public:
	LightCubePass(unsigned int width, unsigned int height);
	~LightCubePass();
	void Render() override;
	void ResizeBuffers(unsigned int width, unsigned int height) override;
};

#endif // !define LIGHTCUBE_H