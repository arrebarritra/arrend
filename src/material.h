#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <imgui/imgui.h>


struct Texture {
	unsigned int id;
	int width, height;
	std::string type;
	std::string path;
	std::string name;

	void renderUI() {
		ImGui::LabelText(name.c_str(), type.c_str());
		ImGui::Text("%d x %d", width, height);
		ImGui::Image((ImTextureID)id, ImVec2(100.0f, 100.0f * (height / (float)width)));
	}
};


struct Material {
	std::string name;
	std::shared_ptr<Texture> texture_diffuse;
	std::shared_ptr<Texture> texture_specular;
	std::shared_ptr<Texture> normal_map;

	void renderUI() {
		ImGui::SeparatorText("Material");
		ImGui::Text(name.c_str());

		if (texture_diffuse) texture_diffuse->renderUI();
		if (texture_specular) texture_specular->renderUI();
		if (normal_map) normal_map->renderUI();
	}
};

#endif