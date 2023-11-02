#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.h"
#include "mesh.h"

#include <iostream>
#include <string>
#include <vector>

class Model
{

public:
	void Draw();
	void DrawDepth();
	static std::shared_ptr<Model> loadModel(std::string path);
	void renderUI();
	static void renderLoadInfoUI();


	// don't use this one to instantiate
	Model(std::string path, std::string directory, const aiScene* scene);

private:
	// model data
	static std::vector<std::shared_ptr<Model>> models;
	static std::vector<std::shared_ptr<Material>> materials;
	static std::vector<std::shared_ptr<Texture>> textures_loaded;

	std::string path;
	std::string directory;
	std::string name;
	std::vector<Mesh> meshes;

	static int loadMeshClicked; 
	int selectedMeshIndex = -1;

	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::shared_ptr<Texture> loadMaterialTexture(const aiScene* scene, aiMaterial* mat, aiTextureType type, std::string typeName);
	unsigned int TextureFromFile(const char* path, int& width, int& height, const std::string& directory);
	unsigned int TextureEmbedded(const aiTexture* texture, int& width, int& height);
};
#endif