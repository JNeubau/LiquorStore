#define GLM_FORCE_RADIANS

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

GLuint tex;

glm::vec3 pos = glm::vec3(0, 1, -10);

std::vector<glm::vec4> verts;
std::vector<glm::vec4> norms;
std::vector<glm::vec2> texCoords;
std::vector<unsigned int> indices;

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
		if (key == GLFW_KEY_UP) speed_walk = 2;
		if (key == GLFW_KEY_DOWN) speed_walk = -2;
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_LEFT) speed_y = 0;
		if (key == GLFW_KEY_RIGHT) speed_y = 0;
		if (key == GLFW_KEY_UP) speed_walk = 0;
		if (key == GLFW_KEY_DOWN) speed_walk = 0;
		if (key == GLFW_KEY_PAGE_UP) speed_x = 0;
		if (key == GLFW_KEY_PAGE_DOWN) speed_x = 0;
	}
}
void loadModel(string filename)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
	cout << importer.GetErrorString() << endl;

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
}

//Procedura obs??ugi b????d??w
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

//Procedura inicjuj??ca
void initOpenGLProgram(GLFWwindow* window) {
	initShaders();
	//************Tutaj umieszczaj kod, kt??ry nale??y wykona?? raz, na pocz??tku programu************
	glClearColor(0, 0, 0, 1); //Ustaw kolor czyszczenia bufora kolor??w
	glEnable(GL_DEPTH_TEST); //W????cz test g????boko??ci na pikselach
	glfwSetKeyCallback(window, key_callback);
}


//Zwolnienie zasob??w zaj??tych przez program
void freeOpenGLProgram(GLFWwindow* window) {
	freeShaders();
	//************Tutaj umieszczaj kod, kt??ry nale??y wykona?? po zako??czeniu p??tli g????wnej************
}



void texRack(glm::mat4 P, glm::mat4 V, glm::mat4 M)
{
	spLambert->use();
	glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V));
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M));

	glEnableVertexAttribArray(spLambert->a("vertex"));
	glVertexAttribPointer(spLambert->a("vertex"), 4, GL_FLOAT, false, 0, verts.data());

	glEnableVertexAttribArray(spLambert->a("texCoord"));
	glVertexAttribPointer(spLambert->a("texCoord"), 2, GL_FLOAT, false, 0, texCoords.data());

	glEnableVertexAttribArray(spLambert->a("normal"));
	glVertexAttribPointer(spLambert->a("normal"), 4, GL_FLOAT, false, 0, norms.data());

	glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, tex);
	glUniform1i(spLambert->u("tex"), 0);

	//glDrawArrays(GL_TRIANGLES, 0, vertexCount);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, indices.data());

	glDisableVertexAttribArray(spLambert->a("vertex"));
	glDisableVertexAttribArray(spLambert->a("texCoord"));
	glDisableVertexAttribArray(spLambert->a("normal"));

}
//Procedura rysuj??ca zawarto???? sceny
void drawScene(GLFWwindow* window, float kat_x, float kat_y) {
	//************Tutaj umieszczaj kod rysuj??cy obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Wyczy???? bufor koloru i bufor g????boko??ci


	glm::mat4 V = glm::lookAt(pos, pos + computeDir(kat_x, kat_y), glm::vec3(0.0f, 1.0f, 0.0f)); //Wylicz macierz widoku
	glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 0.1f, -100.0f); //Wylicz macierz rzutowania

	spLambert->use(); //Aktyeuj program cieniuj??cy

	glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P)); //Za??aduj do programu cieniuj??cego macierz rzutowania
	glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V)); //Za??aduj do programu cieniuj??cego macierz widoku

	loadModel("etagereEnfant.obj");
	glm::mat4 M1 = glm::mat4(1.0f); //Zainicjuj macierz modelu macierz?? jednostkow??
	//M1 = glm::translate(M1, glm::vec3(4, 3, 0));
	M1 = glm::scale(M1, glm::vec3(0.08, 0.03, 0.01));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1); //Ustaw kolor rysowania obiektu
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M1)); //Za??aduj do programu cieniuj??cego macierz modelu
	//Models::cube.drawSolid(); //Narysuj obiekt
	loadModel("etagereEnfant.obj");
	texRack(P, V, M1);


	glm::mat4 M2 = glm::mat4(1.0f); //Zainicjuj macierz modelu macierz?? jednostkow??
	M2 = glm::translate(M2, glm::vec3(-4, 2, 0));
	M2 = glm::scale(M2, glm::vec3(1.5, 2, 1));
	glUniform4f(spLambert->u("color"), 1, 1, 0, 1); //Ustaw kolor rysowania obiektu
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M2)); //Za??aduj do programu cieniuj??cego macierz modelu
	Models::cube.drawSolid(); //Narysuj obiekt

	glm::mat4 M3 = glm::mat4(1.0f); //Zainicjuj macierz modelu macierz?? jednostkow??
	M3 = glm::translate(M3, glm::vec3(-4, 1, 3));
	M3 = glm::scale(M3, glm::vec3(0.5, 1, 1.5));
	glUniform4f(spLambert->u("color"), 0, 1, 1, 1); //Ustaw kolor rysowania obiektu
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M3)); //Za??aduj do programu cieniuj??cego macierz modelu
	Models::cube.drawSolid(); //Narysuj obiekt

	glm::mat4 M4 = glm::mat4(1.0f); //Zainicjuj macierz modelu macierz?? jednostkow??	
	M4 = glm::translate(M4, glm::vec3(0, 0.25f, 0));
	M4 = glm::scale(M4, glm::vec3(0.5, 0.25, 0.5));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1); //Ustaw kolor rysowania obiektu
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M4)); //Za??aduj do programu cieniuj??cego macierz modelu
	Models::cube.drawSolid(); //Narysuj obiekt

	glm::mat4 M5 = glm::mat4(1.0f); //Zainicjuj macierz modelu macierz?? jednostkow??	
	M5 = glm::translate(M5, glm::vec3(0, 0.9f, 0));
	glUniform4f(spLambert->u("color"), 1, 0, 0, 1); //Ustaw kolor rysowania obiektu
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M5)); //Za??aduj do programu cieniuj??cego macierz modelu
	Models::teapot.drawSolid(); //Narysuj obiekt

	glfwSwapBuffers(window); //Skopiuj bufor tylny do bufora przedniego
}


int main(void)
{
	GLFWwindow* window; //Wska??nik na obiekt reprezentuj??cy okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedur?? obs??ugi b????d??w

	if (!glfwInit()) { //Zainicjuj bibliotek?? GLFW
		fprintf(stderr, "Nie mo??na zainicjowa?? GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utw??rz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Je??eli okna nie uda??o si?? utworzy??, to zamknij program
	{
		fprintf(stderr, "Nie mo??na utworzy?? okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje si?? aktywny i polecenia OpenGL b??d?? dotyczy?? w??a??nie jego.
	glfwSwapInterval(1); //Czekaj na 1 powr??t plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotek?? GLEW
		fprintf(stderr, "Nie mo??na zainicjowa?? GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjuj??ce

	//G????wna p??tla
	float angle = 0; //zadeklaruj zmienn?? przechowuj??c?? aktualny k??t obrotu
	float kat_y = 0;
	float kat_x = 0;
	glfwSetTime(0); //Wyzeruj licznik czasu
	while (!glfwWindowShouldClose(window)) //Tak d??ugo jak okno nie powinno zosta?? zamkni??te
	{
		kat_y += speed_y * glfwGetTime(); //Oblicz k??t o jaki obiekt obr??ci?? si?? podczas poprzedniej klatki
		kat_x += speed_x * glfwGetTime(); //Oblicz k??t o jaki obiekt obr??ci?? si?? podczas poprzedniej klatki
		pos += (float)(speed_walk * glfwGetTime()) * computeDir(0, kat_y);
		glfwSetTime(0); //Wyzeruj licznik czasu
		drawScene(window, kat_x, kat_y); //Wykonaj procedur?? rysuj??c??
		glfwPollEvents(); //Wykonaj procedury callback w zalezno??ci od zdarze?? jakie zasz??y.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usu?? kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zaj??te przez GLFW
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
////Procedura obs??ugi b????d??w
//void error_callback(int error, const char* description) {
//	fputs(description, stderr);
//}
//
//// callback obs??ugi klawiatury
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
////Procedura inicjuj??ca
//void initOpenGLProgram(GLFWwindow* window) {
//    initShaders();
//	//************Tutaj umieszczaj kod, kt??ry nale??y wykona?? raz, na pocz??tku programu************
//	glClearColor(0, 0, 0, 1);
//
//	glEnable(GL_DEPTH_TEST); //w cz????ci inicjuj??cej, w????czenie Z bufora
//}
//
//
////Zwolnienie zasob??w zaj??tych przez program
//void freeOpenGLProgram(GLFWwindow* window) {
//    freeShaders();
//    //************Tutaj umieszczaj kod, kt??ry nale??y wykona?? po zako??czeniu p??tli g????wnej************
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
////Procedura rysuj??ca zawarto???? sceny
//void drawScene(GLFWwindow* window, float angle, float angleWheels) {
//	//************Tutaj umieszczaj kod rysuj??cy obraz******************
//
//	// sta??e
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// w????czenie czyszczenia bufora kolor??w ii gl??bi co klatk??
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
//	spLambert->use();	//Aktywacja programu cieniuj??cego
//	glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P));
//	glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V));
//
//	//---Poni??szy kawa??ek kodu powtarzamy dla ka??dego obiektu
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
//	//---Poni??szy kawa??ek kodu powtarzamy dla ka??dego obiektu
//	//Obliczanie macierzy modelu
//	glm::mat4 MS = glm::mat4(1.0f);
//	MS = glm::rotate(MS, angle, glm::vec3(0.0f, 1.0f, 0.0f));
//
//		// podwozie
//	glm::mat4 MP = glm::scale(MS, glm::vec3(1.5f, 0.125f, 1.0f));
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MP));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::cube.drawWire(); //Narysowanie obiektu
//
//		// ko??a
//	// przednie
//	glm::mat4 MK1 = glm::translate(MS, glm::vec3(1.5f, 0.0f, 1.0f));
//	MK1 = glm::rotate(MK1, moveWheels, glm::vec3(0.0f, 1.0f, 0.0f));
//	MK1 = glm::rotate(MK1, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
//	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//	MK1 = glm::scale(MK1, glm::vec3(0.4f, 0.4f, 0.3f));
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK1));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::torus.drawSolid(); //Narysowanie obiektu
//
//	glm::mat4 MK2 = glm::translate(MS, glm::vec3(1.5f, 0.0f, -1.0f));
//	MK2 = glm::rotate(MK2, moveWheels, glm::vec3(0.0f, 1.0f, 0.0f));
//	MK2 = glm::rotate(MK2, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
//	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//	MK2 = glm::scale(MK2, glm::vec3(0.4f, 0.4f, 0.3f));
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK2));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::torus.drawSolid(); //Narysowanie obiektu
//
//	// tylne
//	glm::mat4 MK3 = glm::translate(MS, glm::vec3(-1.5f, 0.0f, 1.0f));
//	MK3 = glm::rotate(MK3, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
//	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//	MK3 = glm::scale(MK3, glm::vec3(0.4f, 0.4f, 0.3f));
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK3));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::torus.drawWire(); //Narysowanie obiektu
//
//	glm::mat4 MK4 = glm::translate(MS, glm::vec3(-1.5f, 0.0f, -1.0f));
//	MK4 = glm::rotate(MK4, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
//	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
//	MK4 = glm::scale(MK4, glm::vec3(0.4f, 0.4f, 0.3f));
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK4));
//	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
//	Models::torus.drawWire(); //Narysowanie obiektu
//	*/
//
//	/*
//	for (int j = 0; j < 6; j++) {
//
//		//---Poni??szy kawa??ek kodu powtarzamy dla ka??dego obiektu
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
//		//Za??adowanie macierzy modelu do programu cieniuj??cego
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
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
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
//
//	/*
//		Rysowanie uk??adu s??onecznego
//		// ---Poni??szy kawa??ek kodu powtarzamy dla ka??dego obiektu-------------------------------------
//	// S??o??ce
//	// Obliczanie macierzy modelu
//	glm::mat4 MS = glm::mat4(1.0f);
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
//	glUniform4f(spLambert->u("color"), 1.0f, 1.0f, 0.0f, 1.0f);
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MS));
//	Sun.drawSolid();
//
//	// Planeta
//	//Obliczanie macierzy modelu
//	glm::mat4 MP = MS;
//	MP = glm::rotate(MP, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // obr???? o zadany k??t
//	MP = glm::translate(MP, glm::vec3(1.5f, 0.0f, 0.0f));     // przesu?? na odleg??o???? orbity
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
//	glUniform4f(spLambert->u("color"), 0.0, 1.0, 0.0, 1.0);
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MP));
//	Earth.drawSolid();
//
//	// Ksi????yc
//	//Obliczanie macierzy modelu
//	glm::mat4 MK = MP;
//	MK = glm::rotate(MK, angle * 3.7f, glm::vec3(0.0f, 1.0f, 0.0f)); // mno???? k??t aby nada?? r????ne pr??dko??ci orbit
//	MK = glm::translate(MK, glm::vec3(0.5f, 0.0f, 0.0f));
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK));
//	glUniform4f(spLambert->u("color"), 0.5, 0.5, 0.5, 1);
//	Moon.drawSolid();
//		//-----------------------------------------------------------------------------------------------
//
//	// Planeta-Mars
//	//Obliczanie macierzy modelu
//	glm::mat4 MM = MS;
//	MM = glm::rotate(MM, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // obr???? o zadany k??t
//	MM = glm::translate(MM, glm::vec3(1.5f, 0.0f, 0.0f));     // przesu?? na odleg??o???? orbity
//	//Za??adowanie macierzy modelu do programu cieniuj??cego
//	glUniform4f(spLambert->u("color"), 0.0, 0.0, 1.0, 1.0);
//	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MM));
//	Mars.drawSolid();
//	*/
//
//
//
//		/* Stara cz?????? programu */
//	// R = glm::rotate(M, angle, glm::vec3(1.0f, 0.0f, 1.0f));
//	// T = glm::translate(M, glm::vec3(1.0f, 1.0f, 0.0f));
//	// S = glm::scale(M,glm::vec3(1.5f,1.0f,2.0f));
//	// M = R * T * S;
//
//	// spLambert->use();	//Aktywacja programu cieniuj??cego
//	// glUniform4f(spLambert->u("color"), 0.6, 0.0, 0.1, 1);
//	// glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P));
//	// glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V));
//	// glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M));
//
//	// Models::torus.drawSolid();
//
//	glfwSwapBuffers(window);
//}
//
//
//int main(void)
//{
//	GLFWwindow* window; //Wska??nik na obiekt reprezentuj??cy okno
//
//	glfwSetErrorCallback(error_callback);//Zarejestruj procedur?? obs??ugi b????d??w
//
//	if (!glfwInit()) { //Zainicjuj bibliotek?? GLFW
//		fprintf(stderr, "Nie mo??na zainicjowa?? GLFW.\n");
//		exit(EXIT_FAILURE);
//	}
//
//	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utw??rz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.
//
//	if (!window) //Je??eli okna nie uda??o si?? utworzy??, to zamknij program
//	{
//		fprintf(stderr, "Nie mo??na utworzy?? okna.\n");
//		glfwTerminate();
//		exit(EXIT_FAILURE);
//	}
//
//	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje si?? aktywny i polecenia OpenGL b??d?? dotyczy?? w??a??nie jego.
//	glfwSwapInterval(1); //Czekaj na 1 powr??t plamki przed pokazaniem ukrytego bufora
//
//	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotek?? GLEW
//		fprintf(stderr, "Nie mo??na zainicjowa?? GLEW.\n");
//		exit(EXIT_FAILURE);
//	}
//
//	initOpenGLProgram(window); //Operacje inicjuj??ce
//
//	float angle = 0;
//	float angleWheels = 0;
//	glfwSetTime(0);
//
//	//Rejestracja procedury callback:
//	glfwSetKeyCallback(window, key_callback);
//
//	//G????wna p??tla
//	while (!glfwWindowShouldClose(window)) //Tak d??ugo jak okno nie powinno zosta?? zamkni??te
//	{
//		angleWheels += speedWheels * glfwGetTime();
//		angle += speed * glfwGetTime();
//		glfwSetTime(0);
//
//		drawScene(window, angle, angleWheels); //Wykonaj procedur?? rysuj??c??
//		glfwPollEvents(); //Wykonaj procedury callback w zalezno??ci od zdarze?? jakie zasz??y.
//	}
//
//	freeOpenGLProgram(window);
//
//	glfwDestroyWindow(window); //Usu?? kontekst OpenGL i okno
//	glfwTerminate(); //Zwolnij zasoby zaj??te przez GLFW
//	exit(EXIT_SUCCESS);
//}
