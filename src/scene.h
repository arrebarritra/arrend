#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>

#include <memory>
#include <vector>

#include <imgui/imgui.h>
#include "entity.h"
#include "light.h"
#include "model.h"

struct SceneLights {
	PointLight pointLights[1];
	DirLight dirLights[1];
	SpotLight spotLights[1];
};

class Scene {
	enum LoadSuccess {
		waiting, failed, successful
	};

private:
	size_t node_clicked = -1;
	Entity* selected_entity;
	char model_file_path[128] = "models/bat.glb";
	LoadSuccess load_success = waiting;
public:
	std::unique_ptr<Entity> root;
	SceneLights lights;
	std::vector<unsigned int> shadowPointLights;
	std::vector<unsigned int> shadowDirLights;
	std::vector<unsigned int> shadowSpotLights;

	Scene() {
		root = std::make_unique<Entity>();
		strncpy_s(root->name, "root", 128);
		node_clicked = root->id;
		selected_entity = root.get();
	}

	void renderUI() {
		ImGui::Begin("Scene##window");
		if (ImGui::IsItemActive()) { node_clicked = root->id; selected_entity = root.get(); load_success = waiting; }

		ImGui::InputText("Model file name", model_file_path, IM_ARRAYSIZE(model_file_path));
		if (ImGui::Button("Add")) {
			std::shared_ptr<Model> model = Model::loadModel(model_file_path);
			if (model) {
				selected_entity->addChild(model_file_path);
				load_success = successful;
			}
			else {
				load_success = failed;
			}
		}

		if (load_success == successful)		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Model loaded successfully");
		else if (load_success == failed)	ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Model could not be loaded");
		else	ImGui::Spacing();

		ImGui::SeparatorText("Scene##header");
		for (auto& child : root->children)
			child->renderUI(node_clicked, selected_entity);

		// Lights UI
		int index;

		ImGui::SeparatorText("Point lights");
		index = 0;
		for (auto& pointLight : lights.pointLights)
			pointLight.renderUI(++index);

		ImGui::SeparatorText("Directional lights");
		index = 0;
		for (auto& dirLight : lights.dirLights)
			dirLight.renderUI(++index);
		
		ImGui::SeparatorText("Spot lights");
		index = 0;
		for (auto& spotLight : lights.spotLights)
			spotLight.renderUI(++index);

		ImGui::End();
	}
};
#endif