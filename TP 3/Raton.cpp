#include "Raton.h"


int Raton::x;
int Raton::y;
bool Raton::leftClick;
bool Raton::rightClick;


void Raton::Iniciar() {
	glutMouseFunc(Mouse_cb);
	glutPassiveMotionFunc(PasiveMouse_cb);
}

void Raton::Mouse_cb(int button, int state, int _x, int _y) {
	if (state == GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON)
			leftClick = true;
		if (button == GLUT_RIGHT_BUTTON)
			rightClick = true;
	}
	if (state == GLUT_UP) {
		if (button == GLUT_LEFT_BUTTON)
			leftClick = false;
		if (button == GLUT_RIGHT_BUTTON)
			rightClick = false;
	}
	x = _x;
	y = h - _y;
}

void Raton::PasiveMouse_cb(int _x, int _y) {
	x = _x;
	y = h - _y;
}

int Raton::getX() {
	return x;
}

int Raton::getY() {
	return y;
}

bool Raton::IsLClicking() {
	return leftClick;
}

bool Raton::IsRClicking() {
	return rightClick;
}