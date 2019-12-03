#include "arena.h"

Arena::~Arena() {
    delete(player);
    
    delete(airstrip);

    for (Circle* c : groundEnemies) {
        delete(c);
    }

    for (Enemy* c : airEnemies) {
        delete(c);
    }
}

Arena::Arena(string path) : Circle() {
    /* Abertura SVG */
    XMLDocument doc;

    if (doc.LoadFile(path.c_str()) != XML_SUCCESS) {
        cout << "Erro ao tentar abrir o arquivo " << path << endl;
        exit(EXIT_FAILURE);
    }

    /* Raiz SVG */
    XMLElement* root = doc.RootElement();

    /* Busca exclusivamente a arena antes de todos os outros objetos do jogo */
    XMLElement* findBlueCircle = root->FirstChildElement("circle");

    /* 
     * Iremos compensar em X e Y os elementos da arena para que a mesma fique
     * na origem (0, 0)
     */
    GLfloat arenaX;
    GLfloat arenaY;

    while (findBlueCircle != NULL) {
        string colorName = findBlueCircle->Attribute("fill");

        if (!colorName.compare("blue")) {
            GLfloat radius = stof(findBlueCircle->Attribute("r"));
            arenaX = stoi(findBlueCircle->Attribute("cx"));
            arenaY = 2 * radius - stoi(findBlueCircle->Attribute("cy"));

            this->setCx(0.0);
            this->setCy(0.0);
            this->setRadius(radius);
            this->setColor(BLUE);

            break;
        }
    }

    /* Caso a arena nao seja encontrada o programa eh finalizado */
    if (findBlueCircle == NULL) {
        cout << "Nao foi possivel configurar a Arena" << endl;
        exit(EXIT_FAILURE);
    }

    /* Linha */
    XMLElement* l = root->FirstChildElement("line");

    GLfloat x1 = stoi(l->Attribute("x1")) - arenaX;
    GLfloat y1 = 2 * this->getRadius() - stoi(l->Attribute("y1")) - arenaY;
    GLfloat x2 = stoi(l->Attribute("x2")) - arenaX;
    GLfloat y2 = 2 * this->getRadius() - stoi(l->Attribute("y2")) - arenaY;

    airstrip = new Line(x1, y1, x2, y2);

    /* Player */
    XMLElement* findGreenCircle = root->FirstChildElement("circle");

    while (findGreenCircle != NULL) {
        string colorName = findGreenCircle->Attribute("fill");
        GLfloat radius = stof(findGreenCircle->Attribute("r"));
        GLfloat cx = stoi(findGreenCircle->Attribute("cx")) - arenaX;
        GLfloat cy = 2 * this->getRadius() - stoi(findGreenCircle->Attribute("cy")) - arenaY;

        if (!colorName.compare("green")) {
            player = new Player(this, cx, cy, radius);
            break;
        }

        findGreenCircle = findGreenCircle->NextSiblingElement("circle");
    }

    /* Caso o player nao seja encontrada o programa eh finalizado */
    if (findGreenCircle == NULL) {
        cout << "Nao foi possivel configurar a Arena" << endl;
        exit(EXIT_FAILURE);
    }

    /* Circulos */
    XMLElement* c = root->FirstChildElement("circle");

    while (c != NULL) {
        GLfloat radius = stof(c->Attribute("r"));
        GLfloat cx = stoi(c->Attribute("cx")) - arenaX;
        GLfloat cy = 2 * this->getRadius() - stoi(c->Attribute("cy")) - arenaY;

        string colorName = c->Attribute("fill");

        if (!colorName.compare("red")) {
            Enemy* enemy = new Enemy(this, cx, cy, radius);
            enemy->setSpeed(player->getSpeed());

            airEnemies.push_back(enemy);
        }

        if (!colorName.compare("orange")) {
            Circle* circle = new Circle(cx, cy, radius, ORANGE);

            groundEnemies.push_back(circle);
        }

        /* Next */
        c = c->NextSiblingElement("circle");
    }
}

void Arena::draw(GLuint arenaTexture1, GLuint arenaTexture2, GLuint playerTexture, GLuint airEnemiesTexture, GLuint groundEnemiesTexture, GLuint projTexture, GLuint bombTexture) {
    GLfloat materialEmission[] = {1.0, 1.0, 1.0, 1.0};
    glMaterialfv(GL_FRONT, GL_EMISSION, materialEmission);
    
    // Corpo do Cilindro
    glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
            glScalef(8.0, 2.0, 1.0);
            
            glBindTexture(GL_TEXTURE_2D, arenaTexture1);   
            GLUquadricObj* sky  = gluNewQuadric();
                glColor3f(1.0, 1.0, 1.0);
                gluQuadricDrawStyle(sky, GLU_FILL);
                gluQuadricNormals(sky, GLU_SMOOTH);
                gluQuadricTexture(sky, GLU_TRUE);
                gluQuadricOrientation(sky, GLU_INSIDE);
                gluCylinder(sky, this->getRadius(), this->getRadius(), this->getRadius(), 180, 1);
            gluDeleteQuadric(sky);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    glDisable(GL_TEXTURE_2D);
    
    glColor3f(0.0, 0.0, 1.0);
  
    // Tampa superior
    glPushMatrix();
        glTranslatef(.0, .0, this->getRadius());
        
        glEnable(GL_TEXTURE_2D);
            glMatrixMode(GL_TEXTURE);
            glPushMatrix();
                glScalef(8.0, 2.0, 1.0);
                
                glBindTexture(GL_TEXTURE_2D, arenaTexture2);
                GLUquadricObj* roof = gluNewQuadric();
                    glColor3f(1.0, 1.0, 1.0);
                    gluQuadricDrawStyle(roof, GLU_FILL);
                    gluQuadricNormals(roof, GLU_SMOOTH);
                    gluQuadricTexture(roof, GLU_TRUE);
                    gluQuadricOrientation(roof, GLU_OUTSIDE);
                    gluDisk(roof, 0, this->getRadius(), 180, 1);
                gluDeleteQuadric(roof);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
        glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Tampa inferior
    glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
            glScalef(10.0, 10.0, 1.0);
            
            glBindTexture(GL_TEXTURE_2D, arenaTexture2);
            GLUquadricObj* ground = gluNewQuadric();
                glColor3f(1.0, 1.0, 1.0);
                gluQuadricDrawStyle(ground, GLU_FILL);
                gluQuadricNormals(ground, GLU_SMOOTH);
                gluQuadricTexture(ground, GLU_TRUE);
                gluQuadricOrientation(ground, GLU_OUTSIDE);
                gluDisk(ground, 0, this->getRadius(), 180, 1);
            gluDeleteQuadric(ground);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    glDisable(GL_TEXTURE_2D);

    // Pista e inimigos terrestres no chao
    airstrip->drawSolidLine();

    for (auto ge : groundEnemies) {
        if (!ge->isDead()) {
            glPushMatrix();
                glTranslatef(ge->getCx(), ge->getCy(), .0);
                
                // Esfera da base
                glEnable(GL_TEXTURE_2D);
                    glMatrixMode(GL_TEXTURE);
                    glPushMatrix();
                        glScalef(1.0, 2.0, 1.0);

                        glBindTexture(GL_TEXTURE_2D, groundEnemiesTexture);
                        GLUquadricObj* base = gluNewQuadric();
                            glColor3f(1.0, 1.0, 1.0);
                            gluQuadricDrawStyle(base, GLU_FILL);
                            gluQuadricNormals(base, GLU_SMOOTH);
                            gluQuadricTexture(base, GLU_TRUE);
                            gluQuadricOrientation(base, GLU_OUTSIDE);
                            gluSphere(base, ge->getRadius(), 16, 16);
                        gluDeleteQuadric(base);
                    glPopMatrix();
                    glMatrixMode(GL_MODELVIEW);
                glDisable(GL_TEXTURE_2D);
                
                GLfloat materialEmission2[] = {0.1, 0.1, 0.1, 1.0};
                glMaterialfv(GL_FRONT, GL_EMISSION, materialEmission2);
                
                // Haste da bandeira
                glPushMatrix();
                    glTranslatef(0.0, 0.0, ge->getRadius());
                    
                    GLUquadricObj* stem = gluNewQuadric();
                        glColor3f(0.0, 0.0, 0.0);
                        gluQuadricDrawStyle(stem, GLU_FILL);
                        gluQuadricNormals(stem, GLU_SMOOTH);
                        gluQuadricTexture(stem, GLU_FALSE);
                        gluQuadricOrientation(stem, GLU_OUTSIDE);
                        gluCylinder(stem, ge->getRadius() / 12, ge->getRadius() / 12, ge->getRadius() / 1.5, 16, 16);
                    gluDeleteQuadric(stem);
                    
                    // Bandeira
                    glPushMatrix();
                        glTranslatef(0.0, ge->getRadius() / 8, ge->getRadius() / 1.5);
                        glScalef(1.0, ge->getRadius() / 4, ge->getRadius() / 8);
                        
                        glColor3f(1.0, 0.0, 0.0);
                        glutSolidCube(ge->getRadius() / 12);
                    glPopMatrix();
                glPopMatrix();
            glPopMatrix();
        }
    }

    player->drawAirplane(playerTexture, projTexture, bombTexture);
    
    for (auto ae : airEnemies) {
        if (!ae->isDead()) {
            ae->drawAirplane(airEnemiesTexture, projTexture);
        }
    }
}
