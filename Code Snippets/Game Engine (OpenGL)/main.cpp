#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <gl/GL.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <Windows.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 

#include <fstream>
#include <string>
#include <vector>

#include <assimp direct/assimp-5.0.1/include/assimp/Importer.hpp>
#include <assimp direct/assimp-5.0.1/include/assimp/scene.h>
#include <assimp direct/assimp-5.0.1/include/assimp/postprocess.h>


#include <ft2build.h>
#include FT_FREETYPE_H


std::string command;

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0f / 2.0;
float fov = 45.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

class Shader {
public:
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;

		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vertexPath, NULL);
		glCompileShader(vertex);

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fragmentPath, NULL);
		glCompileShader(fragment);

		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		};
		unsigned int geometry;
		if (geometryPath != nullptr) {
			const char* gShaderCode = geometryCode.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
		}

		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		if (geometryPath != nullptr)
			glAttachShader(ID, geometry);
		glLinkProgram(ID);

		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
	}
	void use() {
		glUseProgram(ID);
	}
	void setBool(const std::string& name, bool value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setVec2(const std::string& name, const glm::vec2& value) const {
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string& name, float x, float y) const {
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	void setVec3(const std::string& name, const glm::vec3& value) const {
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const {
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	void setVec4(const std::string& name, const glm::vec4& value) const {
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w) const {
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	void setMat2(const std::string& name, const glm::mat2& mat) const {
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat3(const std::string& name, const glm::mat3& mat) const {
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat4(const std::string& name, const glm::mat4& mat) const {
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
};

//Mesh import

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};
struct Texture {
	unsigned int id;
	string type;
	string path;
};
int numMeshTri;
class Mesh {
public:
	vector<Vertex>	vertices;
	vector<unsigned int>	indices;
	vector<Texture>	textures;
	unsigned int VAO;

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures) {
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		setupMesh();
	}
	void Draw(Shader& shader) {
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		for (unsigned int i = 0; i < textures.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			string number;
			string name = textures[i].type;
			if (name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if (name == "texture_specular")
				number = std::to_string(specularNr++);
			else if (name == "texture_normal")
				number = std::to_string(normalNr++);
			else if (name == "texture_height")
				number = std::to_string(heightNr++);

			glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glBindVertexArray(GL_TEXTURE0);
	}
private:
	unsigned int VBO, EBO;

	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		numMeshTri = indices.size();
		//vertex pos
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		//vertex norms
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		//vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

		//vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

		//vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		glBindVertexArray(0);
	}

};
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false) {
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
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
	else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
class Model {
public:
	vector<Texture> textures_loaded;
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;
	Model(string const &path) {
		loadModel(path);
	}
	void Draw(Shader& shader) {
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}
private:
	void loadModel(string path) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);


		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));

		processNode(scene->mRootNode, scene);
	}
	void processNode(aiNode* node, const aiScene* scene) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}
	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			glm::vec3 vector;

			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;

			if (mesh->HasNormals()) {
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			if (mesh->mTextureCoords[0]) {
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;

				vector.x = mesh->mTangents[i].x;
				vector.y = mesh->mTangents[i].y;
				vector.z = mesh->mTangents[i].z;
				vertex.Tangent = vector;

				vector.x = mesh->mBitangents[i].x;
				vector.y = mesh->mBitangents[i].y;
				vector.z = mesh->mBitangents[i].z;
				vertex.Bitangent = vector;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		//height and ambient

		vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_height");
		textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

		vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

		return Mesh(vertices, indices, textures);

	}
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName) {
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);

			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip) {
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				texture.type = typeName;
				texture.path = str.C_Str();
				textures.push_back(texture);
				textures_loaded.push_back(texture);
			}
		}
		return textures;
	}
};

int checker = 0;

void getFPS(int shower) {
	static float framespersecond = 0.0f;
	static float lasttime = 0.0f;
	float currenttime = GetTickCount() * 0.001f;
	framespersecond++;
	if (currenttime - lasttime > 1.0f) {
		lasttime = currenttime;
		if (checker == 1) {
			cout << (int)framespersecond << " FPS" << endl;
			checker = 0;
		}
		framespersecond = 0;
	}
}

int numobjects = 0;
int currFPS;
int TriCount = 0;

int spawncheck = 0;

string levelcheck;
int leveltemp = 0;
int skytemp = 0;


float spawnx, spawny, spawnz = 0.0f;
glm::mat4 GetViewMatrix() {
	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

unsigned int loadCubemap(vector<std::string> faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else {
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
//________________________________________________________________________________________________________________________________________________
int main() {

	string str;
	//ofstream editorfile("LevelEditor.sak");
	ifstream editor("LevelEditor.sak");
	glm::vec3 fromfile[] = {glm::vec3(0.0f,-2.0f,5.0f)};
	while (getline(editor, str)) {
		if (str == "~floorcreate{" || str == "	};") {
			numobjects = numobjects;
		}
		else {
			//cout << str << endl;
			numobjects++;
		}	
	}	

	int amountfile = numobjects;

	//glm::vec3 fromfile[amountfile] = {};
	//cout << numobjects << endl;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Sakura Engine", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glViewport(0, 0, 800, 600);

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	vector<std::string> faces{
		"right.jpg",
		"left.jpg",
		"top.jpg",
		"bottom.jpg",
		"front.jpg",
		"back.jpg"
	};

	unsigned int cubemapTexture = loadCubemap(faces);

	float vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	unsigned int indices[] = {
		0,1,3,
		1,2,3
	};
	glm::vec3 cube2pos[] = {
		glm::vec3(0.0f, -2.0f, 5.0f),
		glm::vec3(3.0f, -1.0f, 5.0f)
	};

	glm::vec3 cubePositions[] = {
	glm::vec3(0.0f,  -2.0f,  5.0f),
	glm::vec3(1.0f, -2.0f, 5.0f),
	glm::vec3(2.0f, -2.0f, 5.0f),
	glm::vec3(3.0f, -2.0f, 5.0f),
	glm::vec3(-1.0f, -2.0f, 5.0f),
	glm::vec3(-1.0f, -3.0f, 5.0f),
	glm::vec3(-1.0f, -3.0f, 4.0f),
	glm::vec3(-1.0f, -3.0f, 3.0f),
	glm::vec3(-1.0f, -3.0f, 2.0f),
	glm::vec3(-1.0f, -3.0f, 1.0f),
	glm::vec3(-1.0f, -3.0f, 0.0f),
	glm::vec3(-1.0f, -2.0f, 4.0f),
	glm::vec3(-1.0f, -2.0f, 3.0f),
	glm::vec3(-1.0f, -2.0f, 2.0f),
	glm::vec3(-1.0f, -2.0f, 1.0f),
	glm::vec3(-1.0f, -2.0f, 0.0f),
	glm::vec3(-1.0f, -2.0f, -1.0f),
	glm::vec3(-1.0f, -2.0f, -2.0f),
	glm::vec3(-1.0f, -2.0f, -3.0f),
	glm::vec3(0.0f, -2.0f, -3.0f),
	glm::vec3(1.0f, -2.0f, -3.0f),
	glm::vec3(2.0f, -2.0f, -3.0f),
	glm::vec3(4.0f, -2.0f, -3.0f),
	glm::vec3(4.0f, -2.0f, -2.0f),
	glm::vec3(4.0f, -2.0f, -1.0f),
	glm::vec3(4.0f, -2.0f, 0.0f),
	glm::vec3(4.0f, -2.0f, 1.0f),
	glm::vec3(4.0f, -2.0f, 2.0f),
	glm::vec3(4.0f, -2.0f, 3.0f),
	glm::vec3(4.0f, -2.0f, 4.0f),
	glm::vec3(4.0f, -2.0f, 5.0f),
	glm::vec3(4.0f, -3.0f, 5.0f),
	glm::vec3(4.0f, -3.0f, 4.0f),
	glm::vec3(4.0f, -3.0f, 3.0f),
	glm::vec3(4.0f, -3.0f, 2.0f),
	glm::vec3(4.0f, -3.0f, 1.0f),
	glm::vec3(4.0f, -3.0f, 0.0f),
	glm::vec3(4.0f, -3.0f, -1.0f),
	glm::vec3(4.0f, -3.0f, -2.0f),
	glm::vec3(4.0f, -3.0f, -3.0f),
	glm::vec3(3.0f, -3.0f, -3.0f),
	glm::vec3(2.0f, -3.0f, -3.0f),
	glm::vec3(1.0f, -3.0f, -3.0f),
	glm::vec3(0.0f, -3.0f, -3.0f),
	glm::vec3(-1.0f, -3.0f, -3.0f),
	glm::vec3(-1.0f, -3.0f, -2.0f),
	glm::vec3(-1.0f, -3.0f, -1.0f),
	glm::vec3(0.0f, -3.0f, -1.0f),
	glm::vec3(0.0f, -3.0f, -2.0f),
	glm::vec3(0.0f, -3.0f, 0.0f),
	glm::vec3(0.0f, -3.0f, 1.0f),
	glm::vec3(0.0f, -3.0f, 2.0f),
	glm::vec3(0.0f, -3.0f, 3.0f),
	glm::vec3(0.0f, -3.0f, 4.0f),
	glm::vec3(0.0f, -3.0f, 5.0f),
	glm::vec3(1.0f, -3.0f, 4.0f),
	glm::vec3(2.0f, -3.0f, 4.0f),
	glm::vec3(3.0f, -3.0f, 4.0f),
	glm::vec3(3.0f, -3.0f, 3.0f),
	glm::vec3(2.0f, -3.0f, 3.0f),
	glm::vec3(1.0f, -3.0f, 3.0f),
	glm::vec3(1.0f, -3.0f, 2.0f),
	glm::vec3(2.0f, -3.0f, 2.0f),
	glm::vec3(3.0f, -3.0f, 2.0f),
	glm::vec3(2.0f, -3.0f, 1.0f),
	glm::vec3(1.0f, -3.0f, 1.0f),
	glm::vec3(1.0f, -3.0f, 0.0f),
	glm::vec3(1.0f, -3.0f, -1.0f),
	glm::vec3(2.0f, -3.0f, 0.0f),
	glm::vec3(2.0f, -3.0f, -1.0f),
	glm::vec3(3.0f, -3.0f, -1.0f),
	glm::vec3(3.0f, -3.0f, 1.0f),
	glm::vec3(3.0f, -3.0f, 0.0f),
	glm::vec3(1.0f, -3.0f, -2.0f),
	glm::vec3(2.0f, -3.0f, -2.0f),
	glm::vec3(3.0f, -3.0f, -2.0f)
	};


	glm::vec3 cube2Positions[] = {
	glm::vec3(3.0f,  -2.0f,  5.0f),
	glm::vec3(4.0f, -2.0f, 5.0f),
	glm::vec3(5.0f, -2.0f, 5.0f),
	glm::vec3(6.0f, -2.0f, 5.0f),
	glm::vec3(2.0f, -2.0f, 5.0f),
	glm::vec3(2.0f, -3.0f, 5.0f),
	glm::vec3(2.0f, -3.0f, 4.0f),
	glm::vec3(2.0f, -3.0f, 3.0f),
	glm::vec3(2.0f, -3.0f, 2.0f),
	glm::vec3(2.0f, -3.0f, 1.0f),
	glm::vec3(2.0f, -3.0f, 0.0f),
	glm::vec3(2.0f, -2.0f, 4.0f),
	glm::vec3(2.0f, -2.0f, 3.0f),
	glm::vec3(2.0f, -2.0f, 2.0f),
	glm::vec3(2.0f, -2.0f, 1.0f),
	glm::vec3(2.0f, -2.0f, 0.0f),
	glm::vec3(2.0f, -2.0f, -1.0f),
	glm::vec3(2.0f, -2.0f, -2.0f),
	glm::vec3(2.0f, -2.0f, -3.0f),
	glm::vec3(3.0f, -2.0f, -3.0f),
	glm::vec3(4.0f, -2.0f, -3.0f),
	glm::vec3(5.0f, -2.0f, -3.0f),
	glm::vec3(7.0f, -2.0f, -3.0f),
	glm::vec3(7.0f, -2.0f, -2.0f),
	glm::vec3(7.0f, -2.0f, -1.0f),
	glm::vec3(7.0f, -2.0f, 0.0f),
	glm::vec3(7.0f, -2.0f, 1.0f),
	glm::vec3(7.0f, -2.0f, 2.0f),
	glm::vec3(7.0f, -2.0f, 3.0f),
	glm::vec3(7.0f, -2.0f, 4.0f),
	glm::vec3(7.0f, -2.0f, 5.0f),
	glm::vec3(7.0f, -3.0f, 5.0f),
	glm::vec3(7.0f, -3.0f, 4.0f),
	glm::vec3(7.0f, -3.0f, 3.0f),
	glm::vec3(7.0f, -3.0f, 2.0f),
	glm::vec3(7.0f, -3.0f, 1.0f),
	glm::vec3(7.0f, -3.0f, 0.0f),
	glm::vec3(7.0f, -3.0f, -1.0f),
	glm::vec3(7.0f, -3.0f, -2.0f),
	glm::vec3(7.0f, -3.0f, -3.0f),
	glm::vec3(6.0f, -3.0f, -3.0f),
	glm::vec3(5.0f, -3.0f, -3.0f),
	glm::vec3(4.0f, -3.0f, -3.0f),
	glm::vec3(3.0f, -3.0f, -3.0f),
	glm::vec3(2.0f, -3.0f, -3.0f),
	glm::vec3(2.0f, -3.0f, -2.0f),
	glm::vec3(2.0f, -3.0f, -1.0f),
	glm::vec3(3.0f, -3.0f, -1.0f),
	glm::vec3(3.0f, -3.0f, -2.0f),
	glm::vec3(3.0f, -3.0f, 0.0f),
	glm::vec3(3.0f, -3.0f, 1.0f),
	glm::vec3(3.0f, -3.0f, 2.0f),
	glm::vec3(3.0f, -3.0f, 3.0f),
	glm::vec3(3.0f, -3.0f, 4.0f),
	glm::vec3(3.0f, -3.0f, 5.0f),
	glm::vec3(4.0f, -3.0f, 4.0f),
	glm::vec3(5.0f, -3.0f, 4.0f),
	glm::vec3(6.0f, -3.0f, 4.0f),
	glm::vec3(6.0f, -3.0f, 3.0f),
	glm::vec3(5.0f, -3.0f, 3.0f),
	glm::vec3(4.0f, -3.0f, 3.0f),
	glm::vec3(4.0f, -3.0f, 2.0f),
	glm::vec3(5.0f, -3.0f, 2.0f),
	glm::vec3(6.0f, -3.0f, 2.0f),
	glm::vec3(5.0f, -3.0f, 1.0f),
	glm::vec3(4.0f, -3.0f, 1.0f),
	glm::vec3(4.0f, -3.0f, 0.0f),
	glm::vec3(4.0f, -3.0f, -1.0f),
	glm::vec3(5.0f, -3.0f, 0.0f),
	glm::vec3(5.0f, -3.0f, -1.0f),
	glm::vec3(6.0f, -3.0f, -1.0f),
	glm::vec3(6.0f, -3.0f, 1.0f),
	glm::vec3(6.0f, -3.0f, 0.0f),
	glm::vec3(4.0f, -3.0f, -2.0f),
	glm::vec3(5.0f, -3.0f, -2.0f),
	glm::vec3(6.0f, -3.0f, -2.0f)
	};

	glm::vec3 cube3Positions[] = {
	glm::vec3(3.0f,-2.0f,5.0f),
	glm::vec3(3.0f, 0.0f, 5.0f),
	glm::vec3(4.0f, -2.0f, 5.0f),
	glm::vec3(5.0f, -2.0f, 5.0f),
	glm::vec3(3.0f,-1.0f,5.0f),
	glm::vec3(4.0f, -1.0f, 5.0f),
	glm::vec3(5.0f, -1.0f, 5.0f),
	glm::vec3(3.0f, -2.0f, 4.0f),
	glm::vec3(3.0f, -2.0f, 3.0f),
	glm::vec3(3.0f, -2.0f, 2.0f),
	glm::vec3(2.0f, -2.0f, 2.0f),
	glm::vec3(1.0f,-2.0f,2.0f),
	glm::vec3(0.0f,-2.0f,2.0f),
	glm::vec3(-1.0f,-2.0f,2.0f),
	glm::vec3(-2.0f,-2.0f,2.0f),
	glm::vec3(-3, -2, 2),
	glm::vec3(-4,-2,2),
	glm::vec3(-4,-1,2),
	glm::vec3(3,-1,2),
	glm::vec3(3,0,2),
	glm::vec3(2,-2,2),
	glm::vec3(4,-2,5),
	glm::vec3(4,-2,4),
	glm::vec3(4,-2,3),
	glm::vec3(4,-2,2),
	glm::vec3(4,-2,1),
	glm::vec3(5,-2,4),
	glm::vec3(5,-2,3),
	glm::vec3(5,-2,2),
	glm::vec3(3,-2,2),
	glm::vec3(5,-2,1),
	glm::vec3(2,-2,5),
	glm::vec3(1,-2,5),
	glm::vec3(0,-2,5),
	glm::vec3(-1,-2,5),
	glm::vec3(-2,-2,5),
	glm::vec3(-3,-2,5),
	glm::vec3(-4,-2,5),
	glm::vec3(-4,-2,4),
	glm::vec3(-4,-2,3),
	glm::vec3(-3,-2,4),
	glm::vec3(-3,-2,3),
	glm::vec3(-2, -2, 4),
	glm::vec3(-2,-2,3),
	glm::vec3(-1,-2,4),
	glm::vec3(-1,-2,3),
	glm::vec3(0,-2,3),
	glm::vec3(0,-2,4),
	glm::vec3(1,-2,3),
	glm::vec3(1,-2,4),
	glm::vec3(2,-2,4),
	glm::vec3(2,-2,3),
	glm::vec3(4,-2,1),
	glm::vec3(4,-2,0),
	glm::vec3(4,-2,-1),
	glm::vec3(5,-2,0),
	glm::vec3(5,-2,-1),
	glm::vec3(3,-2,1),
	glm::vec3(3,-2,0),
	glm::vec3(3,-2,-1),
	glm::vec3(3,-2,-2),
	glm::vec3(4,-2,-2),
	glm::vec3(5,-2,-2),
	glm::vec3(6,-2,-2),
	glm::vec3(6,-2,-1),
	glm::vec3(6,-2,0),
	glm::vec3(6,-2,1),
	glm::vec3(6,-2,2),
	glm::vec3(6,-2,3),
	glm::vec3(6,-2,4),
	glm::vec3(6,-2,5),
	glm::vec3(6,-1,5),
	glm::vec3(6,-1,4),
	glm::vec3(6,-1,3),
	glm::vec3(6,-1,2),
	glm::vec3(6,-1,1),
	glm::vec3(6,-1,0),
	glm::vec3(6,-1,-1),
	glm::vec3(6,-1,-2),
	glm::vec3(5,-1,-2),
	glm::vec3(4,-1,-2),
	glm::vec3(3,-1,-2),
	glm::vec3(3,-1,-1),
	glm::vec3(3,-1,0),
	glm::vec3(3,-1,-1),
	glm::vec3(3,0,-1),
	glm::vec3(3,0,-2),
	glm::vec3(4,0,-2),
	glm::vec3(3,0,0),
	glm::vec3(3,-1,0),
	glm::vec3(-3,-1,5),
	glm::vec3(-4,-1,5),
	glm::vec3(-2,-1,5),
	glm::vec3(-1,-1,5),
	glm::vec3(0,-1,5),
	glm::vec3(1,-1,5),
	glm::vec3(2,-1,5),
	glm::vec3(2,-2,5),
	glm::vec3(1,-2,5),
	glm::vec3(0,-2,5),
	glm::vec3(-1,0,5),
	glm::vec3(-2,0,5),
	glm::vec3(-3,0,5),
	glm::vec3(-4,0,5),
	glm::vec3(0,0,5),
		glm::vec3(1,0,5),
		glm::vec3(2,0,5),
		glm::vec3(4,0,5),
		glm::vec3(5,0,5),
		glm::vec3(6,0,5),
	};

	unsigned int VBO;
	unsigned int VAO;
	unsigned int EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	unsigned int VAO2;
	unsigned int VBO2;
	unsigned int EBO2;
	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);

	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	//_______________________________________________________________________________________________________________ Lights
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	const char* skyVertexSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"out vec3 TexCoords;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"void main(){\n"
		"TexCoords = aPos;\n"
		"vec4 pos = projection * view * vec4(aPos, 1.0);\n"
		"gl_Position = pos.xyww;\n"
		"}";

	const char* skyFragmentSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 TexCoords;\n"
		"uniform samplerCube skybox;\n"
		"void main(){\n"
		"FragColor = texture(skybox, TexCoords);\n"
		"}";

	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aColor;\n"
		"layout (location = 2) in vec2 aTexCoord;\n"
		"out vec2 TexCoord;\n"
		"out vec3 ourColor;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main(){\n"
		"gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"ourColor = aColor;\n"
		"TexCoord = vec2(aTexCoord.x,  aTexCoord.y);\n"
		"}\0";

	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 ourColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D ourTexture;\n"
		"void main(){\n"
		"FragColor = texture(ourTexture, TexCoord);\n"
		"}\0";

	const char* wallfrag = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 ourColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D ourTexture2;\n"
		"void main(){\n"
		"FragColor = texture(ourTexture2, TexCoord);\n"
		"}\0";

	const char* colorVS = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aNormal;\n"
		"out vec3 FragPos;\n"
		"out vec3 Normal;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main(){\n"
		"FragPos = vec3(model * vec4(aPos,1.0));\n"
		"Normal = aNormal;\n"
		"gl_Position = projection * view * vec4(FragPos, 1.0);\n"
		"}\0";

	const char* colorFS = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec3 Normal;\n"
		"in vec3 FragPos;\n"
		"uniform vec3 lightPos;\n"
		"uniform vec3 lightColor;\n"
		"uniform vec3 objectColor;\n"
		"void main(){\n"
		"float ambientStrength = 0.1;\n"
		"vec3 ambient = ambientStrength * lightColor;\n"
		"vec3 norm = normalize(Normal);\n"
		"vec3 lightDir = normalize(lightPos - FragPos);\n"
		"float diff = max(dot(norm, lightDir), 0.0);\n"
		"vec3 diffuse = diff * lightColor;\n"
		"vec3 result = (ambient + diffuse) * objectColor;\n"
		"FragColor = vec4(result, 1.0);\n"
		"}\0";

	const char* lightCubeVS = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main(){\n"
		"gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}\0";

	const char* lightCubeFS = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D ourtexture;\n"
		"void main(){\n"
		"FragColor = texture(ourtexture, TexCoord);\n"
		"}\0";

	const char* importVS = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aNormal;\n"
		"layout (location = 2) in vec2 aTexCoords;\n"
		"out vec2 TexCoords;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main(){\n"
		"TexCoords = aTexCoords;\n"
		"gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
		"}\0";

	const char* importFS = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoords;\n"
		"uniform sampler2D texture_diffuse1;\n"
		"void main(){\n"
		"FragColor = texture(texture_diffuse1, TexCoords);\n"
		"}\0";


	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();

	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);

	unsigned int skyvertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(skyvertexShader, 1, &skyVertexSource, NULL);

	glCompileShader(skyvertexShader);

	unsigned int skyfragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(skyfragShader, 1, &skyFragmentSource, NULL);
	glCompileShader(skyfragShader);

	unsigned int skyboxProgram;
	skyboxProgram = glCreateProgram();

	glAttachShader(skyboxProgram, skyvertexShader);
	glAttachShader(skyboxProgram, skyfragShader);
	glLinkProgram(skyboxProgram);

	glUseProgram(skyboxProgram);


	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	Shader ourShader(vertexShaderSource, fragmentShaderSource);

	Shader lightingShader(colorVS, colorFS);
	Shader lightingCubeShader(vertexShaderSource, fragmentShaderSource);

	Shader wallshader(colorVS, wallfrag);

	Shader importShader(importVS, importFS);
	Shader skyboxShader(skyVertexSource, skyFragmentSource);

	int width, height, nrChannels;
	unsigned char* data = stbi_load("floor.png", &width, &height, &nrChannels, 0);
	//unsigned char* data2 = stbi_load("floor.jpg", &width, &height, &nrChannels, 0);
	

	unsigned int texture, texture2;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	
	ourShader.use();
	lightingShader.use();
	wallshader.use();
	lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
	lightingShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
	lightingShader.setVec3("lightPos", lightPos);

	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glUniform1i(glGetUniformLocation(ourShader.ID, "ourTexture"), 0);
	ourShader.setInt("ourTexture", 1);

	glUniform1i(glGetUniformLocation(wallshader.ID, "ourTexture2"), 0);
	wallshader.setInt("ourTexture2", 1);

	importShader.use();
	glUniform1i(glGetUniformLocation(importShader.ID, "texture_diffuse1"), 1);
	importShader.setInt("texture_diffuse1", 1);


	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
	trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));

	glm::mat4 view = glm::mat4(1.0f);
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));

	glEnable(GL_DEPTH_TEST);

	//Model ourmodel("Survival_BackPack_2.fbx");
	Model ourmodel = Model("BeefBuildingMedium.fbx");

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	while (!glfwWindowShouldClose(window)) {
		
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glUseProgram(shaderProgram);
		glUseProgram(skyboxProgram);
		ourShader.use();

		lightingShader.use();
		lightingShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
		lightingShader.setVec3("lightColor", 1.0f,1.0f,1.0f);
		lightingShader.setVec3("lightPos", lightPos);
		
		float timeValue = glfwGetTime();
		float greenValue = sin(timeValue) / 2.0f + 0.5f;

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		getFPS(currFPS);

		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
		lightingShader.setMat4("projection", projection);
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);

		//glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
		glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));

		glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

		glm::mat4 view;

		float radius = 10.0f;
		float camX = sin(glfwGetTime()) * radius;
		float camZ = cos(glfwGetTime()) * radius;
		ourShader.setMat4("view", view);

		view = GetViewMatrix();
		lightingShader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::rotate(model, glm::radians(-10.0f), glm::vec3(1.0f, 0.3f, 0.5f));
		lightingShader.setMat4("model", model);
		

		unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
		//glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform));
		int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		int projLoc = glGetUniformLocation(ourShader.ID, "projection");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
		//glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
		//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

		int doorloc = glGetUniformLocation(wallshader.ID, "model");
		int viewdloc = glGetUniformLocation(wallshader.ID, "view");
		int projdloc = glGetUniformLocation(wallshader.ID, "projection");
		glUniformMatrix4fv(doorloc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewdloc, 1, GL_FALSE, &view[0][0]);


		int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
		glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(VAO);

		//the orange light box
		if (leveltemp == 1) {
			for (unsigned int i = 0; i < 20; i++) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, cubePositions[i]);
				ourShader.setMat4("model", model);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
		}
		
			//ourShader.setMat4("model", model);

		//glm::mat4 model2 = glm::mat4(1.0f);
		lightingCubeShader.use();
		lightingCubeShader.setMat4("projection", projection);
		lightingCubeShader.setMat4("view", view);


		/*glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 76; i++) {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			ourShader.setMat4("model", model);
			int j = sizeof(cubePositions);
			glDrawArrays(GL_TRIANGLES, 0, 36);

			TriCount = j * 12;
			TriCount += numMeshTri;
		}*/

		if (leveltemp == 1) {

			glBindVertexArray(VAO);
			for (unsigned int i = 0; i < 76; i++) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, cubePositions[i]);
				ourShader.setMat4("model", model);
				int j = sizeof(cubePositions);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				TriCount = j * 12;
				TriCount += numMeshTri;
			}
		}
		else if (leveltemp == 2) {
			glBindVertexArray(VAO);
			for (unsigned int i = 0; i < 76; i++) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, cube2Positions[i]);
				ourShader.setMat4("model", model);
				int j = sizeof(cube2Positions);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				TriCount = j * 12;
				TriCount += numMeshTri;
			}
		}
		else if (leveltemp == 3) {
			glBindVertexArray(VAO);
			for (unsigned int i = 0; i < 110; i++) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, cube3Positions[i]);
				ourShader.setMat4("model", model);
				int j = sizeof(cube3Positions);
				glDrawArrays(GL_TRIANGLES, 0, 36);

				TriCount = j * 12;
				TriCount += numMeshTri;
			}
		}

		importShader.use();
		importShader.setMat4("projection", projection);
		importShader.setMat4("view", view);
		if (spawncheck == 1) {
			glBindVertexArray(VAO);
			for (unsigned int i = 0; i < 1; i++) {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(spawnx, spawny, spawnz));
				model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
				importShader.setMat4("model", model);
				ourmodel.Draw(importShader);

				TriCount += numMeshTri;

				//cout << "spawn locations" << spawnx << spawny << spawnz << endl;
			}
		}

		glBindVertexArray(lightCubeVAO);
		for (unsigned int i = 0; i < 1; i++) {
			glm::mat4 door = glm::mat4(1.0f);
			door = glm::translate(door, glm::vec3(3.0f, -2.0f, -3.0f));
			ourShader.setMat4("model", model);
			//glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		view = glm::mat4(glm::mat3(GetViewMatrix()));
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);


		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		//glDepthMask(GL_TRUE);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		/*if (skytemp == 1) {
			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			glDepthMask(GL_TRUE);
			glBindVertexArray(0);
			glUseProgram(skyboxProgram);
		} */
		/*else if (skytemp == 0) {
			glBindVertexArray(skyboxVAO);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			//glDrawArrays(GL_TRIANGLES, 0, 36);
			glBindVertexArray(0);
			glUseProgram(shaderProgram);
		}*/



		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteProgram(shaderProgram);

	glfwTerminate();
	return 0;
}
int console = 0;

void character_callback(GLFWwindow* window, unsigned int codepoint) {
	std::cout << (char)codepoint;
	command += (char)codepoint;
}

std::vector<std::string> parse(std::string s) {

	s += ' ';
	std::vector<std::string> words;
	std::string word;

	for (auto c : s) {
		if (c == ' ') {
			words.push_back(word);
			word = "";
		}
		else {
			word += c;
		}
	}

	for (auto w : words) {
		std::cout << w << endl;
	}

	return words;
}
void consoleCommand(std::vector<std::string> commands) {
	//std::cout << "COMMAND: " << commands[0] << std::endl;
	if (commands[0] == "fps") {
		checker = 1;
		//std::cout << "CURRENT FPS: " << currFPS <<endl;
	}
	else if (commands[0] == "triangles") {
		std::cout << "CURRENT TRIANGLE COUNT: " << TriCount << endl;
	}
	else if (commands[0] == "loadlevel") {
		std::cout << "LOADING LEVEL: " << commands[1] << "..." << endl;
		levelcheck = commands[1];
		if (levelcheck == "example.lvl") {
			leveltemp = 1;
		}
		else if (levelcheck == "changeTester.lvl") {
			leveltemp = 2;
		}
		else if (levelcheck == "engineer.lvl") {
			leveltemp = 3;
		}
		else if (levelcheck == "auction.lvl") {
			leveltemp = 1;
		}
		else if (levelcheck == "grain.lvl") {
			leveltemp = 2;
		}
		else if (levelcheck == "sticky.lvl") {
			leveltemp = 3;
		}
		else if (levelcheck == "area.lvl") {
			leveltemp = 1;
		}
		else if (levelcheck == "laboratory.lvl") {
			leveltemp = 2;
		}
		else if (levelcheck == "spray.lvl") {
			leveltemp = 3;
		}
		else if (levelcheck == "card.lvl") {
			leveltemp = 1;
		}
		else {
			cout << "Level not found/does not exist" << endl;
		}
	}
	else if (commands[0] == "spawn") {
	
		std::cout << "SPAWNING: " << commands[1] << " AT(" << commands[2] << "," << commands[3] << ", " << commands[4] << ")" << endl;
		if (commands[1] == "BeefBuildingMedium.FBX" && commands[2].empty() != true && commands[3].empty() != true && commands[4].empty() != true) {
			spawncheck = 1;
			spawnx = std::stof(commands[2]);
			spawny = std::stof(commands[3]);
			spawnz = std::stof(commands[4]);
		}
		else if (commands[2].empty() || commands[3].empty() || commands[4].empty()) {
			cout << "Position parameters are missing, please enter the position for " << commands[1] << " to spawn in the format:" << endl;
			cout << commands[1] << " X Y Z" << endl;
		}
		else if (commands[1].empty()) {
			cout << "Please specify an object to spawn. Use the format:" << endl;
			cout << "ObjectName X Y Z" << endl;
		}
		else {
			cout << "ERROR::SPAWNING::OBJNOTFOUND\nThe object you are trying to spawn does not exist.\nDid you spell it correctly?" << endl;
		}

	}
	else if (commands[0] == "skyenable") {
		skytemp = 1;
	}
	else if (commands[0] == "skydisable") {
		skytemp = 0;
	}
	else if (commands[0] == "checkpos") {
		cout << cameraPos.x << " " << cameraPos.y << " " << cameraPos.z << endl;
	}
	else {
		cout << "ERROR::COMMAND::CMDNOTFOUND\nThe command you have entered does not exist." << endl;
	}
}

void processInput(GLFWwindow* window) {

	const float cameraSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	
	//_________console
	if (!console) {
		if (glfwGetKey(window, GLFW_KEY_GRAVE_ACCENT) == GLFW_PRESS) {
			std::cout << "CONSOLE MODE ACTIVATED" << std::endl;
			console = 1;
			glfwSetCharCallback(window, character_callback);
		}
	}

	if (console) {

		if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
			std::cout << std::endl << "COMMAND ENTERED " << command << std::endl;
			console = 0;
			//CALL CONSOLE COMMANDS
			glfwSetCharCallback(window, NULL);
			consoleCommand(parse(command));
			command = "";
		}
	}

	//_________
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset += xoffset;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	fov -= (float)yoffset;
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}