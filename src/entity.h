#ifndef ENTITY_H
#define ENTITY_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <chrono>
#include <random>
#include <functional>

#include <imgui/imgui.h>

#include "model.h"
#include "mesh.h"
#include "material.h"

struct Transform {
protected:
	glm::vec3 pos{ 0.0f };
	glm::quat rot{ 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec3 rotEul{ 0.0f };
	glm::vec3 scale{ 1.0f };

	glm::mat4 modelMatrix{ 1.0f };

	bool m_isDirty = false;

protected:
	glm::mat4 getLocalModelMatrix() {
		return glm::translate(glm::mat4(1.0), pos)
			* glm::toMat4(rot)
			* glm::scale(glm::mat4(1.0), scale);
	}

public:

	void computeModelMatrix() {
		modelMatrix = getLocalModelMatrix();
		m_isDirty = false;
	}

	void computeModelMatrix(const glm::mat4& parentGlobalModelMatrix) {
		modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
		m_isDirty = false;
	}

	void setLocalPosition(const glm::vec3& newPosition) {
		pos = newPosition;
		m_isDirty = true;
	}

	void setLocalRotation(const glm::vec3& newRotation) {
		rotEul = newRotation;
		rot = glm::quat(glm::radians(rotEul));
		m_isDirty = true;
	}

	void setLocalScale(const glm::vec3& newScale) {
		scale = newScale;
		m_isDirty = true;
	}

	const glm::vec3 getLocalPosition() {
		return pos;
	}

	const glm::vec3 getLocalScale() {
		return scale;
	}

	const glm::vec3 getLocalRotation() {
		return rotEul;
	}

	const glm::mat4 getModelMatrix() {
		return modelMatrix;
	}

	bool isDirty() {
		return m_isDirty;
	}

	void renderUI() {
		ImGui::Text("Transform");

		if (ImGui::DragFloat3("Position", glm::value_ptr(pos), 0.001f)) {
			m_isDirty = true;
		}
		if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotEul), 0.1f)) {
			rot = glm::quat(glm::radians(rotEul));
			m_isDirty = true;
		}
		if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.001f)) {
			m_isDirty = true;
		}
	}
};

class Entity {
private:
	const static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;

	size_t generateID() {
		// generate id using has of current time and random number
		auto curr_time = std::chrono::system_clock::now().time_since_epoch().count();

		std::random_device rd;
		std::mt19937 generator(rd());
		std::uniform_int_distribution<> distribution(0, 9999);
		auto random_number = distribution(generator);

		std::string combined = std::to_string(curr_time) + std::to_string(random_number);

		// hash
		std::hash<std::string> hasher;
		size_t hash = hasher(combined);

		return hash;
	}
public:
	size_t id;
	char name[128];
	std::vector<std::unique_ptr<Entity>> children;
	Entity* parent = nullptr;
	static unsigned int entityCount;

	Transform transform;
	std::shared_ptr<Model> model;

	Entity();
	Entity(std::string modelPath);
	Entity(char name[128], std::string modelPath);

	template<typename... TArgs>
	void addChild(const TArgs&... args) {
		children.emplace_back(std::make_unique<Entity>(args...));
		children.back()->parent = this;
		children.back()->forceUpdateTransformMatrix();
	}

	void updateTransformMatrix();
	void forceUpdateTransformMatrix();
	void renderUI(size_t& node_clicked, Entity*& selected_entity);
	void renderEntityInfoUI();
};
#endif