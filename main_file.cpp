/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS

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

float speed = 0.75; // [radiany/s]
float speedWheels = 1.5;
float moveWheels = 0.0;

//Procedura obsługi błędów
void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

// callback obsługi klawiatury
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_RIGHT) {
			speed = 1.0;
		}
		if (key == GLFW_KEY_LEFT) {
			speed = -3.14;
		}
		if (key == GLFW_KEY_A) {
			moveWheels = 0.75;
		}
		if (key == GLFW_KEY_D) {
			moveWheels = -0.75;
		}
	}
	if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT) {
			speed = 0;
		}
		else {
			moveWheels = 0.0;
		}
	}
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
    initShaders();
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0, 0, 0, 1);

	glEnable(GL_DEPTH_TEST); //w części inicjującej, włączenie Z bufora
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    freeShaders();
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************
}

void drawKostka(glm::mat4 M, float a, float b, float c) {
	glm::mat4 Mk = glm::scale(M, glm::vec3(a / 2, b / 2, c / 2));
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(Mk));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
	Models::cube.drawSolid();
}

void drawJoint(glm::mat4 M) {
	Models::Sphere joint(0.65, 36, 36);
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
	joint.drawSolid();
}

void drawFinger(glm::mat4 MD, float angle, int num_segments) {
	MD = glm::translate(MD, glm::vec3(0.5f, 0.0f, 0.0f));
	MD = glm::rotate(MD, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	MD = glm::translate(MD, glm::vec3(1.0f, 0.0f, 0.0f));
	drawKostka(MD, 2.0, 0.5, 1.0);
	for (int i = 0; i < num_segments - 1; i++) {
		MD = glm::translate(MD, glm::vec3(1.0f, 0.0f, 0.0f));
		drawJoint(MD);
		MD = glm::rotate(MD, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		MD = glm::translate(MD, glm::vec3(1.0f, 0.0f, 0.0f));
		drawKostka(MD, 2.0, 0.5, 1.0);
	}
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float angle, float angleWheels) {
	//************Tutaj umieszczaj kod rysujący obraz******************

	// stałe
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// włączenie czyszczenia bufora kolorów ii glębi co klatkę

	// glm::mat4 MS, MP, MK;
	Models::Sphere Sun(0.5, 36, 36);
	Models::Sphere Earth(0.2, 36, 36);
	Models::Sphere Mars(0.15, 36, 36);
	Models::Sphere Moon(0.1, 36, 36);

	// glm::mat4 RP, TP;
	
	glm::mat4 V = glm::lookAt(
		glm::vec3(5.0f, 5.0f, -10.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 1.0f, 50.0f);


		/* Torusy */
	spLambert->use();	//Aktywacja programu cieniującego
	glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P));
	glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V));

	//---Poniższy kawałek kodu powtarzamy dla każdego obiektu
	//Obliczanie macierzy modelu
	glm::mat4 MDloni = glm::mat4(1.0f);
	glm::mat4 Mpalm = MDloni;
	drawKostka(Mpalm, 1.0, 0.5, 1.0);

	int num_joints = 4;
	for (int i = 0; i < num_joints; i++) {
		MDloni = glm::rotate(MDloni, glm::radians(90.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
		drawFinger(MDloni, angle, num_joints);
	}

	/*
	//---Poniższy kawałek kodu powtarzamy dla każdego obiektu
	//Obliczanie macierzy modelu
	glm::mat4 MS = glm::mat4(1.0f);
	MS = glm::rotate(MS, angle, glm::vec3(0.0f, 1.0f, 0.0f));

		// podwozie
	glm::mat4 MP = glm::scale(MS, glm::vec3(1.5f, 0.125f, 1.0f));
	//Załadowanie macierzy modelu do programu cieniującego
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MP));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
	Models::cube.drawWire(); //Narysowanie obiektu

		// koła
	// przednie
	glm::mat4 MK1 = glm::translate(MS, glm::vec3(1.5f, 0.0f, 1.0f));
	MK1 = glm::rotate(MK1, moveWheels, glm::vec3(0.0f, 1.0f, 0.0f));
	MK1 = glm::rotate(MK1, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	MK1 = glm::scale(MK1, glm::vec3(0.4f, 0.4f, 0.3f));
	//Załadowanie macierzy modelu do programu cieniującego
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK1));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
	Models::torus.drawSolid(); //Narysowanie obiektu

	glm::mat4 MK2 = glm::translate(MS, glm::vec3(1.5f, 0.0f, -1.0f));
	MK2 = glm::rotate(MK2, moveWheels, glm::vec3(0.0f, 1.0f, 0.0f));
	MK2 = glm::rotate(MK2, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	MK2 = glm::scale(MK2, glm::vec3(0.4f, 0.4f, 0.3f));
	//Załadowanie macierzy modelu do programu cieniującego
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK2));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
	Models::torus.drawSolid(); //Narysowanie obiektu

	// tylne
	glm::mat4 MK3 = glm::translate(MS, glm::vec3(-1.5f, 0.0f, 1.0f));
	MK3 = glm::rotate(MK3, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	MK3 = glm::scale(MK3, glm::vec3(0.4f, 0.4f, 0.3f));
	//Załadowanie macierzy modelu do programu cieniującego
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK3));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
	Models::torus.drawWire(); //Narysowanie obiektu

	glm::mat4 MK4 = glm::translate(MS, glm::vec3(-1.5f, 0.0f, -1.0f));
	MK4 = glm::rotate(MK4, angleWheels, glm::vec3(0.0f, 0.0f, 1.0f));
	// MK = glm::rotate(MK, angle, glm::vec3(0.0f, 0.0f, 1.0f));
	MK4 = glm::scale(MK4, glm::vec3(0.4f, 0.4f, 0.3f));
	//Załadowanie macierzy modelu do programu cieniującego
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK4));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
	Models::torus.drawWire(); //Narysowanie obiektu
	*/

	/*
	for (int j = 0; j < 6; j++) {

		//---Poniższy kawałek kodu powtarzamy dla każdego obiektu
		//Obliczanie macierzy modelu - lewy
		glm::mat4 M1 = glm::mat4(1.0f);
		M1 = glm::rotate(M1, glm::radians(60.0f * j), glm::vec3(0.0f, 0.0f, 1.0f));
		M1 = glm::translate(M1, glm::vec3(2.05f, 0.0f, 0.0f));
		M1 = glm::rotate(M1, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		if (j % 2 == 0) {
			M1 = glm::rotate(M1, angle, glm::vec3(0.0f, 0.0f, 1.0f));
		}
		else {
			M1 = glm::rotate(M1, glm::radians(12.0f), glm::vec3(0.0f, 0.0f, -1.0f));
			M1 = glm::rotate(M1, angle, glm::vec3(0.0f, 0.0f, -1.0f));
		}
		//Załadowanie macierzy modelu do programu cieniującego
		glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M1));
		glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
		Models::torus.drawSolid(); //Narysowanie obiektu

		for (int i = 1; i <= 12; i++) {
			glm::mat4 MK = M1;
			MK = glm::rotate(MK, glm::radians(30.0f) * (float)i, glm::vec3(0.0f, 0.0f, 1.0f));
			MK = glm::translate(MK, glm::vec3(1.0f, 0.0f, 0.0f));
			MK = glm::scale(MK, glm::vec3(0.1f, 0.1f, 0.1f));
			glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK));
			glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
			Models::cube.drawSolid(); //Narysowanie obiektu
		}
	}
	*/

	/*
	//Obliczanie macierzy modelu - prawy
	glm::mat4 M2 = glm::mat4(1.0f);
	M2 = glm::translate(M2, glm::vec3(-1.05f, 0.0f, 0.0f));
	M2 = glm::rotate(M2, angle, glm::vec3(0.0f, 0.0f, -1.0f));
	//Załadowanie macierzy modelu do programu cieniującego
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M2));
	glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
	Models::torus.drawSolid(); //Narysowanie obiektu
	//-----------

	float moveObj = glm::radians(11.0f);
	for (int i = 1; i <= 12; i++) {
		glm::mat4 MK = M2;
		MK = glm::rotate(MK, glm::radians(30.0f) * (float)i + moveObj, glm::vec3(0.0f, 0.0f, 1.0f));
		MK = glm::translate(MK, glm::vec3(1.0f, 0.0f, 0.0f));
		MK = glm::scale(MK, glm::vec3(0.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK));
		glUniform4f(spLambert->u("color"), 0, 1, 0, 1);
		Models::cube.drawSolid(); //Narysowanie obiektu
	}
	*/


	/*
		Rysowanie układu słonecznego
		// ---Poniższy kawałek kodu powtarzamy dla każdego obiektu-------------------------------------
	// Słońce
	// Obliczanie macierzy modelu
	glm::mat4 MS = glm::mat4(1.0f);
	//Załadowanie macierzy modelu do programu cieniującego
	glUniform4f(spLambert->u("color"), 1.0f, 1.0f, 0.0f, 1.0f);
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MS));
	Sun.drawSolid();

	// Planeta
	//Obliczanie macierzy modelu
	glm::mat4 MP = MS;
	MP = glm::rotate(MP, angle, glm::vec3(0.0f, 1.0f, 0.0f)); // obróć o zadany kąt
	MP = glm::translate(MP, glm::vec3(1.5f, 0.0f, 0.0f));     // przesuń na odległość orbity
	//Załadowanie macierzy modelu do programu cieniującego
	glUniform4f(spLambert->u("color"), 0.0, 1.0, 0.0, 1.0);
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MP));
	Earth.drawSolid();

	// Księżyc
	//Obliczanie macierzy modelu
	glm::mat4 MK = MP;
	MK = glm::rotate(MK, angle * 3.7f, glm::vec3(0.0f, 1.0f, 0.0f)); // mnożę kąt aby nadać różne prędkości orbit
	MK = glm::translate(MK, glm::vec3(0.5f, 0.0f, 0.0f));
	//Załadowanie macierzy modelu do programu cieniującego
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MK));
	glUniform4f(spLambert->u("color"), 0.5, 0.5, 0.5, 1);
	Moon.drawSolid();
		//-----------------------------------------------------------------------------------------------

	// Planeta-Mars
	//Obliczanie macierzy modelu
	glm::mat4 MM = MS;
	MM = glm::rotate(MM, angle, glm::vec3(0.0f, 0.0f, 1.0f)); // obróć o zadany kąt
	MM = glm::translate(MM, glm::vec3(1.5f, 0.0f, 0.0f));     // przesuń na odległość orbity
	//Załadowanie macierzy modelu do programu cieniującego
	glUniform4f(spLambert->u("color"), 0.0, 0.0, 1.0, 1.0);
	glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(MM));
	Mars.drawSolid();
	*/



		/* Stara część programu */
	// R = glm::rotate(M, angle, glm::vec3(1.0f, 0.0f, 1.0f));
	// T = glm::translate(M, glm::vec3(1.0f, 1.0f, 0.0f));
	// S = glm::scale(M,glm::vec3(1.5f,1.0f,2.0f));
	// M = R * T * S;

	// spLambert->use();	//Aktywacja programu cieniującego
	// glUniform4f(spLambert->u("color"), 0.6, 0.0, 0.1, 1);
	// glUniformMatrix4fv(spLambert->u("P"), 1, false, glm::value_ptr(P));
	// glUniformMatrix4fv(spLambert->u("V"), 1, false, glm::value_ptr(V));
	// glUniformMatrix4fv(spLambert->u("M"), 1, false, glm::value_ptr(M));

	// Models::torus.drawSolid();

	glfwSwapBuffers(window);
}


int main(void)
{
	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

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

	float angle = 0;
	float angleWheels = 0;
	glfwSetTime(0);

	//Rejestracja procedury callback:
	glfwSetKeyCallback(window, key_callback);

	//Główna pętla
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		angleWheels += speedWheels * glfwGetTime();
		angle += speed * glfwGetTime();
		glfwSetTime(0);

		drawScene(window, angle, angleWheels); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
