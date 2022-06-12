#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include "allmodels.h"
#include "lodepng.h"
#include "shaderprogram.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

float speed_x = 0;//[radiany/s]
float speed_y = 0;//[radiany/s]
float speed_walk = 0;
float aspectRatio = 1;

int flagDrink = 0;

glm::mat4 V, P;

ShaderProgram* sp;

GLuint tex;
GLuint texWall0, texFloor, texCeiling;
GLuint texWoodenDoor, texTemp;
GLuint texVodka, texCounter, texKangaroo, texZubr, texAmarena, texHarnas, texWino;


float* verticesCube = myCubeVertices;
float* normalsCube = myCubeNormals;
float* texCoordsCube = myCubeTexCoords;
float* colorsCube = myCubeColors;
int vertexCountCube = myCubeVertexCount;

glm::vec3 pos = glm::vec3(0, 1, -1);

std::vector<glm::vec4> verts;
std::vector<glm::vec4> norms;
std::vector<glm::vec2> texCoords;
std::vector<unsigned int> indices;

std::vector<glm::vec4> verts1;
std::vector<glm::vec4> norms1;
std::vector<glm::vec2> texCoords1;
std::vector<unsigned int> indices1;

std::vector<glm::vec4> verts2;
std::vector<glm::vec4> norms2;
std::vector<glm::vec2> texCoords2;
std::vector<unsigned int> indices2;

std::vector<glm::vec4> verts3;
std::vector<glm::vec4> norms3;
std::vector<glm::vec2> texCoords3;
std::vector<unsigned int> indices3;

glm::vec3 computeDir(float kat_x, float kat_y) {
	glm::vec4 dir(0, 0, 1, 0);
	glm::mat4 M = glm::mat4(1.0f);
	M = glm::rotate(M, kat_y, glm::vec3(0, 1, 0));
	M = glm::rotate(M, kat_x, glm::vec3(1, 0, 0));
	dir = M * dir;
	return glm::vec3(dir);
}


void key_callback(
	GLFWwindow* window,
	int key,
	int scancode,
	int action,
	int mod
) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_LEFT) speed_y = 1;
		if (key == GLFW_KEY_RIGHT) speed_y = -1;
		if (key == GLFW_KEY_PAGE_UP) speed_x = -1;
		if (key == GLFW_KEY_PAGE_DOWN) speed_x = 1;
		if (key == GLFW_KEY_UP) speed_walk = 4;
		if (key == GLFW_KEY_DOWN) speed_walk = -2;
		if (key == GLFW_KEY_D) flagDrink = 1;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_y = 0;
		if (key == GLFW_KEY_RIGHT) speed_y = 0;
		if (key == GLFW_KEY_UP) speed_walk = 0;
		if (key == GLFW_KEY_DOWN) speed_walk = 0;
		if (key == GLFW_KEY_PAGE_UP) speed_x = 0;
		if (key == GLFW_KEY_PAGE_DOWN) speed_x = 0;
		if (key == GLFW_KEY_D) flagDrink = -1;
	}
}


void windowResizeCallback(GLFWwindow* window, int width, int height) {
	if (height == 0) return;
	aspectRatio = (float)width / (float)height;
	glViewport(0, 0, width, height);
}


//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}


GLuint readTexture(const char* filename) {
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);

	//Wczytanie do pamięci komputera
	std::vector<unsigned char> image;   //Alokuj wektor do wczytania obrazka
	unsigned width, height;   //Zmienne do których wczytamy wymiary obrazka
	//Wczytaj obrazek
	unsigned error = lodepng::decode(image, width, height, filename);

	//Import do pamięci karty graficznej
	glGenTextures(1, &tex); //Zainicjuj jeden uchwyt
	glBindTexture(GL_TEXTURE_2D, tex); //Uaktywnij uchwyt
	//Wczytaj obrazek do pamięci KG skojarzonej z uchwytem
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	return tex;
}


//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	initShaders();
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0, 0, 0, 1); //Ustaw kolor czyszczenia bufora kolorów
	glEnable(GL_DEPTH_TEST); //Włącz test głębokości na pikselach
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, key_callback);

	sp = new ShaderProgram("v_lamberttextured.glsl", NULL, "f_lamberttextured.glsl");

	texWall0 = readTexture("pics/white-brick.png");
	texFloor = readTexture("pics/floor.png");
	texCeiling = readTexture("pics/ceiling.png");
	texWoodenDoor = readTexture("pics/wooden_door.png");
	texTemp = readTexture("pics/floor_1.png");
	texCounter = readTexture("pics/aditya-joshi-wG923J9naFQ-unsplash.png");
	texKangaroo = readTexture("pics/Kangaroo_texture.png");
	texVodka = readTexture("pics/zubrowka.png");
	texZubr = readTexture("pics/zubr.png");
	texHarnas = readTexture("pics/harnas.png");
	texAmarena = readTexture("pics/amarena.png");
	texWino = readTexture("pics/wino.png");

}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	freeShaders();
	//************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
	glDeleteTextures(1, &texWall0);
}



void texModel(std::vector<glm::vec4> verts,
	std::vector<glm::vec4> norms,
	std::vector<glm::vec2> texCoords,
	std::vector<unsigned int> indices, bool ifTex, GLuint tex)
{

	sp->use();
	//glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P));
	//glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V));
	//glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M));

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, verts.data());

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, texCoords.data());

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, norms.data());

	//glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, tex);
	//glUniform1i(spLambert->u("tex"), 0);
	if (ifTex == true) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glUniform1i(sp->u("tex"), 0);
	}


	//glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());

	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("texCoord"));
	glDisableVertexAttribArray(sp->a("normal"));
}

bool loadModel(string filename, std::vector<glm::vec4> verts,
	std::vector<glm::vec4> norms,
	std::vector<glm::vec2> texCoords,
	std::vector<unsigned int> indices, bool ifTex, GLuint tex)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);
	//cout << importer.GetErrorString() << endl;

	/*if (scene->HasMeshes())
	{
		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			importMesh(scene->mMeshes[i]);
		}
	}*/

	aiMesh* mesh = scene->mMeshes[0];

	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		aiVector3D vertex = mesh->mVertices[i];
		//cout << vertex.x << " " << vertex.y << " " << vertex.z << endl;
		verts.push_back(glm::vec4(vertex.x, vertex.y, vertex.z, 1));

		aiVector3D normal = mesh->mNormals[i];
		//cout << normal.x << " " << normal.y << " " << normal.z << endl;
		norms.push_back(glm::vec4(normal.x, normal.y, normal.z, 0));


		unsigned int number_of_sets = mesh->GetNumUVChannels();
		unsigned int number_of_tex = mesh->mNumUVComponents[0];
		aiVector3D texCoord = mesh->mTextureCoords[0][i];
		texCoords.push_back(glm::vec2(texCoord.x, texCoord.y));

		//cout << texCoord.x << " " << texCoord.y << endl;
	}

	for (int i = 0; i < mesh->mNumFaces; i++) {
		aiFace& face = mesh->mFaces[i];

		for (int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
		//cout << endl;
	}
	texModel(verts, norms, texCoords, indices, ifTex, tex);

	return true;
}


void drawDoor(glm::mat4 M) {
	glm::mat4 MD = glm::translate(M, glm::vec3(0.0f, -0.5f, 0.0f));
	MD = glm::scale(MD, glm::vec3(3.1 / 2, 5.25 / 2, 1.5 / 2));

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(MD));

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, verticesCube); //Współrzędne wierzchołków bierz z tablicy myCubeVertices

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normalsCube);

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, texCoordsCube); //Współrzędne teksturowania bierz z tablicy myCubeTexCoords

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texWoodenDoor);
	glUniform1i(sp->u("tex"), 0);

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("color"));
}

// narysuj podłogę;
void drawFloor(glm::mat4 M) {
	float width = 16, hight = 12, thickness = 0.5;
	glm::mat4 MF = glm::rotate(M, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	MF = glm::translate(MF, glm::vec3(0.0f, 0.0f, 3.0f));
	MF = glm::scale(MF, glm::vec3(width / 2, hight / 2, thickness / 2));

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(MF));

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, verticesCube); //Współrzędne wierzchołków bierz z tablicy myCubeVertices

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normalsCube);

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, texCoordsCube); //Współrzędne teksturowania bierz z tablicy myCubeTexCoords

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texFloor);
	glUniform1i(sp->u("tex"), 0);

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("color"));
}


// narysuj sufit;
void drawCeiling(glm::mat4 M) {
	float width = 16, hight = 12, thickness = 0.5;
	glm::mat4 MF = glm::rotate(M, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	MF = glm::translate(MF, glm::vec3(0.0f, 0.0f, 4.0f));
	MF = glm::scale(MF, glm::vec3(width / 2, hight / 2, thickness / 2));

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(MF));

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, verticesCube); //Współrzędne wierzchołków bierz z tablicy myCubeVertices

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normalsCube);

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, texCoordsCube); //Współrzędne teksturowania bierz z tablicy myCubeTexCoords

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texCeiling);
	glUniform1i(sp->u("tex"), 0);

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("color"));
}


// narysuj ścianę o danej wielkości w przesłanej macierzy
void drawWall(glm::mat4 M, float width, float hight, float thickness) {

	glm::mat4 MW = glm::scale(M, glm::vec3(width / 2, hight / 2, thickness / 2));

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(MW));

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, verticesCube); //Współrzędne wierzchołków bierz z tablicy myCubeVertices

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normalsCube);

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, texCoordsCube); //Współrzędne teksturowania bierz z tablicy myCubeTexCoords

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texWall0);
	glUniform1i(sp->u("tex"), 0);

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("color"));
}


void drawRoom() {
	glm::mat4 MR = glm::mat4(1.0f);

	// narysuj podłogę i sufit
	MR = glm::translate(MR, glm::vec3(0.0f, 0.0f, 2.0f));
	drawFloor(MR);
	drawCeiling(MR);

	// narysuj ściany
	MR = glm::translate(MR, glm::vec3(0.0f, 0.0f, 6.0f));
	drawWall(MR, 16, 8, 0.5);

	MR = glm::translate(MR, glm::vec3(0.0f, 0.0f, -6.0f));
	MR = glm::rotate(MR, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	MR = glm::translate(MR, glm::vec3(0.0f, 0.0f, 8.0f));
	drawWall(MR, 12, 8, 0.5);

	MR = glm::translate(MR, glm::vec3(0.0f, 0.0f, -8.0f));
	MR = glm::rotate(MR, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	MR = glm::translate(MR, glm::vec3(0.0f, 0.0f, 6.0f));
	drawWall(MR, 16, 8, 0.5);
	drawDoor(MR);

	MR = glm::translate(MR, glm::vec3(0.0f, 0.0f, -6.0f));
	MR = glm::rotate(MR, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	MR = glm::translate(MR, glm::vec3(0.0f, 0.0f, 8.0f));
	drawWall(MR, 12, 8, 0.5);

}

void drawTest(glm::mat4 M, float angle);

void drink() {
	cout << "drink\n";
	glm::mat4 MDrink = glm::mat4(1.0f);

	for (int i = 0; i < 10; i++) {
		drawTest(MDrink, glm::radians(i * 90.0));
	}
}

void drawRack(glm::mat4 M, GLuint tex) {
	glm::mat4 M1 = M; //Zainicjuj macierz modelu macierzą jednostkową
	M1 = glm::translate(M1, glm::vec3(-3.5, 0, -10));
	M1 = glm::scale(M1, glm::vec3(0.01, 0.01, 0.01));
	glUniform4f(sp->u("color"), 0.37, 0.22, 0.08, 0.8); //Ustaw kolor rysowania obiektu
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M1)); //Załaduj do programu cieniującego macierz modelu
	loadModel("Models/pinewoodRackFullHeight.fbx", verts1, norms1, texCoords1, indices1, true, texWoodenDoor);

	glm::mat4 MB = M; //Zainicjuj macierz modelu macierzą jednostkową
	MB = glm::translate(MB, glm::vec3(-0.63, 0.15, -6.15));
	MB = glm::rotate(MB, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	MB = glm::scale(MB, glm::vec3(0.001, 0.001, 0.0006));
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniform4f(sp->u("color"), 0.68, 0.68, 0.68, 0.7); //Ustaw kolor rysowania obiektu
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(MB)); //Załaduj do programu cieniującego macierz modelu
	loadModel("Models/Bottle.fbx", verts2, norms2, texCoords2, indices2, true, tex);
	int x = 130;
	int m = 0;
	for (int i = 1; i <= 5; i++)
	{
		if (i % 2 == 0)
			x = -130;
		else
			x = 130;

		if (i > 1)
		{
			MB = glm::translate(MB, glm::vec3(0, 0, 460 + m));
			glUniform4f(spLambert->u("color"), 0.68, 0.68, 0.68, 0.7); //Ustaw kolor rysowania obiektu
			glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
			glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
			glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(MB)); //Załaduj do programu cieniującego macierz modelu
			loadModel("Models/Bottle.fbx", verts2, norms2, texCoords2, indices2, true, tex);
		}

		for (int j = 1; j <= 5; j++)
		{
			MB = glm::translate(MB, glm::vec3(x, 0, 0));
			glUniform4f(sp->u("color"), 0.68, 0.68, 0.68, 0.7); //Ustaw kolor rysowania obiektu
			glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
			glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
			glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MB));
			//texModel(verts2, norms2, texCoords2, indices2);
			loadModel("Models/Bottle.fbx", verts2, norms2, texCoords2, indices2, true, tex);
		}
		m += 32;
	}


	MB = glm::translate(MB, glm::vec3(0, 0, 530));
	glUniform4f(spLambert->u("color"), 0.68, 0.68, 0.68, 0.7); //Ustaw kolor rysowania obiektu
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(MB)); //Załaduj do programu cieniującego macierz modelu
	loadModel("Models/Bottle.fbx", verts2, norms2, texCoords2, indices2, true, tex);

	for (int j = 1; j <= 5; j++)
	{
		MB = glm::translate(MB, glm::vec3(-130, 0, 0));
		glUniform4f(sp->u("color"), 0.68, 0.68, 0.68, 0.7); //Ustaw kolor rysowania obiektu
		glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
		glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
		glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MB));
		//texModel(verts2, norms2, texCoords2, indices2);
		loadModel("Models/Bottle.fbx", verts2, norms2, texCoords2, indices2, true, tex);
	}

}

void drawTest(glm::mat4 M, float angle)
{
	glm::mat4 MTest = glm::mat4(1.0f);
	MTest = glm::rotate(MTest, glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f));
	MTest = glm::translate(MTest, glm::vec3(0.0f, -1.5f, 0.0f));

	sp->use();
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(MTest));

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, verticesCube); //Współrzędne wierzchołków bierz z tablicy myCubeVertices

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normalsCube);

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, texCoordsCube); //Współrzędne teksturowania bierz z tablicy myCubeTexCoords

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texTemp);
	glUniform1i(sp->u("tex"), 0);

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("color"));
}

void drawCounter() {
	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
	glm::mat4 MCounter = glm::mat4(1.0f);
	MCounter = glm::translate(MCounter, glm::vec3(0.0f, -1.5f, 3.0f));
	MCounter = glm::scale(MCounter, glm::vec3(1.5, 1.3, 0.5));


	sp->use();
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(MCounter));

	glEnableVertexAttribArray(sp->a("vertex"));
	glVertexAttribPointer(sp->a("vertex"), 4, GL_FLOAT, false, 0, verticesCube); //Współrzędne wierzchołków bierz z tablicy myCubeVertices

	glEnableVertexAttribArray(sp->a("normal"));
	glVertexAttribPointer(sp->a("normal"), 4, GL_FLOAT, false, 0, normalsCube);

	glEnableVertexAttribArray(sp->a("texCoord"));
	glVertexAttribPointer(sp->a("texCoord"), 2, GL_FLOAT, false, 0, texCoordsCube); //Współrzędne teksturowania bierz z tablicy myCubeTexCoords

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texCounter);
	glUniform1i(sp->u("tex"), 0);

	glDrawArrays(GL_TRIANGLES, 0, myCubeVertexCount);

	glDisableVertexAttribArray(sp->a("vertex"));
	glDisableVertexAttribArray(sp->a("normal"));
	glDisableVertexAttribArray(sp->a("color"));
}

void drawKangaroo()
{
	glm::mat4 M1 = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową
	M1 = glm::translate(M1, glm::vec3(0, -2, 6));
	M1 = glm::rotate(M1, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	M1 = glm::rotate(M1, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	M1 = glm::scale(M1, glm::vec3(0.025, 0.025, 0.025));
	//glUniform4f(spLambert->u("color"), 0.37, 0.22, 0.08, 0.8); //Ustaw kolor rysowania obiektu
	glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M1)); //Załaduj do programu cieniującego macierz modelu
	loadModel("Models/Kangaroo.fbx", verts3, norms3, texCoords3, indices3, true, texKangaroo);
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float kat_x, float kat_y, float angleForDrink) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczyść bufor koloru i bufor głębokości

	GLuint drinks[5] = { texVodka, texWino, texZubr, texAmarena, texHarnas };

	V = glm::lookAt(pos, pos + computeDir(kat_x, kat_y), glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz widoku
	P = glm::perspective(glm::radians(50.0f), 1.0f, 0.1f, -100.0f); //Wylicz macierz rzutowania

	sp->use();

	glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P)); //Załaduj macierz rzutowania
	glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V)); //Załaduj macierz widoku


	glm::mat4 MWall = glm::mat4(1.0f);
	drawRoom();

	glm::mat4 MRack = glm::mat4(1.0f); //Zainicjuj macierz modelu macierzą jednostkową

	MRack = glm::translate(MRack, glm::vec3(6.3, -2.7, -3.2));
	MRack = glm::scale(MRack, glm::vec3(2, 2.2, 2));
	MRack = glm::rotate(MRack, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	drawRack(MRack, drinks[0]);


	for (int i = 1; i <= 4; i++) {
		MRack = glm::translate(MRack, glm::vec3(-1, 0, 0));
		drawRack(MRack, drinks[i]);
	}

	MRack = glm::translate(MRack, glm::vec3(3.5, 0, -5.6));
	MRack = glm::rotate(MRack, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	drawRack(MRack, drinks[0]);

	for (int i = 1; i <= 4; i++) {
		MRack = glm::translate(MRack, glm::vec3(1, 0, 0));
		drawRack(MRack, drinks[i]);
	}


	glm::mat4 MCounter = glm::mat4(1.0f);

	drawCounter();
	glm::mat4 MKangaroo = glm::mat4(1.0f);

	drawKangaroo();


	//if (flagDrink != 0) drawTest(MWall, angleForDrink);

	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(1000, 1000, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla
	float angleDrink = 0;
	float kat_y = 0;
	float kat_x = 0;
	glfwSetTime(0); //Wyzeruj licznik czasu
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		kat_y += speed_y * glfwGetTime(); //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		kat_x += speed_x * glfwGetTime(); //Oblicz kąt o jaki obiekt obrócił się podczas poprzedniej klatki
		pos += (float)(speed_walk * glfwGetTime()) * computeDir(0, kat_y);
		if (flagDrink == 0) {
			angleDrink = 0.0f;
		}
		if (flagDrink == 1) {
			angleDrink = angleDrink + 2.0f;
			angleDrink = min(angleDrink, 90.0f);
		}
		if (flagDrink == -1) {
			angleDrink = angleDrink - 2.0f;
			angleDrink = max(angleDrink, 0.0f);
			if (angleDrink <= 0.0f) flagDrink = 0;
		}
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window, kat_x, kat_y, angleDrink); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}



//#define GLM_FORCE_RADIANS
//
//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>
//#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <stdlib.h>
//#include <stdio.h>
//#include "constants.h"
//#include "allmodels.h"
//#include "lodepng.h"
//#include "shaderprogram.h"
//
//float speed = 0.75; // [radiany/s]
//float speedWheels = 1.5;
//float moveWheels = 0.0;
//
////Procedura obsługi błędów
//void error_callback(int error, const char* description) {
//	fputs(description, stderr);
//}
//
//// callback obsługi klawiatury
//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
//	if (action == GLFW_PRESS) {
//		if (key == GLFW_KEY_RIGHT) {
//			speed = 1.0;
//		}
//		if (key == GLFW_KEY_LEFT) {
//			speed = -3.14;
//		}
//		if (key == GLFW_KEY_A) {
//			moveWheels = 0.75;
//		}
//		if (key == GLFW_KEY_D) {
//			moveWheels = -0.75;
//		}
//	}
//	if (action == GLFW_RELEASE) {
//		if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT) {
//			speed = 0;
//		}
//		else {
//			moveWheels = 0.0;
//		}
//	}
//}
//
////Procedura inicjująca
//void initOpenGLProgram(GLFWwindow* window) {
//    initShaders();
//	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
//	glClearColor(0, 0, 0, 1);
//
//	glEnable(GL_DEPTH_TEST); //w części inicjującej, włączenie Z bufora
//}
//
//
////Zwolnienie zasobów zajętych przez program
//void freeOpenGLProgram(GLFWwindow* window) {
//    freeShaders();
//    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
//}
//
//void drawKostka(glm::mat4 M, float a, float b, float c) {
//	glm::mat4 Mk = glm::scale(M, glm::vec3(a / 2, b / 2, c / 2));
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(Mk));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::cube.drawSolid();
//}
//
//void drawJoint(glm::mat4 M) {
//	Models::Sphere joint(0.65, 36, 36);
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	joint.drawSolid();
//}
//
//void drawFinger(glm::mat4 MD, float angle, int num_segments) {
//	MD = glm::translate(MD, glm::vec3(0.5f, 0.0f, 0.0f));
//	MD = glm::rotate(MD, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//	MD = glm::translate(MD, glm::vec3(1.0f, 0.0f, 0.0f));
//	drawKostka(MD, 2.0, 0.5, 1.0);
//	for (int i = 0; i < num_segments - 1; i++) {
//		MD = glm::translate(MD, glm::vec3(1.0f, 0.0f, 0.0f));
//		drawJoint(MD);
//		MD = glm::rotate(MD, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//		MD = glm::translate(MD, glm::vec3(1.0f, 0.0f, 0.0f));
//		drawKostka(MD, 2.0, 0.5, 1.0);
//	}
//}
//
////Procedura rysująca zawartość sceny
//void drawScene(GLFWwindow* window, float angle, float angleWheels) {
//	//************Tutaj umieszczaj kod rysujący obraz******************
//
//	// stałe
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// włączenie czyszczenia bufora kolorów ii glębi co klatkę
//
//	// glm::mat4 MS, MP, MK;
//	Models::Sphere Sun(0.5, 36, 36);
//	Models::Sphere Earth(0.2, 36, 36);
//	Models::Sphere Mars(0.15, 36, 36);
//	Models::Sphere Moon(0.1, 36, 36);
//
//	// glm::mat4 RP, TP;
//	
//	glm::mat4 V = glm::lookAt(
//		glm::vec3(5.0f, 5.0f, -10.0f),
//		glm::vec3(0.0f, 0.0f, 0.0f),
//		glm::vec3(0.0f, 1.0f, 0.0f));
//	glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 1.0f, 50.0f);
//
//
//		/* Torusy */
//	spLambert->use();	//Aktywacja programu cieniującego
//	glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P));
//	glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V));
//
//	//---Poniższy kawałek kodu powtarzamy dla każdego obiektu
//	//Obliczanie macierzy modelu
//	glm::mat4 MDloni = glm::mat4(1.0f);
//	glm::mat4 Mpalm = MDloni;
//	drawKostka(Mpalm, 1.0, 0.5, 1.0);
//
//	int num_joints = 4;
//	for (int i = 0; i < num_joints; i++) {
//		MDloni = glm::rotate(MDloni, glm::radians(90.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
//		drawFinger(MDloni, angle, num_joints);
//	}
//
//	/*
//	//---Poniższy kawałek kodu powtarzamy dla każdego obiektu
//	//Obliczanie macierzy modelu
//	glm::mat4 MS = glm::mat4(1.0f);
//	MS = glm::rotate(MS, angle, glm::vec3(0.0f, 1.0f, 0.0f));
//
//		// podwozie
//	glm::mat4 MP = glm::scale(MS, glm::vec3(1.5f, 0.125f, 1.0f));
//	//Załadowanie macierzy modelu do programu cieniującego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MP));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::cube.drawWire(); //Narysowanie obiektu
//
//		// koła
//	// przednie
//	glm::mat4 MK1 = glm::translate(MS, glm::vec3(1.5f, 0.0f, 1.0f));
//	MK1 = glm::rotate(MK1, moveWheels, glm::vec3(0.0f, 1.0f, 0.0f));
//	MK1 = glm::rotate(MK1, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
//	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//	MK1 = glm::scale(MK1, glm::vec3(0.4f, 0.4f, 0.3f));
//	//Załadowanie macierzy modelu do programu cieniującego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK1));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::torus.drawSolid(); //Narysowanie obiektu
//
//	glm::mat4 MK2 = glm::translate(MS, glm::vec3(1.5f, 0.0f, -1.0f));
//	MK2 = glm::rotate(MK2, moveWheels, glm::vec3(0.0f, 1.0f, 0.0f));
//	MK2 = glm::rotate(MK2, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
//	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//	MK2 = glm::scale(MK2, glm::vec3(0.4f, 0.4f, 0.3f));
//	//Załadowanie macierzy modelu do programu cieniującego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK2));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::torus.drawSolid(); //Narysowanie obiektu
//
//	// tylne
//	glm::mat4 MK3 = glm::translate(MS, glm::vec3(-1.5f, 0.0f, 1.0f));
//	MK3 = glm::rotate(MK3, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
//	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//	MK3 = glm::scale(MK3, glm::vec3(0.4f, 0.4f, 0.3f));
//	//Załadowanie macierzy modelu do programu cieniującego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK3));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::torus.drawWire(); //Narysowanie obiektu
//
//	glm::mat4 MK4 = glm::translate(MS, glm::vec3(-1.5f, 0.0f, -1.0f));
//	MK4 = glm::rotate(MK4, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
//	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//	MK4 = glm::scale(MK4, glm::vec3(0.4f, 0.4f, 0.3f));
//	//Załadowanie macierzy modelu do programu cieniującego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK4));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::torus.drawWire(); //Narysowanie obiektu
//	*/
//
//	/*
//	for (int j = 0; j < 6; j++) {
//
//		//---Poniższy kawałek kodu powtarzamy dla każdego obiektu
//		//Obliczanie macierzy modelu - lewy
//		glm::mat4 M1 = glm::mat4(1.0f);
//		M1 = glm::rotate(M1, glm::radians(60.0f * j), glm::vec3(0.0f, 0.0f, 1.0f));
//		M1 = glm::translate(M1, glm::vec3(2.05f, 0.0f, 0.0f));
//		M1 = glm::rotate(M1, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
//		if (j % 2 == 0) {
//			M1 = glm::rotate(M1, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//		}
//		else {
//			M1 = glm::rotate(M1, glm::radians(12.0f), glm::vec3(0.0f, 0.0f, -1.0f));
//			M1 = glm::rotate(M1, angle, glm::vec3(0.0f, 0.0f, -1.0f));
//		}
//		//Załadowanie macierzy modelu do programu cieniującego
//		glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M1));
//		glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//		Models::torus.drawSolid(); //Narysowanie obiektu
//
//		for (int i = 1; i <= 12; i++) {
//			glm::mat4 MK = M1;
//			MK = glm::rotate(MK, glm::radians(30.0f) * (float)i, glm::vec3(0.0f, 0.0f, 1.0f));
//			MK = glm::translate(MK, glm::vec3(1.0f, 0.0f, 0.0f));
//			MK = glm::scale(MK, glm::vec3(0.1f, 0.1f, 0.1f));
//			glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK));
//			glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//			Models::cube.drawSolid(); //Narysowanie obiektu
//		}
//	}
//	*/
//
//	/*
//	//Obliczanie macierzy modelu - prawy
//	glm::mat4 M2 = glm::mat4(1.0f);
//	M2 = glm::translate(M2, glm::vec3(-1.05f, 0.0f, 0.0f));
//	M2 = glm::rotate(M2, angle, glm::vec3(0.0f, 0.0f, -1.0f));
//	//Załadowanie macierzy modelu do programu cieniującego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M2));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::torus.drawSolid(); //Narysowanie obiektu
//	//-----------
//
//	float moveObj = glm::radians(11.0f);
//	for (int i = 1; i <= 12; i++) {
//		glm::mat4 MK = M2;
//		MK = glm::rotate(MK, glm::radians(30.0f) * (float)i + moveObj, glm::vec3(0.0f, 0.0f, 1.0f));
//		MK = glm::translate(MK, glm::vec3(1.0f, 0.0f, 0.0f));
//		MK = glm::scale(MK, glm::vec3(0.1f, 0.1f, 0.1f));
//		glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK));
//		glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//		Models::cube.drawSolid(); //Narysowanie obiektu
//	}
//	*/
//
//	glfwSwapBuffers(window);
//}
//
//
//int main(void)
//{
//	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno
//
//	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów
//
//	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
//		fprintf(stderr, "Nie można zainicjować GLFW.\n");
//		exit(EXIT_FAILURE);
//	}
//
//	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.
//
//	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
//	{
//		fprintf(stderr, "Nie można utworzyć okna.\n");
//		glfwTerminate();
//		exit(EXIT_FAILURE);
//	}
//
//	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
//	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora
//
//	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
//		fprintf(stderr, "Nie można zainicjować GLEW.\n");
//		exit(EXIT_FAILURE);
//	}
//
//	initOpenGLProgram(window); //Operacje inicjujące
//
//	float angle = 0;
//	float angleWheels = 0;
//	glfwSetTime(0);
//
//	//Rejestracja procedury callback:
//	glfwSetKeyCallback(window, key_callback);
//
//	//Główna pętla
//	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
//	{
//		angleWheels += speedWheels * glfwGetTime();
//		angle += speed * glfwGetTime();
//		glfwSetTime(0);
//
//		drawScene(window, angle, angleWheels); //Wykonaj procedurę rysującą
//		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
//	}
//
//	freeOpenGLProgram(window);
//
//	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
//	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
//	exit(EXIT_SUCCESS);
//}
