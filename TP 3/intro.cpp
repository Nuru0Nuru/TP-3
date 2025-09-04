#include <iostream> // cout
#include <cstdlib> // exit
#include <cmath> // fabs
#include <fstream> // file io
#include <list>
#include <string>
#include <stdio.h> // sprintf
#include <GL/glut.h>
#include "Teclado.h"
#include "uglyfont.h"
#include "Raton.h"

using namespace std;

//------------------------------------------------------------
// variables globales
int w = 800, h = 600; // tamanio inicial de la ventana
int EnemigoX = 60, EnemigoY = 60, Energia = 100;

double
AvionX = 400,
AvionY = 300,
AvionAng = 0,
ArmaAng = 0,
ArmaTamanio = 0;

int segundos; // segundos transcurridos desde el inicio del programa
int milisegundos;

const double PI = 4 * atan(1.0);
bool cl_info = true, luces_on = true; // informa por la linea de comandos

static int AngLineaRadar = 0; // angulo de rotacion de linea del radar

GLuint texid[3];

// Carga una imagen en formato ppm, crea el canal alpha (ya que las imagenes ppm 
// no guardan este canal) , crea los mipmaps y deja la textura lista para usar en nuestro programa
bool mipmap_ppm(const char* ifile) {
    // Se declaran algunas variables para usar despues y se abre el archivo que contiene la imagen
    char dummy; int maxc, wt, ht;
    ifstream fileinput(ifile, ios::binary);
    // Si no se pudo abrir el archivo se escribe por consola que hubo un error y se anula la operacion
    if (!fileinput.is_open()) { cerr << "Not found" << endl; return false; }
    fileinput.get(dummy);
    // Se leen dos caracteres llamados "números mágicos" que indican que el archivo es una 
    // imagen en formato ppm. Estos caracteres son una 'P' seguida de un '6'
    // Si no se encuentran estos caracteres significa que el archivo no es una 
    // imagen ppm por lo que se anula el proceso
    if (dummy != 'P') { cerr << "Not P6 PPM file" << endl; return false; }
    fileinput.get(dummy);
    if (dummy != '6') { cerr << "Not P6 PPM file" << endl; return false; }
    // Se leen algunos caracteres de relleno, que no tienen ningun significado util, se leen solo 
    // para saltearlos. Primero se lee un espacio, luego, el metodo peek lee un caracter sin 
    // adelantar la posicion, si este es un '#' se continua leyendo hasta que se encuentra 
    // un salto de linea (un enter)  
    fileinput.get(dummy);
    dummy = fileinput.peek();
    if (dummy == '#') do {
        fileinput.get(dummy);
    } while (dummy != 10);
    // Se lee el ancho y alto de la imagen, y el ultimo indica la cantidad de colores de la imagen
    fileinput >> wt >> ht;
    fileinput >> maxc;
    // Se saltea un espacio en blanco
    fileinput.get(dummy);
    // Se reserva memoria para wt*xt pixels. Como tenemos un byte para cada canal (rgb), 
    // son 3 bytes por pixel. Por ultimo se cierra en archivo    
    unsigned char* img = new unsigned char[3 * wt * ht];
    fileinput.read((char*)img, 3 * wt * ht);
    fileinput.close();
    // gluBuild2DMipmaps(GL_TEXTURE_2D, 3, wt, ht,  GL_RGB, GL_UNSIGNED_BYTE, img);
    // Conversion a rgba alpha=255-(r+g+b)/3 (blanco=transparente, negro=opaco)
    // Las imagenes en formato ppm no pueden guardar el canal alpha, por lo que lo creamos 
    // a partir de los canales rgb. Primero reservamos memoria para wt*ht pixels, esta vez 
    // con 4 bytes por pixel (rgba)
    // Recorremos la imagen leida del archivo, copiamos los canales r, g y b. El canal 
    // alpha lo definimos como trasparente si el color es blanco (si r=g=b=255, 
    // entonces r+g+b=765). Sino alpha es completamente opaco (=255)
    unsigned char* imga = new unsigned char[4 * wt * ht];
    unsigned char r, g, b;
    for (int i = 0; i < wt * ht; i++) {
        r = imga[4 * i + 0] = img[3 * i + 0];
        g = imga[4 * i + 1] = img[3 * i + 1];
        b = imga[4 * i + 2] = img[3 * i + 2];
        imga[4 * i + 3] = ((r + g + b == 765) ? 0 : 255);
    }
    // Por ultimo le pedimos a glut que cree un mipmap a partir de la imagen rgba y 
    // liberamos la memoria reservada. Al creer el mipmap la textura queda lista para usarse
    // Cualquier primitiva de dibujo que aparezca a continuacion, si esta habilitada la 
    // generacion de texturas 2D (con glEnable(GL_TEXTURE_2D)) hara uso de esta textura
    delete[] img;
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, wt, ht, GL_RGBA, GL_UNSIGNED_BYTE, imga);
    delete[] imga;
    return true;
}

//============================================================

Raton raton;

Teclado teclado('w', 's', 'a', 'd', 'l', 'k', 'g', 'e', 'r');
double vel = 0; //velocidad, posicion y direccion del "personaje"

const double
zAVION = 0.5,
zMOTOR = 0.3,
zTORRE = 0.7,
zPROYECTIL = 0.75,
zCANON = 0.8,
zALA = 0.3,
zALERON = 0.35,
zPISO = -0.9,
zCABINA = 0.6,
zENEMIGO = 0.85,
zPALO = 0.9,
zRADAR = 0.85;

//============================================================

class Bala {
private:
    double x;
    double y;
    double incrementox;
    double incrementoy;
public:
    Bala(double posX, double posY, double incX, double incY) : x(posX), y(posY), incrementox(incX), incrementoy(incY) {}
    bool Update() {
        x += incrementox;
        y += incrementoy;
        if (fabs(x - EnemigoX) + fabs(y - EnemigoY) < 50) {
            Energia -= 10;
            return true;
        }
        //Si esta fuera de la pantalla, elimino la bala
        return (x > w || x < 0 || y > h || y < 0);
    }
    void Draw() {
        glVertex2d(x, y);
    }
};

list<Bala> proyectil;

// Dibuja el motor turbofan
void DibujarMotor() {
    glColor3f(0.576471, 0.439216, 0.858824);
    glBegin(GL_QUADS);
    glVertex2d(0, 0);
    glVertex2d(15, 0);
    glVertex2d(15, 10);
    glVertex2d(0, 10);
    glEnd();

    glPushMatrix();
    glTranslated(0, 0, zMOTOR);
    glTranslatef(7.5f, 7.5f, 0.0f);
    double baseRadius = 7.5; double startAngle = 270; double barridoAngle = 180;
    int lod = 30;
    int slices = lod, stacks = lod;
    GLUquadricObj* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    gluPartialDisk(q, 0, baseRadius, slices, stacks, startAngle, barridoAngle);
    gluDeleteQuadric(q);
    glPopMatrix();
}

void DibujarCabina() {
    glColor3d(0.196078, 0.6, 0.8);

    glPushMatrix();
    glTranslated(0, 0, zCABINA);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(0, 0);
    for (double r = 0; r < PI * 2; r += 0.1)
        glVertex2d(cos(r), sin(r));
    glVertex2d(1.0, 0.0);
    glEnd();
    glPopMatrix();
}

void DibujarCuerpo() {
    glColor3d(0.576471, 0.439216, 0.858824);

    glPushMatrix();
    glTranslated(0, 0, zAVION);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(0.0, 0.0);
    glVertex2d(0.0, 70.0);
    glVertex2d(-8, 35.0);
    glVertex2d(-8, -45.0);
    glVertex2d(-8, -55.0);
    glVertex2d(0.0, -65.0);
    glVertex2d(8, -55.0);
    glVertex2d(8, -45.0);
    glVertex2d(8, 35.0);
    glVertex2d(0.0, 70.0);
    glEnd();
    glPopMatrix();
}

void DibujarAla() {
    glColor3d(0.7, 0.7, 0.7);

    glPushMatrix();
    glTranslated(0, 0, zALA);
    glBegin(GL_TRIANGLE_FAN);

    glVertex2d(0.0, 20.0);
    glVertex2d(0.0, 0.0);
    glVertex2d(65, -10);
    glVertex2d(65, -10);
    glVertex2d(60.0, 0.0);
    glEnd();
    glPopMatrix();
}

void DibujarAleron() {
    glColor3d(0.7, 0.7, 0.7);

    glPushMatrix();
    glTranslated(0, 0, zALERON);
    glBegin(GL_TRIANGLE_FAN);

    glVertex2d(0.0, 20.0);
    glVertex2d(0.0, 0.0);
    glVertex2d(65, -10);
    glVertex2d(65, -10);
    glVertex2d(60.0, 0.0);
    glEnd();
    glPopMatrix();
}

// Dibuja el canion
void DibujarArma() {
    glColor3f(0.309804f, 0.18, 0.309804f);
    glTranslated(0, 0, zCANON);
    glBegin(GL_QUADS);
    glVertex2d(-3.0, 0.0);
    glVertex2d(3.0, 0.0);
    glVertex2d(3.0, 15.0);
    glVertex2d(-3.0, 15.0);
    glEnd();
}

// Dibuja la torre, que tiene radio 40
void DibujarTorre() {
    glColor3f(0.309804f, 0.184314f, 0.309804f);

    glPushMatrix();
    glTranslated(0, 0, zTORRE);

    double baseRadius = 9.0;
    int lod = 30;
    int slices = lod, stacks = lod;
    GLUquadricObj* q = gluNewQuadric();
    // GLU_FILL, GLU_LINE, GLU_POINT or GLU_SILHOUETTE
    gluQuadricDrawStyle(q, GLU_FILL);
    gluDisk(q, 0, baseRadius, slices, stacks);
    gluDeleteQuadric(q);

    glColor3f(0.7, 0.7, 0.7);
    glPointSize(2.0);
    glBegin(GL_POINTS);
    for (double x = 0; x < 2 * PI; x += 0.9)
        glVertex2d(9.0 * 0.8 * cos(x), 9.0 * 0.8 * sin(x));
    glEnd();
    glPopMatrix();

    glPushMatrix();
    glScaled(1.0, 2.0 + (ArmaTamanio * 0.1), 1.0);//glScaled(1.0,2.0,1.0);
    DibujarArma();
    glPopMatrix();
}


void DibujarAvion() {
    glPushMatrix();// inicio push1

    // Posiciona y rota el Avion en el modelo
    glTranslated(AvionX, AvionY, 0);
    glRotated(AvionAng, 0, 0, 1);

    //Dibujamos las distintas partes de la nave, aplicando las transformaciones necesarias



    glPushMatrix();
    // Posiciona el motor derecho superior
    glTranslatef(17.0f, 2.0f, 0.0f);
    DibujarMotor();
    glPopMatrix();

    glPushMatrix();
    // Posiciona el motor izquierdo superior
    glScaled(-1, 1, 1);
    glTranslatef(17.0f, 2.0f, 0.0f);
    //glTranslatef(-31.0f, 7.0f, 0.0f);
    DibujarMotor();
    glPopMatrix();

    if (luces_on) {
        glPushMatrix();
        // Posiciona el motor derecho inferior
        glTranslatef(40.0f, -5.0f, 0.0f);
        DibujarMotor();
        glPopMatrix();

        glPushMatrix();
        // Posiciona el motor izquierdo inferior
        glScaled(-1, 1, 1);
        glTranslatef(40.0f, -5.0f, 0.0f);
        //glTranslatef(-55.0f, 0.0f, 0.0f);
        DibujarMotor();
        glPopMatrix();
    }

    /*glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    DibujarLuces();
    glPopMatrix();
    glPushMatrix();
    glScaled(-1, 1, 1);
    glTranslatef(0.0f, 0.0f, 0.0f);
    DibujarLuces();
    glPopMatrix();
    */

    //Ala derecha
    glPushMatrix();
    DibujarAla();
    glPopMatrix();

    //Ala izquierda
    glPushMatrix();
    glScaled(-1, 1, 1); //Con este escalamiento logramos reflejar (x = -AnchoAla * x)  
    DibujarAla();
    glPopMatrix();



    glPushMatrix();
    // Posiciona el aleron derecho 
    glScaled(0.6, 0.6, 1);
    glTranslatef(8.0f, -80.0f, 0.0f);
    DibujarAleron();
    glPopMatrix();

    glPushMatrix();
    // Posiciona el aleron izquierdo
    glScaled(-0.6, 0.6, 1);
    glTranslatef(8.0f, -80.0f, 0.0f);
    DibujarAleron();
    glPopMatrix();



    //Cuerpo
    DibujarCuerpo();



    //Cabina
    glPushMatrix();
    glScaled(6, 12, 1);
    glTranslatef(0.0f, 2.0f, 0.0f);
    DibujarCabina();
    glPopMatrix();



    glPushMatrix();
    // Posiciona el cañon 
    glTranslatef(0.0f, -7.0f, 0.0f);
    glRotated(ArmaAng, 0, 0, 1);
    DibujarTorre();
    glPopMatrix();

    glPopMatrix();// fin push1
}

void DibujarPiso() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texid[0]); //selecciono la textura a utilizar
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2i(0, 0);
    glTexCoord2f(3, 0); glVertex2i(800, 0);
    glTexCoord2f(3, 3); glVertex2i(800, 600);
    glTexCoord2f(0, 3); glVertex2i(0, 600);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}
void DibujarLuna() {
    // Aplicacion de textura 2D
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, texid[1]);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(300, 400, 0);
    glTexCoord2f(1, 0); glVertex3f(300, 200, 0);
    glTexCoord2f(1, 1); glVertex3f(520, 200,0);
    glTexCoord2f(0, 1); glVertex3f(520, 400,0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}


void DibujarProyectiles() {
    glPushMatrix();
    glTranslated(0, 0, zPROYECTIL);

    list<Bala>::iterator p = proyectil.begin();
    glPointSize(8);
    glColor3d(0.196078, 0.6, 0.8);
    glBegin(GL_POINTS);
    while (p != proyectil.end()) {
        p->Draw();
        p++;
    }
    glEnd();

    glPopMatrix();
}

void DibujarEnemigo() {
    glPushMatrix();
    glTranslated(0, 0, zENEMIGO);

    glPushMatrix();
    glTranslated(EnemigoX, EnemigoY, 0);
    glColor3f(0.576471, 0.439216, 0.858824);
    GLdouble baseRadius = 40;
    int lod = 30;
    GLint slices = lod, stacks = lod;
    GLUquadricObj* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);
    gluDisk(q, 0, baseRadius, slices, stacks);
    gluDeleteQuadric(q);
    glColor3f(0, 0.1, 0);
    glPointSize(3);
    glBegin(GL_POINTS);
    for (double x = 0; x < 2 * PI; x += 0.4) {
        float _x = (glutGet(GLUT_ELAPSED_TIME) % 3000) / 2999.0f;
        float opacidadLucesitas = 315 / (64 * 3.1415 * pow(0.5, 9)) * pow((pow(0.5, 2) - pow(_x - 0.5, 2)), 3) / 12.5335 + 0.1f;
        if (opacidadLucesitas > 1.0f)
            opacidadLucesitas = 1.0f;
        glColor4f(1.0f, 1.0f, 0.5f, opacidadLucesitas);
        glVertex2d(60.0 * 0.8 * cos(x), 60.0 * 0.8 * sin(x));
    }
    glEnd();
    glPopMatrix();

    glPopMatrix();
}

void DibujarRadar() {
    glPushMatrix();
    glTranslated(0, 0, zRADAR);
    glTranslatef(700, 500, 0.0);
    glColor4f(0.3f, 0.3f, 0.3f, 0.5f);//glColor3f(0.8,0.5,0.1);//(0.0,0.0,0.0);
    glPointSize(1);
    GLUquadricObj* q = gluNewQuadric();
    gluQuadricDrawStyle(q, GLU_FILL);//GLU_POINT//GLU_FILL
    gluDisk(q, 85, 90, 30, 30);//gluDisk(q,0,baseRadius,slices,stacks);  

    glColor4f(0.309804f, 0.184314f, 0.309804f, 0.5f);
    gluQuadricDrawStyle(q, GLU_FILL);//GLU_POINT//GLU_FILL
    gluDisk(q, 0, 90, 30, 30);//gluDisk(q,0,baseRadius,slices,stacks);  
    gluDeleteQuadric(q);

    glPopMatrix();

    //linea
    glPushMatrix();
    glTranslated(0, 0, zPALO);
    glTranslatef(700, 500, 0.0);
    glRotatef(AngLineaRadar, 0.0, 0.0, -1.0);
    glColor3f(1.0f, 1.0f, 0.5f);//glColor3ub(1,245,0);
    glLineWidth(3);//glLineWidth(1.0);
    glBegin(GL_LINES);
    glVertex2i(0, 0); glVertex2i(85, 0);//glVertex2f(0.f,0.f); glVertex2f(0.f,50.f);
    glEnd();
    glPopMatrix();
}

//============================================================
// callbacks

//------------------------------------------------------------


// Renderiza texto en pantalla usando UglyFont
void print_text(string cadena, float x, float y, float escala = 1.0, float r = 1.0, float g = 1.0, float b = 1.0, float a = 1.0, float angulo = 0.0, int centrado = 0, float width = 1.0) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glLineWidth(width); // ancho de linea del caracter
    glColor4f(r, g, b, a); // color del caracter
    glPushMatrix();
    glTranslatef(x, y, 0);// posicion del caracter
    glScalef(escala, escala, escala);// escala del caracter
    glRotatef(angulo, 0, 0, 1); // angulo del caracter
    YsDrawUglyFont(cadena.c_str(), centrado); // el caracter puede estar centrado o no
    glPopMatrix();
    glPopAttrib();
}


// arma un un nuevo buffer (back) y reemplaza el framebuffer
void Display_cb() {
    // arma el back-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// rellena con color de fondo
    DibujarAvion();
    DibujarEnemigo();
    DibujarProyectiles();
    DibujarPiso();
    DibujarRadar();
    DibujarLuna();


    // segundos transcurridos
    char st1[30] = "Segundos Transcurridos: ";
    char st2[10] = "";
    sprintf_s(st2, "%d", 60-segundos);
    strcat_s(st1, st2); // cout << st1 << endl;  
    print_text(st1, 360, 580, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);


    // instrucciones
    char st3[30] = "Teclas a utilizar: ";
    print_text(st3, 5, 580, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    char st4[30] = "w: avanza";
    print_text(st4, 5, 560, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    char st5[30] = "s: retrocede";
    print_text(st5, 5, 540, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    char st6[30] = "d: gira en sentido horario";
    print_text(st6, 5, 520, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    char st7[31] = "a: gira en sentido antihorario";
    print_text(st7, 5, 500, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    char st8[30] = "l: estira arma";
    print_text(st8, 5, 480, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    char st9[30] = "k: contrae arma";
    print_text(st9, 5, 460, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    char st10[30] = "g: dispara";
    print_text(st10, 5, 440, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    char st11[34] = "e: aparece y desaparece las luces";
    print_text(st11, 5, 420, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    char st13[30] = " Vida enemigo: ";
    char st14[10] = "";
    sprintf_s(st14, "%d", Energia);
    strcat_s(st13, st14); // cout << st1 << endl;  
    print_text(st13, 360, 560, 10.0, 0.309804f, 0.184314f, 0.309804f, 1.0, 0.0, 0, 1.0);

    glutSwapBuffers(); // lo manda al monitor

    // chequea errores
    int errornum = glGetError();
    while (errornum != GL_NO_ERROR) {
        if (cl_info) {
            if (errornum == GL_INVALID_ENUM)
                cout << "GL_INVALID_ENUM" << endl;
            else if (errornum == GL_INVALID_VALUE)
                cout << "GL_INVALID_VALUE" << endl;
            else if (errornum == GL_INVALID_OPERATION)
                cout << "GL_INVALID_OPERATION" << endl;
            else if (errornum == GL_STACK_OVERFLOW)
                cout << "GL_STACK_OVERFLOW" << endl;
            else if (errornum == GL_STACK_UNDERFLOW)
                cout << "GL_STACK_UNDERFLOW" << endl;
            else if (errornum == GL_OUT_OF_MEMORY)
                cout << "GL_OUT_OF_MEMORY" << endl;
        }
        errornum = glGetError();
    }
}

//------------------------------------------------------------
// Animacion
void Idle_cb() {
    static unsigned int lt = 0;
    int dt = glutGet(GLUT_ELAPSED_TIME) - lt;
    if (dt > 30) {
        lt = glutGet(GLUT_ELAPSED_TIME);
        AngLineaRadar = (AngLineaRadar + 2) % 360;

        ArmaAng = -AvionAng - atan2(raton.getX() - AvionX, raton.getY() - AvionY) * 180 / PI;

        double ang = AvionAng * PI / 180.0;
        double ang3;

        if (teclado.Salir())
            exit(EXIT_SUCCESS);

        if (teclado.Adelante()) {
            AvionX -= 5 * sin(ang);
            AvionY += 5 * cos(ang);
        }

        if (teclado.Atras()) {
            AvionX += 5 * sin(ang);
            AvionY -= 5 * cos(ang);
        }

        if (teclado.Izquierda()) {
            AvionAng += 2;
        }

        if (teclado.Derecha()) {
            AvionAng -= 2;
        }

        if (teclado.Agrandar()) {
            if (ArmaTamanio < 10) ArmaTamanio += 1;
        }

        if (teclado.Achicar()) {
            if (ArmaTamanio > -3) ArmaTamanio -= 1;
        }

        if (teclado.Disparar()) {
            ang3 = (AvionAng + ArmaAng) * PI / 180.0;
            proyectil.push_back(Bala((AvionX), (AvionY), -30 * sin(ang3), 30 * cos(ang3)));//la bala sale desde la base del arma
        }

        if (teclado.Crear()) {
            luces_on = !luces_on;
        }



        // controlamos que no salga de la pantalla

        if (AvionX < 0) AvionX = 0;
        if (AvionX > w) AvionX = w;
        if (AvionY < 0) AvionY = 0;
        if (AvionY > h) AvionY = h;

        //Actualizamos las posiciones de los proyectiles
        list<Bala>::iterator p = proyectil.begin();
        while (p != proyectil.end()) {
            //Si esta fuera del mapa o impacta con el enemigo eliminamos el proyectil
            if (p->Update())
                p = proyectil.erase(p);
            else
                p++;
        }

        if (Energia < 1) {
            cout << "GANASTE!" << endl;
            exit(EXIT_SUCCESS);
        }
        glutPostRedisplay();
    }
    milisegundos = glutGet(GLUT_ELAPSED_TIME);
    if (milisegundos % 1000 == 0) {
            Display_cb();
            segundos = milisegundos / 1000;
            if (segundos == 60) {
             cout << "SE ACABO EL TIEMPO!" << endl;
             exit(EXIT_SUCCESS);
        }
    }


}


//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height) {
    // cout << "reshape " << width << "x" << height << endl;
    if (!width || !height) return; // minimizado ==> nada
    w = width; h = height;
    glViewport(0, 0, w, h); // región donde se dibuja (toda la ventana)
    // rehace la matriz de proyección (la porcion de espacio visible)
    glMatrixMode(GL_PROJECTION);  glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1); // unidades = pixeles
    // las operaciones subsiguientes se aplican a la matriz de modelado GL_MODELVIEW
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay(); // avisa que se debe redibujar
}

// Special keys (non-ASCII)
// teclas de funcion, flechas, page up/dn, home/end, insert
void Special_cb(int key, int xm = 0, int ym = 0) {
    if (key == GLUT_KEY_F4 && glutGetModifiers() == GLUT_ACTIVE_ALT) // alt+f4 => exit
        exit(EXIT_SUCCESS);
}

//------------------------------------------------------------
void inicializa() {
    // GLUT
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);// pide color RGB y double buffering
    glutInitWindowSize(w, h); glutInitWindowPosition(10, 10);
    glutCreateWindow("TP 3"); // crea el main window

    // declara los callbacks, los que (aun) no se usan (aun) no se declaran
    glutDisplayFunc(Display_cb);
    glutReshapeFunc(Reshape_cb);
    //glutKeyboardFunc(Keyboard_cb);
    glutSpecialFunc(Special_cb);

    glutIdleFunc(Idle_cb); // registra el callback

    ///
    raton.Iniciar();
    teclado.Iniciar();

    // Activamos el z buffer
    glEnable(GL_DEPTH_TEST); // Enable Depth Test
    glDepthFunc(GL_LEQUAL); // Specify the value used for depth buffer comparisons
    glDepthRange(0.0, 1.0); // glDepthRange(zNear, zFar) sets the mapping of the z values to [0.0,1.0]
    glClearDepth(1.0);


    // OpenGL
    glClearColor(0.23f, 0.20f, 0.01f, 1.f); // color de fondo
    //glClearColor(0.74902f, 0.847059f, 0.847059f, 1.f);
    //(0.22f, 0.69f, 0.87f,1.f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Textura 1: inicializacion
    //textura 2D
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(3, texid);
    glBindTexture(GL_TEXTURE_2D, texid[0]);
    mipmap_ppm("cosmos.ppm");
    //Utilizamos Repeat en lugar de clamp para hacer que la textura se repita
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    //Textura 2
//textura 2D
    glBindTexture(GL_TEXTURE_2D, texid[1]);
    mipmap_ppm("luna.ppm");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    //Textura 2
//textura 2D
    glBindTexture(GL_TEXTURE_2D, texid[2]);
    mipmap_ppm("ovni.ppm");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // las operaciones subsiguientes se aplican a la matriz de modelado GL_MODELVIEW
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

//------------------------------------------------------------
// main
int main(int argc, char** argv) {

    glutInit(&argc, argv); // inicialización interna de GLUT
    inicializa(); // define el estado inicial de GLUT y OpenGL
    glutMainLoop(); // entra en loop de reconocimiento de eventos
    return 0; // nunca se llega acá, es sólo para evitar un warning
}