#ifndef PREPROCESS_PASS_H
#define PREPROCESS_PASS_H

#include "renderpass.h"
#include "gbuffer.h"

const std::vector<std::string> preprocessEffects = { "SSAO", "SSR" };
class PreprocessPass : public RenderPass {
public:
	struct OutputTexture {
		unsigned int id;
		std::string name;
		int texture_unit;

		OutputTexture(std::string name) {
			this->name = name;
			this->texture_unit = min_free_texture_unit;
		}
	};

protected:
	std::shared_ptr<GBufferPass> gBuffer;
	static const unsigned int min_texture_unit = 3;
	static unsigned int min_free_texture_unit;
	static std::vector<bool> used_texture_units;

public:
	std::string name;
	std::vector<OutputTexture> output_textures;

	PreprocessPass(unsigned int width, unsigned int height, const std::shared_ptr<GBufferPass> gBuffer, std::string name, std::vector<std::string> output_texture_names);
	virtual ~PreprocessPass();
};
#endif