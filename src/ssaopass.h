#ifndef SSAOPASS_H
#define SSAOPASS_H

#include "preprocesspass.h"
#include "gbuffer.h"

class SSAOPass : public PreprocessPass
{
private:
	unsigned int ssaoFBO, ssaoBlurFBO;
	unsigned int noiseTexture, ssaoBlurBuffer;
	std::unique_ptr<Shader> ssaoShader;
	std::unique_ptr<Shader> ssaoBlurShader;

	static const std::string name;
	static const std::vector<std::string> ssao_output_textures;

public:
	SSAOPass(unsigned int width, unsigned int height, const std::shared_ptr<GBufferPass> gBuffer);
	~SSAOPass();
	void Render() override;
	void ResizeBuffers(unsigned int width, unsigned int height) override;
};
#endif