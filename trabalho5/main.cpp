#include <iostream>
#include <string>
#include <cmath>
#include <GL/glut.h>
#include <ctime>
#include "tinyxml2.h"
#include "config_reader.h"
#include "arena.h"
#include "imageloader.h"

using namespace std;
using namespace tinyxml2;

void cockpitCamera();
void cannonCamera();
void thirdPersonalCamera();
void bombCamera();
void changeCamera(GLdouble angle, GLdouble width, GLdouble height, GLdouble near, GLdouble far);

void renderBitmapString(float x, float y, void *font, string str);
GLuint loadTexture(const char* filename);

void init(void);
void display(void);
void idle(void);

void keyPress(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void mouseMovement(int x, int y);
void mouseAction(int button, int state, int x, int y);

bool keyboard[256];

ConfigReader* configReader;

Arena* arena;
Player* player;
list<Enemy*> airEnemies;
list<Circle*> groundEnemies;
GLfloat speedMultiplier;
GLfloat fireSpeedMultiplier;
GLfloat enemyFireSpeedMultiplier;
GLfloat enemySpeedMultiplier;
GLfloat enemyFireFreq;
GLfloat oldTimeTakeOff;
GLfloat oldTimeFlying;

GLfloat cameraEye[3];
GLfloat cameraLook[3];
GLfloat up[3];

GLfloat tpsAngleXZ;
GLfloat tpsAngleXY;

GLuint arenaTexture1;
GLuint arenaTexture2;
GLuint playerTexture;
GLuint airstripTexture;
GLuint airEnemiesTexture;
GLuint groundEnemiesTexture;
GLuint projTexture;
GLuint bombTexture;

int toggleCamera = 1;
bool nightMode = false;

int main(int argc, char** argv) {
    srand(time(NULL));

    /* Abertura e tratamento do arquivo de configuracao */
    if (argc <= 1) {
        cout << "Nao esqueça de especificar o arquivo de configuracao." << endl;
        exit(EXIT_FAILURE);
    }

    string configPath = strcat(argv[1], "/config.xml");

    ConfigReader& configReader = *(new ConfigReader(configPath));

    string svgPath = configReader.getCaminhoArquivoArena() + "/"
                    + configReader.getNomeArquivoArena() + "."
                    + configReader.getTipoArquivoArena();
    
    speedMultiplier = configReader.getVelocidadeJogador();
    fireSpeedMultiplier = configReader.getVelocidadeTiro();

    enemySpeedMultiplier = configReader.getInimigoVel();
    enemyFireSpeedMultiplier = configReader.getInimigoVelTiro();
    enemyFireFreq = configReader.getInimigoFreqTiro();

    arena = new Arena(svgPath);

    player = arena->getPlayer();

    /* Copia dos inimigos aereos para interacao */
    for (auto a : arena->getAirEnemies()) {
        airEnemies.push_back(a);
        a->setFireFreq(enemyFireFreq);
    }

    for (auto g : arena->getGroundEnemies()) {
        groundEnemies.push_back(g);
    }
    
    /* Glut */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(2 * arena->getRadius(), 2 * arena->getRadius());
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Ace Combat: Poor Edition");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyPress);
    glutKeyboardUpFunc(keyUp);
    glutIdleFunc(idle);
    glutPassiveMotionFunc(mouseMovement);
    glutMouseFunc(mouseAction);

    glutMainLoop();

    return 0;
}

void init(void) {
    /* Seleciona cor de fundo */
    glClearColor(1, 1, 1, 0);
    
    /* Inicializar matriz de texturas */
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    /* Inicializar sistema de viz */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    GLfloat aspectRatio = ((GLfloat) arena->getRadius() * 2) / ((GLfloat) arena->getRadius() * 2);
    gluPerspective(90, aspectRatio, 1, arena->getRadius() * 5);
    
    /* Carrega todas as texturas */
    arenaTexture1 = loadTexture("textures/sky.bmp");
    arenaTexture2 = loadTexture("textures/sand.bmp");
    playerTexture = loadTexture("textures/greencamo.bmp");
    airstripTexture = loadTexture("textures/road.bmp");
    airEnemiesTexture = loadTexture("textures/redcamo.bmp");
    groundEnemiesTexture = loadTexture("textures/wood.bmp");
    projTexture = loadTexture("textures/sun1.bmp");
    bombTexture = loadTexture("textures/sun1.bmp");
    
    /* Inicializa iluminacao */
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    glEnable(GL_LIGHT0);
    //glColorMaterial(GL_FRONT, GL_DIFFUSE);
    //glEnable(GL_COLOR_MATERIAL);
    
    GLfloat light_ambient[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat light_diffuse[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat light0_position[] = {arena->getRadius(), arena->getRadius(), arena->getRadius(), 0.0};
    GLfloat light1_position[] = {};
    GLfloat light1_direction[] = {};
    GLfloat light1_angle[] = {45.0};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
    
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
    
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_direction);
    //glLightfv(GL_LIGHT1, GL_SPOT_EXPONENT, light1_direction);
    glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF, light1_angle);
    
    GLfloat mat_specular[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat mat_shininess[] = {100.0};
 
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    cockpitCamera();
}

void display(void) {
    /* Limpar todos os pixels */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    /* Modo noturno */
    if (nightMode) {
        glDisable(GL_LIGHT0);
        
        GLfloat light_ambient[] = {0.0, 0.0, 0.0, 1.0};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
        //glEnable(GL_LIGHT1);
        
        //GLfloat light1_position[] = {arena->getPlayer()->getCx(), arena->getPlayer()->getCy(), arena->getPlayer()->getCz(), 0.0};
        //GLfloat light1_direction[] = {arena->getPlayer()->getCx() + 100, arena->getPlayer()->getCy() + 100, arena->getPlayer()->getCz(), 0.0};
        //GLfloat light1_angle[] = {45.0};
        
        //glLightfv(GL_LIGHT1, GL_POSITION, light1_position);
        //glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_direction);
        //glLightfv(GL_LIGHT1, GL_SPOT_EXPONENT, light1_direction);
        //glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF, light1_angle);
    } else {
        glEnable(GL_LIGHT0);
        
        GLfloat light_ambient[] = {0.2, 0.2, 0.2, 1.0};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
        //glDisable(GL_LIGHT1);
    }

    /* Camera do Cockpit */
    if (toggleCamera == 1) {
        cockpitCamera();

    /* Camera do Canhao */
    } else if (toggleCamera == 2) {
        cannonCamera();
    
    /* Camera em Terceira Pessoa */    
    } else if (toggleCamera == 3) {
        
    }

    gluLookAt(
        cameraEye[0], cameraEye[1], cameraEye[2],
        cameraLook[0], cameraLook[1], cameraLook[2],
        up[0], up[1], up[2]
    );

    arena->draw(arenaTexture1, arenaTexture2, playerTexture, airstripTexture, airEnemiesTexture, groundEnemiesTexture, projTexture, bombTexture, nightMode);

    if (player->getPoints() == arena->getGroundEnemies().size()) {
        string result("WIN");
        renderBitmapString(-20.0, -5.0, GLUT_BITMAP_TIMES_ROMAN_24, result);
    } else if (player->isDead()) {
        string result("LOSE");
        renderBitmapString(-20.0, -5.0, GLUT_BITMAP_TIMES_ROMAN_24, result);
    }
    
    string totalGroundEnemies(to_string(arena->getGroundEnemies().size() - player->getPoints()));
    string score("Destroyed: " 
            + to_string(player->getPoints()) 
            + " | Remaining: " 
            + totalGroundEnemies);

    renderBitmapString(0.0, arena->getRadius() - 30, GLUT_BITMAP_TIMES_ROMAN_24, score);

    /* Nao esperar! */
    glutSwapBuffers();
}

void keyPress(unsigned char key, int x, int y) {
    keyboard[key] = true;
    
    /* Modo noturno */
    if (key == 'n') {
        nightMode = not nightMode;
    }
}

void keyUp(unsigned char key, int x, int y) {
    keyboard[key] = false;
}

void idle(void) {
    /* Mudanca de camera */
    if (keyboard['1']) {
        toggleCamera = 1;
        cockpitCamera();

    } else if (keyboard['2']) {
        toggleCamera = 2;
        cannonCamera();
        
    } else if (keyboard['3']) {
        toggleCamera = 3;
        
    }
    
    // R
    if (keyboard['r']) {
        player->reset();

        for (auto a : airEnemies) {
            a->reset();
        }

        for (auto g : groundEnemies) {
            g->reset();
        }

        for (int i = 0; i < 256; i++) {
            keyboard[i] = false;
        }
    }

    if (player->getPoints() == arena->getGroundEnemies().size()) {
        return;
    }

    if (player->isDead()) {
        return;
    }

    GLfloat currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    GLfloat diffTime = currentTime - oldTimeFlying;
    oldTimeFlying = currentTime;

    // MOVIMENTACAO DOS INIMIGOS
    for (auto a : airEnemies) {
        if (!a->isDead()) {
            GLint turn = rand() % 18;
            GLint upDown = rand() % 24;

            if (turn <= 2) {
                a->moveX(120, diffTime);
            } else if (turn <= 6) {
                a->moveX(-120, diffTime);
            }

            if (upDown <= 4) {
                a->moveZ(30, diffTime);
            } else if (upDown <= 8) {
                a->moveZ(-30, diffTime);
            }

            a->move(enemySpeedMultiplier, diffTime);
            // atira só depois de x tempos
            a->fire(enemyFireSpeedMultiplier, enemySpeedMultiplier, diffTime);
            a->updateProjectiles(diffTime);
        }
    }

    if (player->isFlying()) {
        // A
        if (keyboard['a']) {
            player->moveX(120, diffTime);
        }

        // D
        if (keyboard['d']) {
            player->moveX(-120, diffTime);
        }

        if (!keyboard['s'] && !keyboard['w']) {
            if (player->getAngleTheta() > 2) {
                player->moveZ(-90, diffTime);
            } else if (player->getAngleTheta() < -2) {
                player->moveZ(90, diffTime);
            } else {
                player->setAngleTheta(.0);
            }
        } else {
            if (keyboard['w']) {
                player->moveZ(90, diffTime);
            }

            if (keyboard['s']) {
                player->moveZ(-90, diffTime);
            }
        }

        if (keyboard['='] || keyboard['+']) {
            player->setSpeed(player->getSpeed() + 200.0 * diffTime);
        }

        if (keyboard['-'] || keyboard['_']) {
            GLfloat speed = player->getSpeed() - 200.0 * diffTime;

            if (speed <= 0) {
                speed = 0;
            }

            player->setSpeed(speed);
        }

        player->updateProjectiles(diffTime);
        player->updateBombs(currentTime, diffTime);
        player->move(speedMultiplier, diffTime);

    } else {
        if (keyboard['u'] && !player->isTakeOff()) {
            player->setTakeOff(true);
            oldTimeTakeOff = glutGet(GLUT_ELAPSED_TIME);
        }

        if (player->isTakeOff()) {
            GLint currentTime = glutGet(GLUT_ELAPSED_TIME) - oldTimeTakeOff;

            player->takeOffAirplane(currentTime, diffTime);

            if (currentTime > 4000) {
                player->setTakeOff(false);
                player->setFlying(true);
                oldTimeFlying = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
            }
        }
    }

    glutPostRedisplay();
}

void mouseMovement(int x, int y) {
    if (!player->isTakeOff() && player->isFlying() && !player->isDead()) {
        GLfloat distanceX = player->getMouseX() - x;
        GLfloat distanceY = player->getMouseY() - y;

        if (player->getCannonAngle() + distanceX >= -30 && player->getCannonAngle() + distanceX <= 30) {
            player->setCannonAngle(player->getCannonAngle() + distanceX / 3);
        }

        if (player->getCannonAngleTheta() + distanceY >= -30 && player->getCannonAngleTheta() + distanceY <= 30) {
            player->setCannonAngleTheta(player->getCannonAngleTheta() + distanceY / 3);
        }
    }

    player->setMouseX(x);
    player->setMouseY(y);
}

void mouseAction(int button, int state, int x, int y) {
    if (player->isFlying() && !player->isTakeOff() && !player->isDead()) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            player->fire(fireSpeedMultiplier, speedMultiplier);
        }

        if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
            player->bomb(speedMultiplier);
        }
    }
}

void cockpitCamera() {
    GLfloat fiRad = player->getAngle() * M_PI / 180;
    GLfloat thetaRad = player->getAngleTheta() * M_PI / 180;

    cameraEye[0] = player->getCx() + player->getRadius() * cos(thetaRad + M_PI / 4) * cos(fiRad);
    cameraEye[1] = player->getCy() + player->getRadius() * cos(thetaRad + M_PI / 4) * sin(fiRad);
    cameraEye[2] = player->getCz() + player->getRadius() * sin(thetaRad + M_PI / 4);

    cameraLook[0] = player->getCx() + 2 * player->getRadius() * cos(thetaRad) * cos(fiRad);
    cameraLook[1] = player->getCy() + 2 * player->getRadius() * cos(thetaRad) * sin(fiRad);
    cameraLook[2] = player->getCz() + 2 * player->getRadius() * sin(thetaRad);

    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
}

void cannonCamera() {
    GLfloat airplaneFiRad = player->getAngle() * M_PI / 180;
    GLfloat airplaneThetaRad = player->getAngleTheta() * M_PI / 180;

    GLfloat cannonFiRad = player->getCannonAngle() * M_PI / 180;
    GLfloat cannonThetaRad = player->getCannonAngleTheta() * M_PI / 180;

    GLfloat px = player->getCx();
    px += (player->getRadius()) * cos(airplaneThetaRad) * cos(airplaneFiRad);
    cameraEye[0] = px + player->getRadius() / 2 * cos(cannonThetaRad + airplaneThetaRad) * cos(cannonFiRad + airplaneFiRad);
    px += (player->getRadius() / 2 + 10) * cos(cannonThetaRad + airplaneThetaRad) * cos(cannonFiRad + airplaneFiRad);
    cameraLook[0] = px;
    
    GLfloat py = player->getCy();
    py += (player->getRadius()) * cos(airplaneThetaRad) * sin(airplaneFiRad);
    cameraEye[1] = py + player->getRadius() / 2 * cos(cannonThetaRad + airplaneThetaRad) * sin(cannonFiRad + airplaneFiRad);
    py += (player->getRadius() / 2 + 10) * cos(cannonThetaRad + airplaneThetaRad) * sin(cannonFiRad + airplaneFiRad);
    cameraLook[1] = py;

    GLfloat pz = player->getCz();
    pz += (player->getRadius()) * sin(airplaneThetaRad);
    cameraEye[2] = pz + player->getRadius() / 2 * sin(cannonThetaRad + airplaneThetaRad);
    pz += (player->getRadius() / 2 + 10) * sin(cannonThetaRad + airplaneThetaRad);
    cameraLook[2] = pz;

    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
}

void thirdPersonalCamera() {
    GLfloat fiRad = player->getAngle() * M_PI / 180;
    GLfloat thetaRad = player->getAngleTheta() * M_PI / 180;

    cameraEye[0] = player->getCx() + player->getRadius() * cos(thetaRad + M_PI / 4) * cos(fiRad);
    cameraEye[1] = player->getCy() + player->getRadius() * cos(thetaRad + M_PI / 4) * sin(fiRad);
    cameraEye[2] = player->getCz() + player->getRadius() * sin(thetaRad + M_PI / 4);

    cameraLook[0] = player->getCx();
    cameraLook[1] = player->getCy();
    cameraLook[2] = player->getCz();

    up[0] = 0;
    up[1] = 0;
    up[2] = 1;
}

void changeCamera(GLdouble angle, GLdouble width, GLdouble height, GLdouble near, GLdouble far) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(angle, width / height, near, far);

    glMatrixMode(GL_MODELVIEW);
}

void renderBitmapString(float x, float y, void *font, string str) {
    glRasterPos2f(x,y);

    for (string::iterator c = (&str)->begin(); c != (&str)->end(); ++c)  {
        glutBitmapCharacter(font, *c);
    }
}

GLuint loadTexture(const char* filename) {
    GLuint texture;
    
    Image* image = loadBMP(filename);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,                // Always GL_TEXTURE_2D
                 0,                            // 0 for now
                 GL_RGB,                       // Format OpenGL uses for image
                 image->width, image->height,  // Width and height
                 0,                            // The border of the image
                 GL_RGB,                       // GL_RGB, because pixels are stored in RGB format
                 GL_UNSIGNED_BYTE,             // GL_UNSIGNED_BYTE, because pixels are stored as unsigned numbers
                 image->pixels);               // The actual pixel data
                 
    delete image;

    return texture;
}
