#include "skybox.h"

const std::string SkyboxPass::skybox_faces[6] = {"right", "left", "top", "bottom", "front", "back"};
const std::string SkyboxPass::name = "Skybox";
const std::vector<std::string> SkyboxPass::skybox_output_textures = {};
unsigned int SkyboxPass::loadCubemap(std::string faces[6])
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	loadFailed = false;
	for (unsigned int i = 0; i < 6; i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			loadFailed = true;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

SkyboxPass::SkyboxPass(unsigned int width, unsigned int height, const std::shared_ptr<DeferredLightingPass> lighting, std::string faces[6]) : PostprocessPass(width, height, lighting, name, skybox_output_textures)
{
	skyboxTexture = loadCubemap(faces);

	skyboxShader = std::make_unique<Shader>("src/shaders/skybox.vert", "src/shaders/skybox.frag");
	skyboxShader->use();
	skyboxShader->setInt("cubemap", 0);

	skyboxShader->bindUniformBlock("Matrices", 0);
}

SkyboxPass::~SkyboxPass()
{
	glDeleteTextures(1, &skyboxTexture);
}

void SkyboxPass::Render()
{
	glBindFramebuffer(GL_FRAMEBUFFER, lighting->hdrFBO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

	skyboxShader->use();
	RenderCube();
}

void SkyboxPass::ResizeBuffers(unsigned int width, unsigned int height) {}
