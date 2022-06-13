#version 330

uniform sampler2D textureMap0;
uniform sampler2D textureMap1;

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

//in vec4 ic; 
in vec4 n;
in vec4 l;
//in vec4 v;
in vec2 iTexCoord0;
//in vec2 iTexCoord1;

in float layer; //aktualna warstwa

void main(void) {

	//Znormalizowane interpolowane wektory
	vec4 ml = normalize(l);
	vec4 mn = normalize(n);
	//vec4 mv = normalize(v);
	//Wektor odbity
	vec4 mr = reflect(-ml, mn);

	//Parametry powierzchni
	//vec4 kd = mix(texture(textureMap0,iTexCoord0),texture(textureMap1, iTexCoord1),0.3);
	//vec4 ks = vec4(1,1,1,1);

	//Obliczenie modelu oświetlenia
	float nl = clamp(dot(mn, ml), 0, 1);
	//float rv = pow(clamp(dot(mr, mv), 0, 1), 50);
	//pixelColor= vec4(kd.rgb * nl, kd.a) + vec4(ks.rgb*rv, 0);

vec4 fur=texture(textureMap0,iTexCoord0); //futro (jest wlos?)
vec4 furCol=texture(textureMap1,iTexCoord0); //kolor futra
vec4 col=furCol;                          //domyślny kolor
//Jeżeli nie pierwsza warstwa to dodaj alfa z teks. futra
if (layer!=0) col=vec4(furCol.rgb,fur.a);
// Usuń niewidoczny fragment, Z Bufor pozostanie bez zmian
if (fur.a==0 && layer!=0) discard; 
pixelColor=col*vec4(nl,nl,nl,1);       //wynikowy kolor

}
