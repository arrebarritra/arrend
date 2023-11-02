#ifndef HDRPASS_H
#define HDRPASS_H

#include "renderpass.h"
#include "deferredlighting.h"
#include "postprocesspass.h"

#include <map>

class BloomPass;
class HDRPass : public RenderPass
{
private:
	std::unique_ptr<Shader> HDRShader;
	std::shared_ptr<DeferredLightingPass> lighting;

public:
	enum HDRmode {
		off,
		reinhard,
		exposure
	};
	int hdrMode = HDRmode::off;
	float exposureVal = 1.0f;
	const char* hdrModes[3] = { "Off", "Reinhard", "Exposure" };

	std::vector<std::shared_ptr<PostprocessPass>> postprocessPasses;

	HDRPass(unsigned int width, unsigned int height, std::shared_ptr<DeferredLightingPass> lighting);
	~HDRPass();
	void Render() override;
	void ResizeBuffers(unsigned int width, unsigned int height) override;

	void AddPostprocessPass(std::shared_ptr<PostprocessPass> postprocessPass);
	void RemovePostprocessPass(std::shared_ptr<PostprocessPass> postprocessPass);

	void updateHDRMode();
	void updateExposure();
};

#endif