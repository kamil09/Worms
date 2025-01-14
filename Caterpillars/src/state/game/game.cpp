#include "game.hpp"
#include <typeinfo>
#include "../../inputActions.hpp"

Caterpillar* Game::currentCutterpillar = nullptr;
const int NUMBERS_OF_PARTICLES = 50000;

Game::Game(GLFWwindow *window,GLFWcursor *cur) : State(window,cur){
   this->sunPosition=glm::vec4(vertX/2,700,vertY/2,1.0f);

   this->map= & Map::getInstance();
   this->wall = new Wall((char*)"../src/obj/wall.obj",0,vertX,0,vertY,0,maxMapHeight*1.3);
   this->targetView = new Sprite(-30, -30, 60, 60, (char *) "../src/img/target-viewfinder.png");
   float roseWidth, roseHeight;
   roseWidth = roseHeight = 200;
   this->rose = new Sprite(-roseWidth / 2, -roseHeight / 2, roseWidth, roseHeight, (char *) "../src/img/rose.png");
   //std::cout << "Width: " << this->windowXsize <<  " rW: " << (this->windowXsize-roseWidth)/2 << " height: " << this->windowYsize << " rH " << -(this->windowYsize-roseHeight)/2;

   this->rose->setTraM((1366.0f-roseWidth)/2,-(768.0f-roseHeight)/2,-0.9f);
   this->projection = glm::perspective(45.0f, (float)this->windowXsize/this->windowYsize , 0.001f, 1000.0f);
   this->createPlayers();

    float min1=999;
    float min2=999;
    for(int i=0;i<100; i++) for(int k=0;k<100;k++) if (this->map->mapVert[i][k]<min1) min1=this->map->mapVert[i][k];
    for(int i=vertX-100;i<vertX; i++) for(int k=vertY-100;k<vertY;k++) if (this->map->mapVert[i][k]<min2) min2=this->map->mapVert[i][k];
    this->towers.push_back(new Tower ((char*)"../src/obj/tower.obj" ,50,min1,50,80,200 ));
    this->towers.push_back(new Tower ((char*)"../src/obj/tower.obj", vertX-50,min2,vertY-50,260,380) );
    inputActions::getInstance().objectPointers.push_back(this->towers[0]);
    inputActions::getInstance().objectPointers.push_back(this->towers[1]);

    this->lightsMat = glm::mat4(0);
    if(this->towers.size() > 0 ) this->lightsMat[0] = glm::vec4(this->towers[0]->pos.x,this->towers[0]->light->pos.y,this->towers[0]->pos.z,1.0f);
    if(this->towers.size() > 1 ) this->lightsMat[2] = glm::vec4(this->towers[1]->pos.x,this->towers[1]->light->pos.y,this->towers[1]->pos.z,1.0f);

    this->lookFrom=glm::vec3(0, 400, 0);
    this->lookAt=glm::vec3(150,0,150);


   glfwSetCursorPos(window,this->windowXsize/2,this->windowYsize/2);
   inputActions::getInstance().cursorFixedCenterPos=true;
    this->shotPower = this->minShotPower;
    this->font = new Font("../src/fonts/Coalition.ttf",32);
    this->font->posM[3][2] = -1.0f;
    float barWidth, barHeight;
    barWidth = 300.0f;
    barHeight = 40.0f;

    this->powerBar = new PowerBar(0.0f,0.0f,barWidth,barHeight,"../src/img/gradient.png");
    this->powerBar->setTraM(-1366.0f/2.0f + 10.0f,-768.0f/2.0f + this->font->height(1.0f) + 20.0f,0.0f);
    this->powerBar->font = new Font("../src/fonts/Coalition.ttf",26);
    this->powerBar->font->posM[3][2] = -0.9f;
    this->powerBar->addTextM("Power: 0%",0.0f,0.0f,1.0f,glm::vec3(1.0f,1.0f,1.0f));
    this->skybox = new Skybox();

    inputActions::getInstance().SoundEngine->play2D("../src/sounds/song_01.wav", GL_TRUE);
}


void Game::createPlayers() {
    //Dodawanie Caterpillarow
    Font *font  = new Font("../src/fonts/Coalition.ttf",400,this->projection);
    for(unsigned int i=0;i< Setting::getInstance().players.size();i++) {
        if(Setting::getInstance().players[i].czyGra){
            Player *newPlayer = new Player(i);
            this->players.push_back(newPlayer);
            this->alivePlayers.push_back(newPlayer);
            for (unsigned int j = 0; j < newPlayer->aliveCaterpillars.size(); j++) {
                Caterpillar *cat = newPlayer->aliveCaterpillars[j];
                cat->font = font;
                cat->setPos(rand() % vertX/2+(vertY/4),maxMapHeight + 200,rand() % vertY/2+(vertY/4)); // Tutaj usunac 200 Pawelek
                this->caterrVec.push_back( cat );
                inputActions::getInstance().objectPointers.push_back(cat);
            }
        }
    }
    this->changePlayer();
}


void Game::changePlayer() {
    int nextPlayer = this->activePlayer + 1;
    if(this->activePlayer > -1 && this->currentCutterpillar != nullptr){
        if(this->currentCutterpillar->dead != 0){
            int temp = this->alivePlayers[this->activePlayer]->activeCaterpillar;
            if(this->alivePlayers[this->activePlayer]->changeCaterpillar() == nullptr){
                this->alivePlayers.erase(this->alivePlayers.begin() + this->activePlayer);
            }
            else{
                this->alivePlayers[this->activePlayer]->activeCaterpillar = temp;
            }
        }
    }

    nextPlayer = nextPlayer % this->alivePlayers.size();
    Caterpillar *nextCat = nullptr;
    while(nextCat== nullptr){
        nextCat = this->alivePlayers[nextPlayer]->changeCaterpillar();
        if(nextCat== nullptr){
            this->alivePlayers.erase(this->alivePlayers.begin()+nextPlayer);
            nextPlayer = nextPlayer % this->alivePlayers.size();
        }
    }
    if(nextCat!= nullptr){
        if(this->alivePlayers.size() == 1){
            inputActions::getInstance().winner = this->alivePlayers[this->activePlayer];
        }
        this->activePlayer = nextPlayer;
        this->currentTime = this->maxTime;
        this->currentCutterpillar = nextCat;
        this->map->rand();
    }
}


//int Game::currCatIndex;

void Game::draw(){
   if(this->towers.size() > 0 ) this->lightsMat[1] = this->towers[0]->light->lightDir;
    if(this->towers.size() > 1 ) this->lightsMat[3] = this->towers[1]->light->lightDir;
    this->modelView = glm::lookAt(this->lookFrom, this->lookAt, glm::vec3(0.0f, 1.0f, 0.0f));
   this->skybox->draw(this->projection,this->modelView);
   this->map->draw(this->projection,this->modelView, this->lightsMat,this->sunPosition);
   this->wall->draw(this->projection,this->modelView, this->lightsMat,this->sunPosition);

    for(unsigned int i=0;i < this->caterrVec.size(); i++){
        if( (this->caterrVec[i] != this->currentCutterpillar) || (this->currentCutterpillar->viewBack < -20) || (!this->bullets.empty())){
            if(!this->caterrVec[i]->colission){
                this->caterrVec[i]->draw(this->projection,this->modelView,this->lightsMat,this->sunPosition);
            }
            else {
               Caterpillar *toErase = this->caterrVec[i];
               std::vector<Caterpillar*>::iterator itC = std::find(this->caterrVec.begin(),this->caterrVec.end(),toErase);
              if(itC != this->caterrVec.end()) this->caterrVec.erase(itC);

              std::vector<Object*>::iterator itO = std::find(inputActions::getInstance().objectPointers.begin(),inputActions::getInstance().objectPointers.end(),toErase);
              if(itO != inputActions::getInstance().objectPointers.end()){
              inputActions::getInstance().objectPointers.erase(itO);
           }
        }
        }
    }

    for(int i=0;i<(int)this->towers.size();i++ )
      this->towers[i]->draw(this->projection,this->modelView,this->lightsMat,this->sunPosition);

      if((int)this->bullets.size() > 0)
      {
        for(int i=0; i < (int)this->bullets.size(); i++)
        {
            if(!this->bullets[i]->colission)
               this->bullets[i]->draw(this->projection,this->modelView,this->lightsMat,this->sunPosition);
            else{
                if(this->bullets[i]->currentWaitTime <= 0.0f){
                    this->bullets.erase(std::remove(this->bullets.begin(), this->bullets.end(), this->bullets[i]), this->bullets.end());
                    inputActions::getInstance().objectPointers.erase(std::remove(inputActions::getInstance().objectPointers.begin(), inputActions::getInstance().objectPointers.end(), this->bullets[i]), inputActions::getInstance().objectPointers.end());
                    this->changePlayer();
                }
            }
         }
       }
   if(this->bullets.empty()) draw2D();
   for(unsigned int i =0; i<Map::getInstance().particleEffectsVector.size();i++ )
      Map::getInstance().particleEffectsVector[i]->draw(this->projection,this->modelView);

}

void Game::draw2D() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if(!(currentCutterpillar->viewBack < -20)) targetView->draw();

    this->drawRose();
    float margines = 10.0f;
    std::string czas =std::to_string((int)this->currentTime) + " s";
    font->print(czas,this->rose->posM[3][0]-50.0f,-this->rose->posM[3][1]-this->font->height(1.0f) - this->rose->size.y/2.0f - 5.0f,1.0f,glm::vec3(0.988, 0.408, 0.0));
    font->print(currentCutterpillar->getLife(), -1366.0f / 2.0f + margines, 768.0f / 2.0f - font->height(1.0f) - margines, 1.0f,
                currentCutterpillar->player->kolor);
    font->print(currentCutterpillar->player->nazwa, -font->length(currentCutterpillar->player->nazwa, 1.0f) / 2.0f, 768.0f / 2.0f - font->height(1.0f) - margines, 1.0f,
                currentCutterpillar->player->kolor);
    powerBar->teksty[0]->text = "Power: " + to_string(this->procentShotPower()) + "%";
    powerBar->draw(this->procentShotPower());
    glDisable(GL_BLEND);
}

void Game::drawRose(){
   glm::vec2 look,wind;
   look.x=this->lookAt.x-this->lookFrom.x;
   look.y=this->lookAt.z-this->lookFrom.z;
   wind.x=this->map->windForce.x;
   wind.y=this->map->windForce.z;
   look=glm::normalize(look);
   wind=glm::normalize(wind);
   double lookD = sqrt( (look.x*look.x)+(look.y*look.y) );
   double windD = sqrt( (wind.x*wind.x)+(wind.y*wind.y) );
   double cosK;

   cosK= ((look.x*wind.x) + (look.y*wind.y)) / (lookD*windD);

   double kat = -acos(cosK);
   float p = look.x*wind.y-look.y*wind.x;
   if(p>0) kat*=-1;

   if(kat!=kat) kat=0;
   glm::mat4 rotM = glm::mat4(
      glm::vec4(cos(kat),-sin(kat),0.0f,0.0f),
      glm::vec4(sin(kat),cos(kat),0.0f,0.0f),
      glm::vec4(0.0f,0.0f,1.0f,0.0f),
      glm::vec4(0.0f,0.0f,0.0f,1.0f)
   );

   this->rose->rotM=rotM;
   this->rose->draw();
}


void Game::changeTime() {
    if(this->bullets.empty()){
        if(this->currentTime <=0.0f){
            if(!this->powerischoosed){
                this->changePlayer();
            }
        }
        else{
            this->currentTime -= inputActions::getInstance().deltaTime;
        }
    }
}



void Game::run(){
    if(inputActions::getInstance().winner!= nullptr){
        this->endGame();
        return;
    }
    this->changeTime();

   if(inputActions::getInstance().SHIFT_pressed) this->testViewMov();
   else {
      this->catterMove();
      this->calcViewMatrix();
       this->bulletShot();
   }
   this->draw();

   for(int i=0;i < (int)this->caterrVec.size(); i++){
       this->caterrVec[i]->recalculateGravity(0.35);
      this->caterrVec[i]->recalculateMatrix();
   }
   for(int i=0; i< (int)this->towers.size();i++ )
     this->towers[i]->light->moveLight();

   //Pociski
   if((int)this->bullets.size() > 0)
   {
     for(int i=0; i < (int)this->bullets.size(); i++){
         if(this->bullets[i]->currentWaitTime <= 0.0f){
             this->bullets[i]->recalculateRotZ();
             this->bullets[i]->recalculateGravity(0.8);
         }
         this->bullets[i]->recalculateMatrix();
         if(this->bullets[i]->currentWaitTime > 0.0f){
             this->bullets[i]->currentWaitTime -= inputActions::getInstance().deltaTime;
         }
      }
    }
    for(int unsigned i =0; i<Map::getInstance().particleEffectsVector.size();i++ ) {
      if(Map::getInstance().particleEffectsVector[i]->effectTimeLeft>0)
         Map::getInstance().particleEffectsVector[i]->run();
      else{
         ParticleEffect *tmp = Map::getInstance().particleEffectsVector[i];
         Map::getInstance().particleEffectsVector.erase(std::remove(Map::getInstance().particleEffectsVector.begin(), Map::getInstance().particleEffectsVector.end(), Map::getInstance().particleEffectsVector[i]), Map::getInstance().particleEffectsVector.end());
         delete tmp;
         puts("deleted particle effect");
      }
    }
}


void Game::calcViewMatrix(){
    if(!this->bullets.empty()){
        this->lookAt = this->bullets[0]->pos;
        //Wektor prostopadły do wektora i punktu?
        //glm::vec3 perpendicular = glm::cross(this->lookAt,glm::vec3(0.0f,1.0f,0.0f));
        if( (this->bullets[0]->speed.x!=0 ) || (this->bullets[0]->speed.z!=0) )
           this->perpendicular = glm::cross(
              glm::vec3(this->bullets[0]->speed.x,0,this->bullets[0]->speed.z),
              glm::vec3(0.0f,1.0f,0.0f)
           );
        glm::vec3 back = glm::normalize(this->perpendicular);

        this->lookFrom = this->lookAt+back;
        if(this->bullets[0]->currentWaitTime <= 0.0f){
            this->lookFrom = this->lookAt + (back * 10.0f);
            this->lookFrom.y += 5.0f;
        }
        else{
            float speed  = 6.5f;
            float delta = this->bullets[0]->waitTime - this->bullets[0]->currentWaitTime;
            this->lookFrom = this->lookAt + (back * (10.0f + speed*delta));
            this->lookFrom.y += 5.0f + speed*(delta);
        }
    }
    else{
        this->lookAt = this->currentCutterpillar->startLook;
        this->lookAt = glm::mat3(this->currentCutterpillar->rotM) * this->lookAt;

        this->lookFrom = this->currentCutterpillar->pos;
        this->lookAt+=this->currentCutterpillar->pos;

        this->lookFrom.y+= 6*this->currentCutterpillar->size.y;
        this->lookAt.y+= 6*this->currentCutterpillar->size.y;

        glm::vec3 look = this->lookAt - this->lookFrom;
        this->lookFrom += glm::normalize(look)*this->currentCutterpillar->viewBack;
    }
}
void Game::catterMove(){
    if(!this->bullets.empty()) return;

	float diff = inputActions::getInstance().deltaTime;
   glm::vec3 catViewVec = this->currentCutterpillar->startLook;
   glm::mat3 rotY = glm::mat3(
		glm::vec3(cos(this->currentCutterpillar->rot.y),0.0f, sin(this->currentCutterpillar->rot.y)),
		glm::vec3(0.0f,1.0f,0.0f),
		glm::vec3(-sin(this->currentCutterpillar->rot.y),0.0f,cos(this->currentCutterpillar->rot.y))
	);

   catViewVec = rotY * catViewVec;
   glm::vec3 prosVec=catViewVec;
   prosVec[1]=0;
   float tmp=prosVec[0];
   prosVec[0]=prosVec[2];
   prosVec[2]=tmp;
   prosVec[2]!=0? prosVec[2]=-prosVec[2] : prosVec[0]=-prosVec[0];

   glm::vec3 newPos = this->currentCutterpillar->pos;
   if(!inputActions::getInstance().space_pressed && this->currentCutterpillar->on_the_ground){

     if(inputActions::getInstance().w_pressed){
        glm::vec3 add = glm::normalize(catViewVec);
        newPos+=add*diff*this->currentCutterpillar->maxWalkSpeed*2.0f;
     }
     if(inputActions::getInstance().s_pressed){
        glm::vec3 add = glm::normalize(catViewVec);
        newPos-=add*diff*this->currentCutterpillar->maxWalkSpeed*2.0f;
     }
     if(inputActions::getInstance().a_pressed){
        glm::vec3 add = glm::normalize(prosVec)*2.0f;
        newPos+=add*diff*this->currentCutterpillar->maxWalkSpeed*1.0f;
     }
     if(inputActions::getInstance().d_pressed){
        glm::vec3 add = glm::normalize(prosVec)*2.0f;
        newPos-=add*diff*this->currentCutterpillar->maxWalkSpeed*1.0f;
     }
   }
   else if(inputActions::getInstance().space_pressed){
     if(this->currentCutterpillar->on_the_ground)
     {
      int jump_coefficient = 15.0f;
      glm::vec3 shot;
      //skok w przod
      if(inputActions::getInstance().w_pressed){
         shot.x = catViewVec.x * jump_coefficient;
         shot.z = catViewVec.z * jump_coefficient;
      }
      //skok w tyl
      if(inputActions::getInstance().s_pressed){
        shot.x = catViewVec.x * -jump_coefficient;
        shot.z = catViewVec.z * -jump_coefficient;
      }
      if(inputActions::getInstance().a_pressed){
         shot.x = prosVec.x * jump_coefficient;
         shot.z = prosVec.z * jump_coefficient;
      }
      //skok w tyl
      if(inputActions::getInstance().d_pressed){
        shot.x = prosVec.x * -jump_coefficient;
        shot.z = prosVec.z * -jump_coefficient;
      }
      shot.y = 20.0f;
      this->currentCutterpillar->diagonalThrow(shot);
    }
  }


  checkCollisionAndMove(this->currentCutterpillar,newPos,inputActions::getInstance().objectPointers);
 if(inputActions::getInstance().movedX!=0){
    this->currentCutterpillar->rot.y+=(float)(inputActions::getInstance().movedX)/500;
    if(this->currentCutterpillar->rot.y<-2*M_PI) this->currentCutterpillar->rot.y+=2*M_PI;
    if(this->currentCutterpillar->rot.y>2*M_PI) this->currentCutterpillar->rot.y-=2*M_PI;
 }
 if(inputActions::getInstance().movedY!=0){
    this->currentCutterpillar->rot.z+=(float)(inputActions::getInstance().movedY)/500;
    if(this->currentCutterpillar->rot.z<-M_PI/3) this->currentCutterpillar->rot.z=-M_PI/3;
    if(this->currentCutterpillar->rot.z>M_PI/3) this->currentCutterpillar->rot.z=M_PI/3;
 }
 if(inputActions::getInstance().scroll!=0){
    this->currentCutterpillar->viewBack += (float)inputActions::getInstance().scroll/1.5;
    if(this->currentCutterpillar->viewBack > 0.0f) this->currentCutterpillar->viewBack = 0.0f;
    if(this->currentCutterpillar->viewBack < -60.0f) this->currentCutterpillar->viewBack = -60.0f;
}
 if(inputActions::getInstance().rightClick){
    if(this->currentCutterpillar->tmpViewBack > 0) this->currentCutterpillar->tmpViewBack = this->currentCutterpillar->viewBack;
    if(this->currentCutterpillar->viewBack < -20) this->currentCutterpillar->viewBack=0.0f;
 }
   else{
     if(this->currentCutterpillar->tmpViewBack <= 0 ){
       this->currentCutterpillar->viewBack = this->currentCutterpillar->tmpViewBack;
        this->currentCutterpillar->tmpViewBack=666.0f;
     }
   }
}

void Game::bulletShot() {//Warunek na strzal - widok z celowikiem i lewy przycisk
    if(!this->bullets.empty()) return;

    if(!inputActions::getInstance().space_pressed && this->currentCutterpillar->on_the_ground) {

        if (inputActions::getInstance().leftClick && !(this->currentCutterpillar->viewBack < -20)) {

            if (!this->powerischoosed) this->shotPower = this->minShotPower;
            if (this->shotPower >= this->maxShotPower) this->shotPower = this->maxShotPower;
            else {
                shotPower = shotPower + this->increaseShotPower*inputActions::getInstance().deltaTime;
                if(this->shotPower > this->maxShotPower){
                    this->shotPower = this->maxShotPower;
                }
            }
            //Wybieranie sily strzalu:
            this->powerischoosed = true;
            this->calculatedDamage = 0;
            this->shot = glm::vec3(0);
        }
        //Po wyborze sily strzaly nastepuje strzal
        if (!inputActions::getInstance().leftClick && powerischoosed) {

            inputActions::getInstance().SoundEngine->play2D("../src/sounds/rpg_shot.wav", GL_FALSE);

            glm::vec3 shotViewVec = glm::mat3(this->currentCutterpillar->rotM) * this->currentCutterpillar->startLook;
            glm::vec3 look = glm::mat3(this->currentCutterpillar->rotMY) * this->currentCutterpillar->startLook;

            look = glm::normalize(look);
            shotViewVec = glm::normalize(shotViewVec);
            float katZ = acos(glm::dot(shotViewVec,look));

            if(look.y - shotViewVec.y < 0) katZ = -katZ;

            //tworzenie obiektu Bullet
            Bullet *bullecik = new Bullet((char *) "../src/obj/bullet.obj", this->shotPower);
            //ustawienie pozycji poczatkowej
            float katY = this->currentCutterpillar->rot.y;

            bullecik->setPos(currentCutterpillar->pos.x,
                             currentCutterpillar->pos.y + currentCutterpillar->size.y * 6.0f,
                             currentCutterpillar->pos.z);
            //ustawienie szybkosci wystrzelonego pocisku
            float shotY,shotXZ,shotX,shotZ;
            shotY = sin(-katZ)*this->shotPower;
            shotXZ = cos(-katZ)*this->shotPower;
            shotZ = sin(katY)*shotXZ;
            shotX = cos(katY)*shotXZ;

            this->shot=glm::vec3(shotX,shotY,shotZ);
            //dodanie wyzej stworzonego obiektu do listy pociskow
            bullets.push_back(bullecik);
            //dodanie do listy wszystkich obiektow
            inputActions::getInstance().objectPointers.push_back(bullecik);
            bullets.back()->diagonalThrow(shot);

            this->powerischoosed = false;
            this->shotPower = this->minShotPower;
        }
    }
}

bool Game::checkCollisionAndMove(Object *o,glm::vec3 pos, std::vector<Object*> v){
   return checkCollisionAndMove(o,pos.x,pos.y,pos.z,v);
}
bool Game::checkCollisionAndMove(Object *o,float x, float y, float z ,std::vector<Object*> v){
   //Czy ruch moze byc wykonany - zmienne dla 3 wymiarow
   bool canX = true;
   bool canY = false;
   bool canZ = true;

   Caterpillar* cat;
   Caterpillar* hcat;
   Bullet* bul;
   int boomRadius = 20;

   //Kolizja z murem
   if(x<=5 || x>vertX-5) canX = false;
   //Ograniczenie na y
   if(y<=0 || y>maxMapHeight+300) canY = false;
   //Kolizja z murem
   if(z<=5 || z>vertY-5) canZ = false;

   for(unsigned int i=0; i< v.size(); i++)
   {
     //Dla wiez
     if(dynamic_cast<Tower *>(v[i]))
     {
       if((x >= (int)v[i]->pos.x - (v[i]->size.x/2)) && (x <= (int)v[i]->pos.x + (v[i]->size.x/2))
          && (z >= (int)v[i]->pos.z - (v[i]->size.z/2)) && (z <= (int)v[i]->pos.z + (v[i]->size.z/2)))
       {
         canX = false;
         canZ = false;
       }
     }
     //Dla Caterpillar
     else if((cat = dynamic_cast<Caterpillar *>(v[i])))
     {
       if(v[i] != Game::currentCutterpillar && o!=v[i])//wykluczenie kolizji z samym soba
    {
         if((x >= (int)v[i]->pos.x - 5) && (x <= (int)v[i]->pos.x + 5)
            && (y >= (int)v[i]->pos.y - (int)v[i]->size.y*6) && (y <= (int)v[i]->pos.y + (int)v[i]->size.y*12)
            && (z >= (int)v[i]->pos.z - 3) && (z <= (int)v[i]->pos.z + 3))
         {
           if(cat->dead == 0) {
           canX = false;
           canY = false;
           canZ = false;
           }
           else {
             if((hcat = dynamic_cast<Caterpillar *>(o)) && !cat->colission && hcat->dead==0){
               hcat->heal(100);
               cat->colission = true;
             }
           }
           //Jesli kolizja z pociskiem to zmniejszamy zycie Caterpillara
           if((bul = dynamic_cast<Bullet *>(o)))
           {
             if(bul->colission == false){
             Map::getInstance().particleEffectsVector.push_back(new ParticleEffect(glm::vec3(x,y,z),5,2,boomRadius*1.3,NUMBERS_OF_PARTICLES,5,0.7));
             inputActions::getInstance().SoundEngine->play2D("../src/sounds/boom_01.wav", GL_FALSE);
             Map::getInstance().kaboom(x,y,z,boomRadius);
             o->colission = true;
             bul->currentWaitTime = bul->waitTime;

              for(unsigned int j=0; j < v.size(); j++)
               {
                 if((cat = dynamic_cast<Caterpillar *>(v[j])) && cat->dead==0)
                 {
                   float rad = sqrt(pow((cat->pos.x - o->pos.x),2)+ pow((cat->pos.z - o->pos.z),2));
                   cout << "Radius: " << rad << endl;
                   if(rad <= boomRadius)
                   {
                       float proc = 1 - (rad/boomRadius);
                       cat->dealDamage((int)(bul->damage*proc));
                   }
                 }
               }
             }
           }
         }
       }
     }
   }
   //jesli jest na mapie
   if(((int)o->pos.x >= 0) && ((int)o->pos.x < vertX) && ((int)o->pos.z >= 0) && ((int)o->pos.z < vertY))
   {
     if(y-(o->size.y) > Map::getInstance().mapVert[(int)x][(int)z])
         canY=true;
      else{
         y = Map::getInstance().mapVert[(int)x][(int)z]+(o->size.y);
         if((bul = dynamic_cast<Bullet *>(o)))
         {
           cout << "Boooooom" <<endl;
           if(!o->colission)
           {
             Map::getInstance().particleEffectsVector.push_back(new ParticleEffect(glm::vec3(x,y,z),5,2,boomRadius*1.3,NUMBERS_OF_PARTICLES,5,0.7));
              Map::getInstance().kaboom(x,y,z,boomRadius);
              inputActions::getInstance().SoundEngine->play2D("../src/sounds/boom_01.wav", GL_FALSE);
              bul->currentWaitTime = bul->waitTime;
              o->colission = true;
           }
           for(unsigned int i=0; i< v.size(); i++)
           {
             if((cat = dynamic_cast<Caterpillar *>(v[i])))
             {
               //Zmniejszanie zycia jesli w promieniu boom booma kabooma
               float rad = sqrt(pow((cat->pos.x - o->pos.x),2)+pow((cat->pos.z - o->pos.z),2));
               cout << "Radius: " << rad << endl;
               if(rad <= boomRadius){
                   //Zmiana do damage
                   float proc = 1 - (rad/boomRadius);
                   cat->dealDamage((int)(bul->damage*proc));
               }
             }
           }
         }
      }
   }
   //Jesli moze wykonac ruch -> wykonaj go
   if(canX) o->pos.x = x;
   if(canY) o->pos.y = y;
   if(canZ) o->pos.z = z;

   return canY;
}

void Game::testViewMov(){
   glm::vec3 viewVec=this->lookAt-this->lookFrom;
   glm::vec3 prosVec=viewVec;
   prosVec[1]=0;
   float tmp=prosVec[0];
   prosVec[0]=prosVec[2];
   prosVec[2]=tmp;
   prosVec[2]!=0? prosVec[2]=-prosVec[2] : prosVec[0]=-prosVec[0];

   if(inputActions::getInstance().w_pressed){
      glm::vec3 add = glm::normalize(viewVec);
      this->lookFrom+=add*2.0f;
      this->lookAt+=add*2.0f;
   }
   if(inputActions::getInstance().s_pressed){
      glm::vec3 add = glm::normalize(viewVec);
      this->lookFrom-=add*2.0f;
      this->lookAt-=add*2.0f;
   }
   if(inputActions::getInstance().a_pressed){
      glm::vec3 add = glm::normalize(prosVec)*2.0f;
      this->lookFrom+=add;
      this->lookAt+=add;
   }
   if(inputActions::getInstance().d_pressed){
      glm::vec3 add = glm::normalize(prosVec)*2.0f;
      this->lookFrom-=add;
      this->lookAt-=add;
   }
   if(inputActions::getInstance().movedX!=0){
      glm::mat3 rotM = glm::mat3(
         glm::vec3(cos(inputActions::getInstance().movedX/300),0.0f,sin(inputActions::getInstance().movedX/300)),
         glm::vec3(0.0f,1.0f,0.0f),
         glm::vec3(-sin(inputActions::getInstance().movedX/300),0.0f,cos(inputActions::getInstance().movedX/300))
      );
      viewVec=rotM*viewVec;
      this->lookAt=viewVec+this->lookFrom;
   }
   if(inputActions::getInstance().movedY!=0)
      this->lookAt[1]-=inputActions::getInstance().movedY/2;
}

void Game::pressESC() {
    inputActions::getInstance().changeState('p',this->window,this->cursor);
    inputActions::getInstance().cursorFixedCenterPos=false;
}


int Game::procentShotPower() {
    float licznik=this->shotPower-this->minShotPower;
    float mianownik=this->maxShotPower-this->minShotPower;
    if(mianownik<=0.0f){
        mianownik = 1.0f;
    }
    return (licznik/mianownik)*100.0f;
}

void Game::endGame() {
    inputActions::getInstance().changeState('e',this->window,this->cursor);
    inputActions::getInstance().cursorFixedCenterPos=false;
}
