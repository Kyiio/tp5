#include "gamewindowshader.h"
#include "filemanager.h"
#include "objetply3d.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>
#include <QtOpenGL/QGLWidget>

#include <QtCore/qmath.h>
#include <QMouseEvent>
#include <QKeyEvent>
#include <time.h>
#include <sys/time.h>
#include <iostream>

#include <QtCore>
#include <QtGui>

using namespace std;


GameWindowShader::GameWindowShader(int fps, Camera* camera, int day)
{
    m_fps = fps;
    m_isRotating = false;
    m_timer = new QTimer(this);
    m_timer->connect(m_timer, SIGNAL(timeout()),this, SLOT(renderNow()));
    m_timer->start(1000/fps);
    m_camera = camera;

    m_timer_day = new QTimer(this);
    m_timer_day->connect(m_timer_day, SIGNAL(timeout()),this, SLOT(update_day()));
    m_timer_day->start(20);

    m_day = day;

    if(m_day > 270){
        titre = "Automne";
        m_type = AUTOMNE;
    }
    else if (m_day > 180){
        titre = "Ete";
        m_type = ETE;
    }
    else if(m_day > 90){
        titre = "Printemps";
        m_type = PRINTEMPS;
    }
    else if(m_day > 0){
        titre = "Hiver";
        m_type = HIVER;
    }

    m_arbreEte =  ObjetPly3D("..\\TP4\\summertree.ply");
    m_arbreHiver =  ObjetPly3D("..\\TP4\\wintertree.ply");
    m_arbreAutomne =  ObjetPly3D("..\\TP4\\autumntree.ply");
    m_arbrePrintemps =  ObjetPly3D("..\\TP4\\springtree.ply");
    m_tonneau = ObjetPly3D ("..\\TP4\\barril.ply");


}



void GameWindowShader::initialize()
{
    const qreal retinaScale = devicePixelRatio();

    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -100.0, 100.0);

    loadMap(":/heightmap-2.png");

//    if(m_type == HIVER || m_type == AUTOMNE)
//        initall();


    m_programNM = new QOpenGLShaderProgram(this);
    m_programNM->addShaderFromSourceFile(QOpenGLShader::Vertex, "..\\tp5\\Shaders\\NM_vertexShader.vsh");
    m_programNM->addShaderFromSourceFile(QOpenGLShader::Fragment, "..\\tp5\\Shaders\\NM_fragmentShader.fsh");

    if(!m_programNM->link()){
        qDebug() << "Erreur de link !" << endl;
        qDebug() << m_programNM->log() << endl;
        exit(0);
    }

    m_matrixUniform = m_programNM->uniformLocation("matrix");

    loadTexture2("..\\tp5\\Texture\\water.jpg", m_textureEau);
    loadTexture2("..\\tp5\\Texture\\Grass.jpg", m_textureHerbe);
    loadTexture2("..\\tp5\\Texture\\texture_montagne.jpg", m_textureMontagne);
    loadTexture2("..\\tp5\\Texture\\textureSandstone.png", m_textureSable);
    loadTexture2("..\\tp5\\Texture\\deep_blue.jpg", m_textureEauProfonde);
    loadTexture2("..\\tp5\\Texture\\neige.jpg", m_textureNeige);
    loadTexture2("..\\tp5\\Texture\\feuilles.jpg", m_textureFeuilleMorte);
    loadTexture2("..\\tp5\\Texture\\normalMap_heightmap2.png", m_normalMap);

    /** Creation des arbres ** /
    int t = ARBRE_ETE;

    m_objectList << t << 0.0 << 0.0 << 0.0 << 0.03;
    m_objectList << t << 0.39 << -0.1 << 0.0 << 0.02;
    m_objectList << t << 0.19 << -0.3 << 0.0 << 0.02;
    m_objectList << t << 0.15 << -0.3 << 0.0 << 0.02;
    m_objectList << t << -0.35 << -0.09 << 0.0 << 0.02;
    m_objectList << t << -0.28 << -0.08 << 0.0 << 0.02;
    m_objectList << t << -0.29 << 0.01 << 0.0 << 0.02;
    m_objectList << t << -0.1 << 0.2 << 0.0 << 0.03;
    m_objectList << t << -0.1 << 0.09 << 0.0 << 0.03;
    m_objectList << t << 0.39 << -0.2 << 0.0 << 0.02;
    m_objectList << t << 0.30 << -0.29 << 0.0 << 0.02;
    m_objectList << t << 0.1 << -0.4 << 0.0 << 0.02;
    m_objectList << t << 0.0 << -0.42 << 0.0 << 0.03;
    m_objectList << t << 0.2 << -0.42 << 0.0 << 0.03;
    m_objectList << t << -0.1 << -0.1 << 0.0 << 0.03;
    m_objectList << t << -0.05 << -0.15 << 0.0 << 0.03;
    /** **/

    //m_objectList << TONNEAU << 0.0 << 0.0 << 0.2 << 0.03;

    glEnable(GL_DEPTH_TEST);

    glScalef(0.3f,0.3f,0.3f);

    glEnable(GL_LIGHTING);  // Active l'éclairage
    glEnable(GL_LIGHT0);    // Allume la lumière n°1

    glEnable( GL_COLOR_MATERIAL );

    float ambient[] = {0.5 , 0.5 , 0.5 , 1.0};
    float diffuse[] = {1.0 , 1.0 , 1.0 , 1.0};
    float position[] = {3.0 , 14.0 , 20.0 , 1.0};

    glLightfv(GL_LIGHT0 , GL_AMBIENT , ambient);
    glLightfv(GL_LIGHT0 , GL_DIFFUSE , diffuse);
    glLightfv(GL_LIGHT0 , GL_POSITION , position);
}

void GameWindowShader::loadMap(QString localPath)
{

    if (QFile::exists(localPath)) {
        m_image = QImage(localPath);
    }

    uint id = 0;
    p = new point[m_image.width() * m_image.height()];
    QRgb pixel;
    for(int i = 0; i < m_image.width(); i++)
    {
        for(int j = 0; j < m_image.height(); j++)
        {

            pixel = m_image.pixel(i,j);

            id = i*m_image.width() +j;

            p[id].x = (float)i/(m_image.width()) - ((float)m_image.width()/2.0)/m_image.width();
            p[id].y = (float)j/(m_image.height()) - ((float)m_image.height()/2.0)/m_image.height();
            p[id].z = 0.001f * (float)(qRed(pixel));
        }
    }
}

void GameWindowShader::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(m_isRotating)
        m_camera->set_rotY(m_camera->get_rotY() + 1.0f);

    glLoadIdentity();
    m_camera->scale();
    glRotatef(m_camera->get_rotX(),1.0f,0.0f,0.0f);
    glRotatef(m_camera->get_rotY(),0.0f,0.0f,1.0f);
    /*
    if(m_type == HIVER || m_type == AUTOMNE){
        renderMeteo();
        update();
    }
    */

    float diffuse1[] = {1.0 , 1.0 , 1.0 , 1.0};
    float diffuse2[] = {0.5 , 0.5 , 0.5 , 1.0};

    switch(m_camera->get_etat())
    {
    case 0:
        this->setTitle(this->titre + ": points + texture");
        glLightfv(GL_LIGHT0 , GL_DIFFUSE , diffuse1);
        displayPoints();
        renderObjectsPoints();
        break;
    case 1:
        this->setTitle(this->titre + ": lines + couleur");
        glLightfv(GL_LIGHT0 , GL_DIFFUSE , diffuse2);
        displayLines();
        renderObjectsPoints();
        break;
    case 2:
        this->setTitle(this->titre + ": triangles + texture");
        glLightfv(GL_LIGHT0 , GL_DIFFUSE , diffuse1);
        displayTrianglesTexture();
        renderObjectsFace();
        break;
    case 3:
        this->setTitle(this->titre + ": triangles + couleur");
        glLightfv(GL_LIGHT0 , GL_DIFFUSE , diffuse2);
        displayTriangles();
        renderObjectsFace();
        break;
    default:
        this->setTitle(this->titre + ": point + texture");
        glLightfv(GL_LIGHT0 , GL_DIFFUSE , diffuse1);
        displayPoints();
        renderObjectsPoints();
        break;
    }

    ++m_frame;

}

void GameWindowShader::renderObjectsFace(){

    int type;

    for(int i = 0; i < m_objectList.size() / 5; ++i){

        type = m_objectList.at(i*5);

        if(type == ARBRE_AUTOMNE){
            glColor3f(165.0f/255.0f, 38.0f/255.0f, 10.0f/255.0f);
            m_arbreAutomne.renderFace(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                      m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
        else if(type == ARBRE_ETE){
            glColor3f(237.0f/255.0f, 127.0f/255.0f, 16.0f/255.0f);
            m_arbreEte.renderFace(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                  m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
        else if(type == ARBRE_HIVER){
            glColor3f(9.0f/255.0f, 82.0f/255.0f, 40.0f/255.0f);
            m_arbreHiver.renderFace(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                    m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
        else if(type == ARBRE_PRINTEMPS){
            glColor3f(255.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f);
            m_arbrePrintemps.renderFace(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                        m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
        else{ //if(type == TONNEAU)
            glColor3f(165.0f/255.0f, 38.0f/255.0f, 10.0f/255.0f);
            m_tonneau.renderFace(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                 m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
    }
}

void GameWindowShader::renderObjectsPoints(){

    int type;

    for(int i = 0; i < m_objectList.size() / 5; ++i){

        type = m_objectList.at(i*5);

        if(type == ARBRE_AUTOMNE){
            glColor3f(165.0f/255.0f, 38.0f/255.0f, 10.0f/255.0f);
            m_arbreAutomne.renderPoints(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                        m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
        else if(type == ARBRE_ETE){
            glColor3f(237.0f/255.0f, 127.0f/255.0f, 16.0f/255.0f);
            m_arbreEte.renderPoints(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                    m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
        else if(type == ARBRE_HIVER){
            glColor3f(9.0f/255.0f, 82.0f/255.0f, 40.0f/255.0f);
            m_arbreHiver.renderPoints(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                      m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
        else if(type == ARBRE_PRINTEMPS){
            glColor3f(255.0f/255.0f, 0.0f/255.0f, 0.0f/255.0f);
            m_arbrePrintemps.renderPoints(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                          m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
        else{ //if(type == TONNEAU)
            glColor3f(165.0f/255.0f, 38.0f/255.0f, 10.0f/255.0f);
            m_tonneau.renderPoints(m_objectList.at(i*5+1),m_objectList.at(i*5+2),
                                   m_objectList.at(i*5+3),m_objectList.at(i*5+4));
        }
    }
}

void GameWindowShader::update_day(){
    m_day = (m_day + 1) % 365;

    if(m_day > 270){
        titre = "Automne";
        m_type = AUTOMNE;
    }
    else if (m_day > 180){
        titre = "Ete";
        m_type = ETE;
    }
    else if(m_day > 90){
        titre = "Printemps";
        m_type = PRINTEMPS;
    }
    else if(m_day > 0){
        titre = "Hiver";
        m_type = HIVER;
    }
}

bool GameWindowShader::event(QEvent *event)
{
    switch (event->type())
    {
    case QEvent::UpdateRequest:

        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void GameWindowShader::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {

    case 'L':
    {
        qDebug() << "Loading";
        FileManager filemanager;
        filemanager.loadFile(this,this->title());

        //reload la bonne map
        QString depth (":/heightmap-");
        depth += QString::number(carte) ;
        depth += ".png" ;

        loadMap(depth);

        qDebug() << "Fin load";
        break;
    }
    case 'G':
    {
        qDebug() << "Sauvegarde";
        FileManager filemanager(this,this->title());
        qDebug() << "Fin sauvegarde";
        break;
    }
    case 'C':
    {
        m_isRotating = !m_isRotating;
        break;
    }
    case 'P':
    {
        if(m_fps < 1000){
            m_timer->stop();
            m_fps *= 2;
            m_timer->start(1000/m_fps);
        }
        break;
    }
    case 'M':
    {
        if(m_fps >= 2){
            m_timer->stop();
            m_fps /= 2;
            m_timer->start(1000/m_fps);
        }
        break;
    }
    case 'Z':
        m_camera->set_ss(m_camera->get_ss()+0.10f);
        break;
    case 'S':
        m_camera->set_ss(m_camera->get_ss()-0.10f);
        break;
    case 'A':
        m_camera->set_rotX(m_camera->get_rotX()+1.0f);
        break;
    case 'E':
        m_camera->set_rotX(m_camera->get_rotX()-1.0f);
        break;
    case 'Q':
        m_camera->set_rotY(m_camera->get_rotY()+1.0f);
        break;
    case 'D':
        m_camera->set_rotY(m_camera->get_rotY()-1.0f);
        break;
    case 'W':
        m_camera->set_etat(m_camera->get_etat()+1);
        if(m_camera->get_etat() > 3)
            m_camera->set_etat(0);
        break;
    case 'X':
        carte ++;
        if(carte > 3)
            carte = 1;
        QString depth (":/heightmap-");
        depth += QString::number(carte) ;
        depth += ".png" ;

        loadMap(depth);
        break;

    }
    //renderNow();
}


void GameWindowShader::initall()
{

    srand(time(NULL));
    for(int i = 0; i < MAX_PARTICLES; i++)
    {
        Particles[i].x = (-0.5f) + ((float)(rand())/ (float)(RAND_MAX));
        Particles[i].y = (-0.5f) + ((float)(rand())/ (float)(RAND_MAX));
        Particles[i].z = ((float)(rand())/ (float)(RAND_MAX));
    }
}

void GameWindowShader::initentity(int index)
{
    Particles[index].x = (-0.5f) + ((float)(rand())/ (float)(RAND_MAX));
    Particles[index].y = (-0.5f) + ((float)(rand())/ (float)(RAND_MAX));
    Particles[index].z = ((float)(rand())/ (float)(RAND_MAX));
}

void GameWindowShader::renderMeteo()
{
    glBegin(GL_POINTS);

    if(m_type == HIVER)
        glColor3f(1.0f,1.0f,1.0f);
    else//automne
        glColor3f(44.0f/255.0f,117.0f/255.0f,1.0f);

    for(int i = 0; i < MAX_PARTICLES; i++)
    {
        glVertex3f(Particles[i].x, Particles[i].y, Particles[i].z);
    }
    glEnd();
}

void GameWindowShader::update()
{
    for(int i = 0; i < MAX_PARTICLES; i++)
    {

       if(m_type == AUTOMNE)
            Particles[i].z -= ((float)(rand())/(float)(RAND_MAX)) * 0.15f;
       else{//hiver
            Particles[i].z -= ((float)(rand())/(float)(RAND_MAX))* 0.025f;
            //Particles[i].x += ((float)(rand())/(float)(RAND_MAX));
            //Particles[i].y += ((float)(rand())/(float)(RAND_MAX));
        }

        if(Particles[i].z < 0)
        {
            initentity(i);
        }
    }
}



void GameWindowShader::displayPoints()
{
    /*glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POINTS);
    uint id = 0;

    for(int i = 0; i < m_image.width(); i++)
    {
        for(int j = 0; j < m_image.height(); j++)
        {
            displayColorSeasons(p[id].z);

            id = i*m_image.width() +j;
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

        }
    }
    glEnd();*/

    m_programNM->bind();

    QMatrix4x4 matrix = QMatrix4x4();
    matrix.scale(m_camera->get_ss());
    matrix.rotate(m_camera->get_rotX(), 1, 0, 0);
    matrix.rotate(m_camera->get_rotY(), 0, 0, 1);

    m_programNM->setUniformValue(m_matrixUniform, matrix);

    glUniform1i(m_programNM->uniformLocation("texture1"), 0);
    glUniform1i(m_programNM->uniformLocation("texture2"), 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_normalMap);

    uint id = 0;
    for(int i = 0; i < m_image.width(); i++)
    {
        for(int j = 0; j < m_image.height(); j++)
        {
            //displayColorSeasons(p[id].z);
            id = i*m_image.width() +j;
            bindTextureRegardingZ(p[id].z);

            glBegin(GL_POINTS);
            glTexCoord2f((float)(i)/(float)(m_image.width()),(float)(j)/(float)(m_image.height()));
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            glEnd();
        }
    }

    m_programNM->release();

}

void GameWindowShader::bindTextureRegardingZ(float z){

    glActiveTexture(GL_TEXTURE0);

    if(m_type == ETE){
        if (z > 0.08f){//marron
            glBindTexture(GL_TEXTURE_2D, m_textureMontagne);
        }
        else if (z > 0.04f){//vert
            glBindTexture(GL_TEXTURE_2D, m_textureHerbe);
        }
        else if (z > 0.020f){//jaune
            glBindTexture(GL_TEXTURE_2D, m_textureSable);
        }
        else {//bleu clair
            glBindTexture(GL_TEXTURE_2D, m_textureEau);
        }
    }
    else if (m_type == AUTOMNE){

        if (z > 0.08f){//marron
            glBindTexture(GL_TEXTURE_2D, m_textureMontagne);
        }
        else if (z > 0.04f){//vert
            glBindTexture(GL_TEXTURE_2D, m_textureHerbe);
        }
        else if (z > 0.030f){//orange
            glBindTexture(GL_TEXTURE_2D, m_textureFeuilleMorte);
        }
        else if (z > 0.025f){//bleu clair
            glBindTexture(GL_TEXTURE_2D, m_textureEau);
        }
        else{//bleu foncé
            glBindTexture(GL_TEXTURE_2D, m_textureEauProfonde);
        }
    }
    else if(m_type == HIVER){

        if(z > 0.15f){//blanc
           glBindTexture(GL_TEXTURE_2D, m_textureNeige);
        }
        else if (z > 0.08f){//marron
            glBindTexture(GL_TEXTURE_2D, m_textureMontagne);
        }
        else if (z > 0.04f){//blanc
            glBindTexture(GL_TEXTURE_2D, m_textureNeige);
        }
        else if (z > 0.015f){//bleu clair
            glBindTexture(GL_TEXTURE_2D, m_textureEau);
        }
        else{//bleu foncé
            glBindTexture(GL_TEXTURE_2D, m_textureEauProfonde);
        }
    }
    else{//PRINTEMPS

        if(z > 0.2f){//blanc
           glBindTexture(GL_TEXTURE_2D, m_textureNeige);
        }
        else if (z > 0.08f){//marron
           glBindTexture(GL_TEXTURE_2D, m_textureMontagne);
        }
        else if (z > 0.03f){//vert
            glBindTexture(GL_TEXTURE_2D, m_textureHerbe);
        }
        else if (z > 0.015f){//bleu clair
            glBindTexture(GL_TEXTURE_2D, m_textureEau);
        }
        else{//bleu foncé
            glBindTexture(GL_TEXTURE_2D, m_textureEauProfonde);
        }
    }
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, m_normalMap);
}


void GameWindowShader::displayTriangles()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {

            id = i*m_image.width() +j;
            //displayColor(p[id].z);
            displayColorSeasons(p[id].z);

            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +(j+1);
            //displayColor(p[id].z);
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j;
            //displayColor(p[id].z);
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);



            id = i*m_image.width() +(j+1);
            //displayColor(p[id].z);
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j+1;
            //displayColor(p[id].z);
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j;
            //displayColor(p[id].z);
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
        }
    }
    glEnd();
}

void GameWindowShader::displayLines()
{
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    uint id = 0;

    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {


            id = i*m_image.width() +j;
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +(j+1);
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = (i+1)*m_image.width() +j;
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +j;
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = (i+1)*m_image.width() +j;
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = i*m_image.width() +(j+1);
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = i*m_image.width() +(j+1);
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            id = (i+1)*m_image.width() +j+1;
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = (i+1)*m_image.width() +j+1;
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);

            id = (i+1)*m_image.width() +(j);
            displayColorSeasons(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
        }
    }

    glEnd();
}

void GameWindowShader::displayTrianglesTexture()
{

    uint id = 0;
    m_programNM->bind();

    QMatrix4x4 matrix = QMatrix4x4();
    //matrix.perspective(45.0f, 4.0f/3.0f, 0.1f, 100.0f);
    //matrix.translate(0, 0, -m_camera->get_ss());
    matrix.scale(m_camera->get_ss());
    matrix.rotate(m_camera->get_rotX(), 1, 0, 0);
    matrix.rotate(m_camera->get_rotY(), 0, 0, 1);

    m_programNM->setUniformValue(m_matrixUniform, matrix);

    glUniform1i(m_programNM->uniformLocation("texture1"), 0);
    glUniform1i(m_programNM->uniformLocation("texture2"), 1);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_normalMap);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureHerbe);


    for(int i = 0; i < m_image.width()-1; i++)
    {
        for(int j = 0; j < m_image.height()-1; j++)
        {

            id = i*m_image.width() +j;
            bindTextureRegardingZ(p[id].z);
            glBegin(GL_TRIANGLES);
            glTexCoord2f((float)(i)/(float)(m_image.width()),(float)(j)/(float)(m_image.height()));
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);


            id = i*m_image.width() +(j+1);
            //bindTextureRegardingZ(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            glTexCoord2f((float)(i)/(float)(m_image.width()),(float)(j+1)/(float)(m_image.height()));

            id = (i+1)*m_image.width() +j;
            //bindTextureRegardingZ(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            glTexCoord2f((float)(i+1)/(float)(m_image.width()),(float)(j)/(float)(m_image.height()));
            //glEnd();

            id = i*m_image.width() +(j+1);
            //bindTextureRegardingZ(p[id].z);

            //glBegin(GL_TRIANGLES);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            glTexCoord2f((float)(i)/(float)(m_image.width()),(float)(j+1)/(float)(m_image.height()));

            id = (i+1)*m_image.width() +j+1;
            //bindTextureRegardingZ(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            glTexCoord2f((float)(i+1)/(float)(m_image.width()),(float)(j+1)/(float)(m_image.height()));

            id = (i+1)*m_image.width() +j;
            //bindTextureRegardingZ(p[id].z);
            glVertex3f(
                        p[id].x,
                        p[id].y,
                        p[id].z);
            glTexCoord2f((float)(i+1)/(float)(m_image.width()),(float)(j)/(float)(m_image.height()));
            glEnd();
        }
    }


    m_programNM->release();
}


void GameWindowShader::displayColor(float alt)
{

    float R;
    float G;
    float B;

    if(alt > 0.15f){//blanc
       R = 1.0f;
       G = 1.0f;
       B = 1.0f;
    }
    else if (alt > 0.08f){//marron
        R = 88.0f/255.0f;
        G = 41.0f/255.0f;
        B = 0.0f;
    }
    else if (alt > 0.04f){//vert
        R = 20.0f/255.0f;
        G = 148.0f/255.0f;
        B = 20.0f/255.0f;
    }
    else if (alt > 0.030f){//jaune
        R = 255.0f/255.0f;
        G = 222.0f/255.0f;
        B = 117.0f/255.0f;
    }
    else if (alt > 0.015f){//bleu clair
        R = 44.0f/255.0f;
        G = 117.0f/255.0f;
        B = 1.0f;
    }
    else{//bleu foncé
        R = 15.0f/255.0f;
        G = 5.0f/255.0f;
        B = 107.0f/255.0f;
    }

    glColor3f(R, G, B);
 /*
    if (alt > 0.2)
    {
        glColor3f(01.0f, 1.0f, 1.0f);
    }
    else     if (alt > 0.1)
    {
        glColor3f(alt, 1.0f, 1.0f);
    }
    else     if (alt > 0.05f)
    {
        glColor3f(01.0f, alt, alt);
    }
    else
    {
        glColor3f(0.0f, 0.0f, 1.0f);
    }
*/
}

void GameWindowShader::displayColorSeasons(float alt)
{
    float R;
    float G;
    float B;

    if(m_type == ETE){

        if (alt > 0.08f){//marron
            R = 143.0f/255.0f;
            G = 89.0f/255.0f;
            B = 34.0f/255.0f;
        }
        else if (alt > 0.04f){//vert
            R = 20.0f/255.0f;
            G = 148.0f/255.0f;
            B = 20.0f/255.0f;
        }
        else if (alt > 0.020f){//jaune
            R = 255.0f/255.0f;
            G = 222.0f/255.0f;
            B = 117.0f/255.0f;
        }
        else {//bleu clair
            R = 44.0f/255.0f;
            G = 117.0f/255.0f;
            B = 1.0f;
        }
    }
    else if(m_type == AUTOMNE){

        if (alt > 0.08f){//marron
            R = 88.0f/255.0f;
            G = 41.0f/255.0f;
            B = 0.0f;
        }
        else if (alt > 0.04f){//vert
            R = 20.0f/255.0f;
            G = 148.0f/255.0f;
            B = 20.0f/255.0f;
        }
        else if (alt > 0.030f){//orange
            R = 223.0f/255.0f;
            G = 120.0f/255.0f;
            B = 20.0f/255.0f;
        }
        else if (alt > 0.025f){//bleu clair
            R = 44.0f/255.0f;
            G = 117.0f/255.0f;
            B = 1.0f;
        }
        else{//bleu foncé
            R = 15.0f/255.0f;
            G = 5.0f/255.0f;
            B = 107.0f/255.0f;
        }
    }
    else if(m_type == HIVER){

        if(alt > 0.15f){//blanc
           R = 1.0f;
           G = 1.0f;
           B = 1.0f;
        }
        else if (alt > 0.08f){//marron
            R = 88.0f/255.0f;
            G = 41.0f/255.0f;
            B = 0.0f;
        }
        else if (alt > 0.04f){//blanc
            R = 1.0f;
            G = 1.0f;
            B = 1.0f;
        }
        else if (alt > 0.015f){//bleu clair
            R = 44.0f/255.0f;
            G = 117.0f/255.0f;
            B = 1.0f;
        }
        else{//bleu foncé
            R = 15.0f/255.0f;
            G = 5.0f/255.0f;
            B = 107.0f/255.0f;
        }
    }
    else{//PRINTEMPS

        if(alt > 0.2f){//blanc
           R = 1.0f;
           G = 1.0f;
           B = 1.0f;
        }
        else if (alt > 0.08f){//marron
            R = 88.0f/255.0f;
            G = 41.0f/255.0f;
            B = 0.0f;
        }
        else if (alt > 0.03f){//vert
            R = 20.0f/255.0f;
            G = 148.0f/255.0f;
            B = 20.0f/255.0f;
        }
        else if (alt > 0.015f){//bleu clair
            R = 44.0f/255.0f;
            G = 117.0f/255.0f;
            B = 1.0f;
        }
        else{//bleu foncé
            R = 15.0f/255.0f;
            G = 5.0f/255.0f;
            B = 107.0f/255.0f;
        }
    }

    glColor3f(R, G, B);

}

void GameWindowShader::loadTexture2(char *filename, GLuint &textureID){
    glEnable(GL_TEXTURE_2D); // Enable texturing

    glGenTextures(1, &textureID); // Obtain an id for the texture
    glBindTexture(GL_TEXTURE_2D, textureID); // Set as the current texture

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    QImage im(filename);

    QImage tex = QGLWidget::convertToGLFormat(im);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glDisable(GL_TEXTURE_2D);

}


int GameWindowShader::day() const
{
    return m_day;
}

void GameWindowShader::setDay(int day)
{
    m_day = day;
}

int GameWindowShader::type() const
{
    return m_type;
}

void GameWindowShader::setType(int type)
{
    m_type = type;
}

Camera *GameWindowShader::camera() const
{
    return m_camera;
}

void GameWindowShader::setCamera(Camera *camera)
{
    m_camera = camera;
}

int GameWindowShader::getCarte() const
{
    return carte;
}

void GameWindowShader::setCarte(int value)
{
    carte = value;
}

bool GameWindowShader::getIsRotating() const
{
    return m_isRotating;
}

void GameWindowShader::setIsRotating(bool isRotating)
{
    m_isRotating = isRotating;
}

QList<float> GameWindowShader::getObjectList() const
{
    return m_objectList;
}

void GameWindowShader::setObjectList(const QList<float> &objectList)
{
    m_objectList = objectList;
}
