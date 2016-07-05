#include "object2D.hpp"
#include "../inputActions.hpp"


object2D::object2D(float x, float y, float width, float height, GLchar *fileName) {
//   this->shader = new Shader("../src/shaders/2dTex.vs","../src/shaders/2dTex.frag");
//   this->vertices.resize(4);
//   this->indices.resize(4);
   this->pos.x = x;
   this->pos.y = y;
   this->pos.z = 0.0f;
    this->size.x = width;
    this->size.y = height;
    this->vertices = {
      x, y, 0.0f, 0.0f, 0.0f,
      x+width, y,  0.0f, 1.0f, 0.0f,
      x, y+height, 0.0f, 0.0f, 1.0f,
      x+width, y+height, 0.0f, 1.0f, 1.0f,
   };
   this->indices = {
      0,1,2,3
   };
   this->bindBuffers(5,GL_STATIC_DRAW);
    this->bindTexture2D(fileName);
   this->posM = glm::mat4(1);
   this->rotM = glm::mat4(1);
   this->sclM = glm::mat4(1);
//   GLint viewport[4];
//   glGetIntegerv(GL_VIEWPORT, viewport);
//    std::cout << "width: " << viewport[2] << " height: " << viewport[3] << std::endl;
//    this->sclM[0][0]=1.0f/viewport[2];
//    this->sclM[0][0]=1.0f;
//   this->sclM[1][1]=1.0f/viewport[3];
//   this->sclM[1][1]=1.0f;
}
void object2D::setTraM(float x,float y,float z){
   this->posM[3][0]=x;
   this->posM[3][1]=y;
   this->posM[3][2]=z;
}
object2D::~object2D(){

}

//void object2D::bindBuffers(){
//   this->initBinding(true);
//
//   glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*this->vertices.size(), &this->vertices.front(), GL_STATIC_DRAW);
//   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*this->indices.size(), &this->indices.front(), GL_STATIC_DRAW);
//
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
//	glEnableVertexAttribArray(0);
//   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
//   glEnableVertexAttribArray(1);
//
//   this->endBinding();
//}

void object2D::draw(){
    errorCheck("draw");

    this->shader->useShaderProgram(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture2D);
    glUniform1i(glGetUniformLocation(this->shader->shaderProgram[0], "ourTexture1"), 0);

    //Macierz Projekcji
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glm::mat4 projection = glm::ortho((float) -viewport[2]/2.0f,(float) viewport[2]/2.0f, (float) -viewport[3]/2.0f,  (float) viewport[3]/2.0f,-1.0f,1.0f);
    glUniformMatrix4fv(this->getUniform("P"),1,GL_FALSE,glm::value_ptr(projection));
    GLint M = glGetUniformLocation(this->shader->shaderProgram[0], "M");
    glUniformMatrix4fv(M, 1, GL_FALSE, glm::value_ptr(this->posM*this->sclM*this->rotM));

    this->inUniform();

    glBindVertexArray(this->currentVAO());
    glDrawElements(GL_TRIANGLE_STRIP, 5, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    //Rysowanie napisów, jeżeli są jakieś dodane do tablicy
    if(!this->teksty.empty() && this->font != NULL){
        glm::mat4 temp = this->posM;
        temp[3][1] = -temp[3][1];
        this->font->posM = temp;

        for(unsigned int i=0;i < this->teksty.size();i++){
            this->teksty[i]->draw(this->font);

        }
    }

}

void object2D::initFont(const char *ttf, int size) {
    this->font = new Font(ttf,size);
}

unsigned int object2D::addText(std::string newText, float newX, float newY, float newSkala, glm::vec3 kolor) {
    Text *temp = new Text(newText,newX,newY,newSkala,kolor);
    this->teksty.push_back(temp);
    return this->teksty.size() - 1;
}

unsigned int object2D::addTextM(std::string newText, float newX, float newY, float newSkala, glm::vec3 kolor) {
    return this->addText(newText,this->pos.x + this->size.x/2+(-this->font->length(newText,newSkala)/2+newX),this->pos.y+this->size.y/2+(-this->font->height(newSkala)/2+newY),newSkala,kolor);
//    return this->addText(newText,(-this->font->length(newText,newSkala)/2+newX),(-this->font->height(newSkala)/2+newY),newSkala,kolor);
}


unsigned int object2D::addTextL(std::string newText, float newX, float newY, float newSkala, glm::vec3 kolor) {
    return this->addText(newText,this->pos.x + this->size.x/2+(-this->font->length(newText,newSkala)/2+newX),this->pos.y+this->size.y/2+(-this->font->height(newSkala)/2+newY),newSkala,kolor);
}

