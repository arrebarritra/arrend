#ifndef SKYBOX_H
#define SKYBOX_H

#include "postprocesspass.h"
#include "stb_image.h"

class SkyboxPass : public PostprocessPass
{
private:
	unsigned int skyboxTexture;
	std::unique_ptr<Shader> skyboxShader;

	static const std::string name;
	static const std::vector<std::string> skybox_output_textures;
public:
	static const std::string skybox_faces[6];
	bool loadFailed = false;

	unsigned int loadCubemap(std::string faces[6]);
	SkyboxPass(unsigned int width, unsigned int height, const std::shared_ptr<DeferredLightingPass> lighting, std::string faces[6]);
	~SkyboxPass();
	void Render() override;
	void ResizeBuffers(unsigned int width, unsigned int height) override;
};

#endif