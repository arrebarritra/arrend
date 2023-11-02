#include "shadowmap.h"

glm::mat4 ShadowMapPass::createLightFrustum()
{
	return glm::mat4();
}

ShadowMapPass::ShadowMapPass(unsigned int width, unsigned int height, LightType lightType) : RenderPass(width, height), lightType(lightType)
{
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	depthShader = std::make_shared<Shader>("src/shaders/depthshader.vert", "src/shaders/depthshader.frag");
}

ShadowMapPass::~ShadowMapPass()
{
	glDeleteTextures(1, &depthMap);
	glDeleteFramebuffers(1, &depthMapFBO);
}

void ShadowMapPass::Render()
{
	glViewport(0, 0, TARGET_WIDTH, TARGET_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	glm::mat4 lightSpaceMatrix = createLightFrustum();

	depthShader->use();
	depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
	for (auto&& entity : scene->root->children) {
		entity->updateTransformMatrix();
		renderEntityToDepthMap(*entity);
	}
}

void ShadowMapPass::ResizeBuffers(unsigned int width, unsigned int height)
{
	TARGET_WIDTH = width; TARGET_HEIGHT = height;
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}

void ShadowMapPass::renderEntityToDepthMap(Entity& entity)
{
	depthShader->setMat4("model", entity.transform.getModelMatrix());
	entity.model->DrawDepth();
	for (auto&& child : entity.children) {
		renderEntityToDepthMap(*child);
	}
}
