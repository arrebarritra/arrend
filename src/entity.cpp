#include "entity.h"

unsigned int Entity::entityCount = 0;

Entity::Entity() {
	this->id = generateID();
	char entity_name[128];
	snprintf(entity_name, 128, "Entity %d", entityCount++);
	strncpy_s(this->name, entity_name, 128);
}

Entity::Entity(std::string modelPath) {
	this->id = generateID();
	char entity_name[128];
	snprintf(entity_name, 128, "Entity %d", entityCount++);
	strncpy_s(this->name, entity_name, 128);
	this->model = Model::loadModel(modelPath);
}

Entity::Entity(char name[128], std::string modelPath) {
	this->id = generateID();
	strncpy_s(this->name, name, 128);
	this->model = Model::loadModel(modelPath);
}

void Entity::updateTransformMatrix() {
	if (transform.isDirty()) {
		forceUpdateTransformMatrix();
		return;
	}

	for (auto&& child : children) child->updateTransformMatrix();
}

void Entity::forceUpdateTransformMatrix() {
	if (parent)
		transform.computeModelMatrix(parent->transform.getModelMatrix());
	else
		transform.computeModelMatrix();

	for (auto&& child : children) child->forceUpdateTransformMatrix();
}

void Entity::renderUI(size_t& node_clicked, Entity*& selected_entity) {
	ImGuiTreeNodeFlags flags = base_flags;
	if (children.size() == 0) flags |= ImGuiTreeNodeFlags_Leaf;
	if (id == node_clicked) flags |= ImGuiTreeNodeFlags_Selected;

	bool node_open = ImGui::TreeNodeEx((std::string(name) + "###" + std::to_string(id)).c_str(), flags);
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) { node_clicked = id; selected_entity = this; }
	if (id == node_clicked)	renderEntityInfoUI();
	if (node_open) {
		for (auto& child : children) child->renderUI(node_clicked, selected_entity);
		ImGui::TreePop();
	}
}

void Entity::renderEntityInfoUI() {
	ImGui::Begin("Entity info");

	ImGui::InputText("Name", name, 128);
	ImGui::Spacing();
	transform.renderUI();
	model->renderUI();

	ImGui::End();
}