#ifndef towerCl
#define towerCl

#include "../object.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "math.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../loadData/loadOBJ.hpp"

class TowerLight : public Object{
public:
    static TowerLight *parent;
   void moveLight();
   TowerLight(char*filename,Object *o, float startA, float endA, int dir);
   TowerLight(char*filename);
   ~TowerLight();
    void copyParent();
   int direction;
   float startA;
   float endA;
   glm::vec4 lightDir;
   void draw(glm::mat4 projection, glm::mat4 modelView,glm::mat4 lights,glm::vec4 sun);
};

class Tower : public Object{
public:
    static Tower *parent;
   Tower(char *filename, int posX, int posY, int posZ, int startAngle, int endAngle);
   Tower(char *filename);
    void copyParent();
   ~Tower();
   void draw(glm::mat4 projection, glm::mat4 modelView, glm::mat4 lights,glm::vec4 sun);
   TowerLight *light;
};

#endif
