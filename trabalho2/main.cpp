#include <iostream>
#include <string>
#include <cmath>
#include <GL/glut.h>
#include "tinyxml2.h"
#include "config_reader.h"
#include "arena.h"

using namespace std;
using namespace tinyxml2;

void init(void);
void display(void);
void idle(void);

void keyPress(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);

bool keyboard[256];

Arena* arena;
Player* player;
GLfloat speedMultiplier;
GLfloat oldTimeTakeOff;
GLfloat oldTimeFlying;

int main(int argc, char** argv) {
    /* Abertura e tratamento do arquivo de configuracao */
    if (argc <= 1) {
        cout << "Nao esqueça de especificar o arquivo de configuracao." << endl;
        exit(EXIT_FAILURE);
    }

    string configPath = strcat(argv[1], "/config.xml");

    ConfigReader configReader(configPath);

    string svgPath = configReader.getCaminhoArquivoArena() + "/"
                    + configReader.getNomeArquivoArena() + "."
                    + configReader.getTipoArquivoArena();

    arena = new Arena(svgPath);

    player = &arena->getPlayer();
    player->calculatePhysics();
    
    speedMultiplier = configReader.getVelocidadeJogador();

    /* Glut */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(2 * arena->getRadius(), 2 * arena->getRadius());
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ace Combat: Poor Edition");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyPress);
    glutKeyboardUpFunc(keyUp);
    glutIdleFunc(idle);

    oldTimeFlying = glutGet(GLUT_ELAPSED_TIME) / 1000.0;

    glutMainLoop();

    return 0;
}

void init(void) {
    /* Seleciona cor de fundo */
    glClearColor(1, 1, 1, 0);

    /* Inicializar sistema de viz */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(
        arena->getCx() - arena->getRadius(),
        arena->getCx() + arena->getRadius(),
        arena->getCy() - arena->getRadius(),
        arena->getCy() + arena->getRadius(), -1.0, 1.0);
}

void display(void) {
    /* Limpar todos os pixels */
    glClear(GL_COLOR_BUFFER_BIT);

    arena->draw();

    /* Nao esperar! */
    glutSwapBuffers();
}

void keyPress(unsigned char key, int x, int y) {
    keyboard[key] = true;
}

void keyUp(unsigned char key, int x, int y) {
    keyboard[key] = false;
}

void idle(void) {
    if (player->isFlying()) {
        GLfloat currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
        GLfloat diffTime = currentTime - oldTimeFlying;
        oldTimeFlying = currentTime;

        GLfloat step = speedMultiplier * diffTime;

        bool isDiagonal = false;

        if (keyboard['w'] && keyboard['a']) {
            isDiagonal = true;
            player->moveXY(-step, step);
        }

        if (keyboard['w'] && keyboard['d']) {
            isDiagonal = true;
            player->moveXY(step, step);
        }

        if (keyboard['a'] && keyboard['s']) {
            isDiagonal = true;
            player->moveXY(-step, -step);
        }

        if (keyboard['s'] && keyboard['d']) {
            isDiagonal = true;
            player->moveXY(step, -step);
        }

        if (!isDiagonal) {
            if (keyboard['w']) {
                player->moveY(step);
            }

            if (keyboard['a']) {
                player->moveX(-step);
            }

            if (keyboard['s']) {
                player->moveY(-step);
            }

            if (keyboard['d']) {
                player->moveX(step);
            }
        }
    } else {
        if (keyboard['u'] && !player->isTakeOff()) {
            player->setTakeOff(true);
            oldTimeTakeOff = glutGet(GLUT_ELAPSED_TIME);
        }

        if (player->isTakeOff()) {
            int currentTime = glutGet(GLUT_ELAPSED_TIME) - oldTimeTakeOff;

            player->takeOffAirplane(currentTime);

            if (currentTime > 4000) {
                player->setTakeOff(false);
                player->setFlying(true);
            }
        }
    }

    glutPostRedisplay();
}
