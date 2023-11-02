#ifndef SHADOWMAP_PASS_H
#define SHADOWMAP_PASS_H

#include "renderpass.h"
#include "camera.h";
#include "scene.h"
#include "entity.h"

class ShadowMapPass : public RenderPass
{
private:
	enum LightType {
		Point, Direction, Spot
	};

	unsigned int depthMapFBO;
	LightType lightType;

	std::shared_ptr<Shader> depthShader;
	std::shared_ptr<Scene> scene;
	std::shared_ptr<Camera> camera;
	glm::mat4 createLightFrustum();

public:
	unsigned int depthMap;

	ShadowMapPass(unsigned int width, unsigned int height, LightType lightType);
	~ShadowMapPass();
	void Render() override;
	void ResizeBuffers(unsigned int width, unsigned int height) override;

private:
	void renderEntityToDepthMap(Entity& entity);
};

#endif