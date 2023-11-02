#ifndef POSTPROCESS_PASS_H
#define POSTPROCESS_PASS_H

#include "renderpass.h"
#include "deferredlighting.h"

const std::vector<std::string> postprocessEffects = { "Bloom" };
class PostprocessPass : public RenderPass {
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
	std::shared_ptr<DeferredLightingPass> lighting;
	static const unsigned int min_texture_unit = 1;
	static unsigned int min_free_texture_unit;
	static std::vector<bool> used_texture_units;

public:
	std::string name;
	std::vector<OutputTexture> output_textures;

	PostprocessPass(unsigned int width, unsigned int height, const std::shared_ptr<DeferredLightingPass> lighting, std::string name, std::vector<std::string> output_texture_names);
	virtual ~PostprocessPass();
};
#endif