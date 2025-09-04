#ifndef RATON_H
#define RATON_H
#include <GL/glut.h>

extern int w, h;

class Raton {
private:
	// En estos atributos almacenamos el estado del raton, la posicion (x,y) y el estado de los botones izquierdo y derecho
	static int x;
	static int y;
	static bool leftClick;
	static bool rightClick;
public:

	// En esta funcion linkeamos los callbacks a glut
	void Iniciar();
	//Estos callbacks llamara glut cuando suceda el evento correspondiente.
	static void Mouse_cb(int button, int state, int _x, int _y);
	static void PasiveMouse_cb(int _x, int _y);

	//Funciones para acceder al estado del raton
	bool IsLClicking();
	bool IsRClicking();
	int getX();
	int getY();
};

#endif

