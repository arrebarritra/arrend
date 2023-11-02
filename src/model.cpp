#include "model.h"
#include <imgui/imgui.h>

std::vector<std::shared_ptr<Model>> Model::models = {};
std::vector<std::shared_ptr<Material>> Model::materials = {};
std::vector<std::shared_ptr<Texture>> Model::textures_loaded = {};

int Model::loadMeshClicked = -1;
void Model::Draw()
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].Draw();
}

void Model::DrawDepth()
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].DrawDepth();
}

void Model::renderUI()
{
	if (ImGui::CollapsingHeader("Model info")) {
		ImGui::LabelText(std::to_string(meshes.size()).c_str(), "Meshes");
		int meshIndex = 0;
		for (auto& mesh : meshes) {
			if (ImGui::Selectable(mesh.name.c_str(), meshIndex == selectedMeshIndex)) {
				selectedMeshIndex = meshIndex;
			}
			if (meshIndex == selectedMeshIndex)	mesh.renderUI();
			meshIndex++;
		}
	}
}

void Model::renderLoadInfoUI()
{
	ImGui::Begin("Loaded assets");

	if (ImGui::CollapsingHeader("Models", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& model : models) {
			if (ImGui::TreeNode((model->name + "##treemodelname").c_str())) {
				ImGui::LabelText(std::to_string(model->meshes.size()).c_str(), "Meshes");
				int meshIndex = 0;
				for (auto& mesh : model->meshes) {
					ImGui::Selectable(mesh.name.c_str(), loadMeshClicked == meshIndex);
					meshIndex++;
				}
				ImGui::TreePop();
			}
		}
	}
	if (ImGui::CollapsingHeader("Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& material : materials) {
			if (ImGui::TreeNode((material->name + "##treematname").c_str())) {
				material->renderUI();
				ImGui::TreePop();
			}
		}
	}
	if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto& texture : textures_loaded)
			if (ImGui::TreeNode((texture->name + "##treetexturename").c_str())) {
				texture->renderUI();
				ImGui::TreePop();
			}
	}
	ImGui::End();
}


std::shared_ptr<Model> Model::loadModel(std::string path) {
	for (unsigned int i = 0; i < models.size(); i++) {
		if (std::strcmp(path.c_str(), models[i]->path.c_str()) == 0) {
			return models[i];
		}
	}

	Assimp::Importer import;
	auto processFlags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals | aiProcess_OptimizeMeshes;
	const aiScene* scene = import.ReadFile(path, processFlags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return nullptr;
	}

	auto model = std::make_shared<Model>(path, path.substr(0, path.find_last_of('/')), scene);
	models.push_back(model);

	return models.back();
}

Model::Model(std::string path, std::string directory, const aiScene* scene) {
	this->path = path;
	this->directory = path.substr(0, path.find_last_of('/'));
	std::string name = scene->mName.C_Str();
	this->name = (name != "") ? name : "model" + std::to_string(models.size());
	this->processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	std::string name = mesh->mName.C_Str();
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// process vertex positions, normals and texture coordinates

		glm::vec3 position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		vertex.Position = position;

		glm::vec3 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		vertex.Normal = normal;

		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 texcoord;
			texcoord.x = mesh->mTextureCoords[0][i].x;
			texcoord.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = texcoord;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		glm::vec3 tangent;
		tangent.x = mesh->mTangents[i].x;
		tangent.y = mesh->mTangents[i].y;
		tangent.z = mesh->mTangents[i].z;
		vertex.Tangent = tangent;

		glm::vec3 bitangent;
		bitangent.x = mesh->mBitangents[i].x;
		bitangent.y = mesh->mBitangents[i].y;
		bitangent.z = mesh->mBitangents[i].z;
		vertex.Bitangent = bitangent;

		vertices.push_back(vertex);
	}
	// process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* aiMat = scene->mMaterials[mesh->mMaterialIndex];

		for (unsigned int i = 0; i < materials.size(); i++) {
			if (std::strcmp(aiMat->GetName().C_Str(), materials[i]->name.c_str()) == 0) {
				return Mesh(name, vertices, indices, materials[i]);
			}
		}
		auto material = std::make_shared<Material>();
		std::string name = aiMat->GetName().C_Str();
		material->name = (name != "") ? name : "mat" + std::to_string(materials.size());
		material->texture_diffuse = loadMaterialTexture(scene, aiMat, aiTextureType_DIFFUSE, "texture_diffuse");
		material->texture_specular = loadMaterialTexture(scene, aiMat, aiTextureType_SPECULAR, "texture_specular");
		material->normal_map = loadMaterialTexture(scene, aiMat, aiTextureType_HEIGHT, "texture_normals");
		materials.push_back(material);
	}
	return Mesh(name, vertices, indices, materials.back());
}

std::shared_ptr<Texture> Model::loadMaterialTexture(const aiScene* scene, aiMaterial* mat, aiTextureType type, std::string typeName) {
	aiString str;
	if (mat->GetTextureCount(type) > 0) {
		mat->GetTexture(type, 0, &str);
		for (unsigned int i = 0; i < textures_loaded.size(); i++)
		{
			if (std::strcmp(textures_loaded[i]->path.c_str(), str.C_Str()) == 0)
			{
				return textures_loaded[i];
			}
		}

		auto texture = std::make_shared<Texture>();
		int tex_width, tex_height;
		if (auto tex = scene->GetEmbeddedTexture(str.C_Str())) {
			texture->id = TextureEmbedded(tex, tex_width, tex_height);
			std::string name = tex->mFilename.C_Str();
			texture->name = (name != "") ? name : "tex" + std::to_string(textures_loaded.size());
		}
		else
		{
			texture->id = TextureFromFile(str.C_Str(), tex_width, tex_height, directory);
			std::string name = texture->path.substr(texture->path.find_last_of("/\\") + 1);
			texture->name = (name != "") ? name : "tex" + std::to_string(textures_loaded.size());
		}

		texture->width = tex_width;
		texture->height = tex_height;
		texture->path = str.C_Str();
		texture->type = typeName;
		textures_loaded.push_back(texture);

		return textures_loaded.back();
	}
	return nullptr;
}

unsigned int Model::TextureFromFile(const char* path, int& width, int& height, const std::string& directory)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = GL_RED;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int Model::TextureEmbedded(const aiTexture* texture, int& width, int& height) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	if (texture->mHeight == 0) {
		int nrComponents;
		unsigned char* data = stbi_load_from_memory((unsigned char*)texture->pcData, texture->mWidth / sizeof(unsigned char), &width, &height, &nrComponents, 0);
		if (data) {
			GLenum format = GL_RED;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			printf("Failed to load from memory: %s\n", stbi_failure_reason());
		}
	}
	else {
		width = texture->mWidth; height = texture->mHeight;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->mWidth, texture->mHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, texture->pcData);
	}
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return textureID;
}
