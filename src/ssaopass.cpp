#include "ssaopass.h"
#include "deferredlighting.h"

const std::string SSAOPass::name = "SSAO";
const std::vector<std::string> SSAOPass::ssao_output_textures = { "ssao" };
SSAOPass::SSAOPass(unsigned int width, unsigned int height, const std::shared_ptr<GBufferPass> gBuffer) : PreprocessPass(width, height, gBuffer, SSAOPass::name, SSAOPass::ssao_output_textures)
{
	// ssao kernel
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	ssaoKernel.reserve(64);
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = (float)i / 64.0;
		scale = glm::mix(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}

	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// ssao buffer
	glGenFramebuffers(1, &ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

	glGenTextures(1, &output_textures[0].id);
	glBindTexture(GL_TEXTURE_2D, output_textures[0].id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_textures[0].id, 0);

	// ssao blur
	glGenFramebuffers(1, &ssaoBlurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
	glGenTextures(1, &ssaoBlurBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoBlurBuffer, 0);

	// set shader parameters
	ssaoShader = std::make_unique<Shader>("src/shaders/ssao.vert", "src/shaders/ssao.frag");
	ssaoBlurShader = std::make_unique<Shader>("src/shaders/ssaoblur.vert", "src/shaders/ssaoblur.frag");

	ssaoShader->use();
	ssaoShader->setInt("gPosition", 0);
	ssaoShader->setInt("gNormal", 1);
	ssaoShader->setInt("noiseTexture", 2);
	for (unsigned int i = 0; i < ssaoKernel.size(); i++) {
		ssaoShader->setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	}

	// bind matrix uniform block
	ssaoShader->bindUniformBlock("Matrices", 0);

	ssaoBlurShader->use();
	ssaoBlurShader->setInt("ssaoInput", 0);

}

SSAOPass::~SSAOPass()
{
	glDeleteTextures(1, &noiseTexture);
	glDeleteTextures(1, &output_textures[0].id);
	glDeleteTextures(1, &ssaoBlurBuffer);

	glDeleteFramebuffers(1, &ssaoFBO);
	glDeleteFramebuffers(1, &ssaoBlurFBO);
}

void SSAOPass::Render()
{
	// ssao
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	glClear(GL_COLOR_BUFFER_BIT);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gBuffer->gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gBuffer->gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);

	ssaoShader->use();
	RenderQuad();

	// ssao blur
	ssaoBlurShader->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, output_textures[0].id);
	RenderQuad();
}

void SSAOPass::ResizeBuffers(unsigned int width, unsigned int height)
{
	TARGET_WIDTH = width; TARGET_HEIGHT = height;
	glBindTexture(GL_TEXTURE_2D, output_textures[0].id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glBindTexture(GL_TEXTURE_2D, ssaoBlurBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, TARGET_WIDTH, TARGET_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
}
