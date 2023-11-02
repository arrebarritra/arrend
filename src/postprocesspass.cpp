#include "postprocesspass.h"

unsigned int PostprocessPass::min_free_texture_unit = PostprocessPass::min_texture_unit;
std::vector<bool> PostprocessPass::used_texture_units(2, false);
PostprocessPass::PostprocessPass(unsigned int width, unsigned int height, const std::shared_ptr<DeferredLightingPass> lighting, std::string name, std::vector<std::string> output_texture_names) : RenderPass(width, height)
{
	this->lighting = lighting;
	this->name = name;
	output_textures.reserve(output_texture_names.size());
	for (auto name : output_texture_names) {
		output_textures.push_back(OutputTexture(name));
		if (used_texture_units.size() == min_free_texture_unit - min_texture_unit)
			used_texture_units.push_back(true);
		else
			used_texture_units[min_free_texture_unit - min_texture_unit] = true;

		unsigned int search_start = glm::min(++min_free_texture_unit - min_texture_unit, (unsigned int)output_textures.size() - 1);
		min_free_texture_unit = min_texture_unit
			+ std::distance(used_texture_units.begin(),
				std::find(used_texture_units.begin() + search_start, used_texture_units.end(), false));
	}
}

PostprocessPass::~PostprocessPass() {
	if (output_textures.size() > 0) {
		min_free_texture_unit = (min_free_texture_unit < output_textures[0].texture_unit) ? min_free_texture_unit : output_textures[0].texture_unit;
		for (auto output_texture : output_textures) {
			used_texture_units[output_texture.texture_unit - min_texture_unit] = false;
		}
	}
}
