#ifndef BLOOMPASS_H
#define BLOOMPASS_H

#include "postprocesspass.h"
#include "deferredlighting.h"

class BloomPass : public PostprocessPass
{
private:
	std::unique_ptr<Shader> bloomBlurShader;

	static const std::string name;
	static const std::vector<std::string> bloom_output_textures;

	unsigned int bloomPingPongFBO[2];
	unsigned int bloomPingPongBuffers[2];
	unsigned int bloomBlurPasses = 10;

public:
	BloomPass(unsigned int width, unsigned int height, const std::shared_ptr<DeferredLightingPass> lighting);
	~BloomPass();
	void Render() override;
	void ResizeBuffers(unsigned int width, unsigned int height) override;
};

#endif