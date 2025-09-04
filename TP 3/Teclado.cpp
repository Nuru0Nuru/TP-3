#include "Teclado.h"

bool Teclado::teclado[256];
bool Teclado::salir;

Teclado::Teclado
	(unsigned char adelante,unsigned char atras,unsigned char izquierda,unsigned char derecha,
		unsigned char agrandar,unsigned char achicar, unsigned char disparar, unsigned char crear, unsigned char prender) {
	// Marco todas las teclas como NO presionadas
	for(int i=0; i<256; i++)
		Teclado::teclado[i] = false;
	// Guardo el mapeo de teclas
	mapa[0]=adelante;
	mapa[1]=atras;
	mapa[2]=izquierda;
	mapa[3]=derecha;
	mapa[4]=agrandar;
	mapa[5]=achicar;	
	mapa[6]=disparar;
	mapa[7]=crear;
	mapa[8]=prender;
}

// Esta funcion registra los callbacks para que el objeto pueda escuchar cuando una tecla se presiona o se libera
void Teclado::Iniciar() {
	glutIgnoreKeyRepeat(true);
	glutKeyboardFunc(Teclado::KeyPressed_cb);
	glutKeyboardUpFunc(Teclado::KeyRelease_cb);
	glutSpecialFunc(Teclado::Special_cb);
}

// x,y posicion del mouse cuando se teclea (aqui no importan)
// Esta funcion marca la tecla 'key' como presionada, asignando al arreglo 'teclado' en la posicion 'key' el valor true.
void Teclado::KeyPressed_cb(unsigned char key,int x,int y) {
	Teclado::teclado[key] = true;
	if( key == 27 )
		salir = true;
}
// Este callback se llama cuando una tecla se suelta.
// Lo unico que hace es marcar el arreglo 'teclado' en la posicion 'key' como false, para indicar que la tecla NO esta presionada.
void Teclado::KeyRelease_cb(unsigned char key, int x, int y) {
	Teclado::teclado[key] = false;
}
// Special keys (non-ASCII)
// teclas de funcion, flechas, page up/dn, home/end, insert
void Teclado::Special_cb(int key,int xm=0,int ym=0) {
	if (key==GLUT_KEY_F4 && glutGetModifiers()==GLUT_ACTIVE_ALT) // alt+f4 => exit
		salir = true;
}

//Las funciones que siguen simplemente se fijan si la tecla guardada en mapa esta presionada

bool Teclado::Adelante( ) {
	return teclado[ mapa[0] ];
}
bool Teclado::Atras( ) {
	return teclado[ mapa[1] ];
}
bool Teclado::Izquierda( ) {
	return teclado[ mapa[2] ];
}
bool Teclado::Derecha( ) {
	return teclado[ mapa[3] ];
}
bool Teclado::Achicar() {
	return teclado[mapa[4]];
}
bool Teclado::Agrandar() {
	return teclado[mapa[5]];
}
bool Teclado::Disparar() {
	return teclado[mapa[6]];
}
bool Teclado::Crear() {
	return teclado[mapa[7]];
}
bool Teclado::Prender() {
	return teclado[mapa[8]];
}
bool Teclado::Salir( ) {
	return salir;
}
