#version 330

//Zmienne jednorodne
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

//Atrybuty
in vec4 vertex; //wspolrzedne wierzcholka w przestrzeni modelu
//in vec4 color; //kolor związany z wierzchołkiem
//in vec4 normal; //wektor normalny w przestrzeni modelu
in vec2 texCoord0;

in vec4 c1;
in vec4 c2;
in vec4 c3;

//Zmienne interpolowane
//out vec4 ic;
out vec4 l;
//out vec4 n;
out vec4 v;
out vec2 iTexCoord0;


void main(void) {
	mat4 invTBN = mat4(c1, c2, c3, vec4(0,0,0,1));

    vec4 lp = vec4(0, 0, -6, 1); //przestrzeń świata
    // l = normalize(V * lp - V*M*vertex); //wektor do światła w przestrzeni oka
	l = normalize(invTBN*(inverse(M)*lp - vertex)); //wektor do światła w przestrzeni oka
    // v = normalize(vec4(0, 0, 0, 1) - V * M * vertex); //wektor do obserwatora w przestrzeni oka
	v = normalize(invTBN*(inverse(V * M)*vec4(0,0,0,1) - vertex)); //wektor do obserwatora w przestrzeni oka
    //n = normalize(V * M * normal); //wektor normalny w przestrzeni oka
    iTexCoord0 = texCoord0;

    //ic = color;
    
    gl_Position=P*V*M*vertex;
}
