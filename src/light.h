#ifndef LIGHT_H
#define LIGHT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <imgui/imgui.h>

struct PointLight {
	glm::vec4 pos, color;
	float Linear, Quadratic, pad1, pad2;

	void renderUI(int index) {
		if (ImGui::TreeNode(("Point light " + std::to_string(index)).c_str())) {
			ImGui::DragFloat3("Position", glm::value_ptr(pos));
			ImGui::ColorEdit3("Colour", glm::value_ptr(color));
			ImGui::Text("Attenuation");
			ImGui::DragFloat("##Linear", &Linear, 0.01, 0.0, 0.0, "Linear: %.2f");
			ImGui::DragFloat("##Quadratic", &Quadratic, 0.01, 0.0, 0.0, "Quadratic: %.2f");
			ImGui::TreePop();
		}
	}
};

struct DirLight {
	glm::vec4 dir, color;
	void renderUI(int index) {
		if (ImGui::TreeNode(("Directional light " + std::to_string(index)).c_str())) {
			if (ImGui::DragFloat3("Direction", glm::value_ptr(dir), 0.001f)) {
				if (glm::vec3(dir) == glm::zero<glm::vec3>())
					dir = glm::vec4(1.0f, 0.0f, 0.0f, 0.0);
				else
					dir = glm::normalize(dir);
			}
			ImGui::ColorEdit3("Colour", glm::value_ptr(color));
			ImGui::TreePop();
		}
	}
};

struct SpotLight {
	glm::vec4 pos, dir, color;
	float cutOff, outerCutOff, Linear, Quadratic;

	void renderUI(int index) {
		if (ImGui::TreeNode(("Spot light " + std::to_string(index)).c_str())) {
			ImGui::DragFloat3("Position", glm::value_ptr(pos));
			if (ImGui::DragFloat3("Direction", glm::value_ptr(dir), 0.001f)) {
				if (glm::vec3(dir) == glm::zero<glm::vec3>())
					dir = glm::vec4(1.0f, 0.0f, 0.0f, 0.0);
				else
					dir = glm::normalize(dir);
			}
			ImGui::ColorEdit3("Colour", glm::value_ptr(color));
			ImGui::Text("Light cone");

			float cutOffDeg = glm::degrees(glm::acos(cutOff));
			float outerCutOffDeg = glm::degrees(glm::acos(outerCutOff));
			if (ImGui::DragFloat("Cutoff", &cutOffDeg, 0.1, 0.0, 90.0f, "%.2f deg")) {
				cutOff = glm::cos(glm::radians(cutOffDeg));
			}
			if (ImGui::DragFloat("Outer cutoff", &outerCutOffDeg, 0.1, 0.0, 90.0f, "%.2f deg")) {
				outerCutOffDeg = glm::clamp(outerCutOffDeg, cutOffDeg, 90.0f);
				outerCutOff = glm::cos(glm::radians(outerCutOffDeg));
			}
			ImGui::Text("Attenuation");
			ImGui::DragFloat("##Linear", &Linear, 0.01, 0.0, 0.0, "Linear: %.2f");
			ImGui::DragFloat("##Quadratic", &Quadratic, 0.01, 0.0, 0.0, "Quadratic: %.2f");
			ImGui::TreePop();
		}
	}
};

#endif