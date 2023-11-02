#ifndef SSRPASS_H
#define SSRPASS_H

#include "preprocesspass.h"
#include "gbuffer.h"

class SSRPass : public PreprocessPass
{
private:
	unsigned int ssrFBO, ssrBlurFBO;
	std::unique_ptr<Shader> ssrShader;

	static const std::string name;
	static const std::vector<std::string> ssr_output_textures;

public:

	SSRPass(unsigned int width, unsigned int height, const std::shared_ptr<GBufferPass> gBuffer);
	~SSRPass();
	void Render() override;
	void ResizeBuffers(unsigned int width, unsigned int height) override;
};
#endif