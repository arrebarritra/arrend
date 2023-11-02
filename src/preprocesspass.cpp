#include "preprocesspass.h"

unsigned int PreprocessPass::min_free_texture_unit = PreprocessPass::min_texture_unit;
std::vector<bool> PreprocessPass::used_texture_units(0);
PreprocessPass::PreprocessPass(unsigned int width, unsigned int height, const std::shared_ptr<GBufferPass> gBuffer, std::string name, std::vector<std::string> output_texture_names) : RenderPass(width, height)
{
	this->gBuffer = gBuffer;
	this->name = name;
	output_textures.reserve(output_texture_names.size());
	for (auto name : output_texture_names) {
		output_textures.push_back(OutputTexture(name));
		if (used_texture_units.size() == min_free_texture_unit - min_texture_unit)
			used_texture_units.push_back(true);
		else
			used_texture_units[min_free_texture_unit - min_texture_unit] = true;

		unsigned int search_start = glm::min(++min_free_texture_unit - min_texture_unit, (unsigned int)used_texture_units.size() - 1);
		min_free_texture_unit = min_texture_unit
			+ std::distance(used_texture_units.begin(),
				std::find(used_texture_units.begin() + search_start, used_texture_units.end(), false));
	}
}

PreprocessPass::~PreprocessPass() {
	min_free_texture_unit = (min_free_texture_unit < output_textures[0].texture_unit) ? min_free_texture_unit : output_textures[0].texture_unit;
	for (auto output_texture : output_textures) {
		used_texture_units[output_texture.texture_unit - min_texture_unit] = false;
	}
}
