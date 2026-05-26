#include "Lighting.h"
#include "gles.h"
#include "../Minecraft.h"

void Lighting::turnOff() {
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_COLOR_MATERIAL);
}

void Lighting::turnOn(Minecraft* minecraft) {
    (void)minecraft;
    glEnable(GL_NORMALIZE);
    // normal lighting path
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_COLOR_MATERIAL);
 //   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    constexpr float a = 0.4f;
    constexpr float d = 0.6f;
    constexpr float s = 0.0f;


    //Vec3 l = world::phys::Vec3{0.2, 1.0, -0.7}.normalize();
    constexpr float lightmodel_ambient[] = {a, a, a, 1.0f};
    constexpr float diffuse[] = {d, d, d, 1.0f};
    constexpr float ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    constexpr float specular[] = {s, s, s, 1.0f};

    constexpr float pos0[] = {(float)(0.16169041989141428), (float)(0.8084520874101966), (float)(-0.5659164515496377), 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

    constexpr float pos1[] = {(float)(-0.16169041989141428), (float)(0.8084520874101966), (float)(0.5659164515496377), 0.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
    glShadeModel(GL_FLAT);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lightmodel_ambient);
}
