/*
glEnable(GL_DEPTH_TEST)の役割
これがあることにより図形に空間的な前後関係が生じる。
つまり，これが存在しないとその立体図形を二次元平面上に
投影した領域のうち最後に描画したものが空間的な前後関係に
関係なく表示されるようになる。今回の正四角錐では、最も
最後に描かれる底面の正方形が最も優先的に表示されるので、
どのような方向から見ても正方形が見える。
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#define WINDOW_X (500)
#define WINDOW_Y (500)
#define WINDOW_NAME "test2"

#define FLAG 0

void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);

void draw_pyramid();

void draw_cube();
void draw_gamecube();

// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 10.0;
bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;

int g_display_mode = 1;

int main(int argc, char *argv[]){
    /* OpenGL の初期化 */
    init_GL(argc,argv);

    /* このプログラム特有の初期化 */
    init();

    /* コールバック関数の登録 */
    set_callback_functions();

    /* メインループ */
    glutMainLoop();

    return 0;
}

void init_GL(int argc, char *argv[]){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_X,WINDOW_Y);
    glutCreateWindow(WINDOW_NAME);
}

void init(){
    glClearColor(0.0, 0.0, 0.0, 0.0); // 背景の塗りつぶし色を指定
}

void set_callback_functions(){
    glutDisplayFunc(glut_display);
    glutKeyboardFunc(glut_keyboard);
    glutMouseFunc(glut_mouse);
    glutMotionFunc(glut_motion);
    glutPassiveMotionFunc(glut_motion);
}

void glut_keyboard(unsigned char key, int x, int y){
    switch(key){
    case 'q':
    case 'Q':
    case '\033':
        exit(0);

    case '1':
    g_display_mode = 1;
    break;
    case '2':
    g_display_mode = 2;
    break;
    case '3':
    g_display_mode = 3;
    break;
    }

    glutPostRedisplay();
}

void glut_mouse(int button, int state, int x, int y){
    if(button == GLUT_LEFT_BUTTON){
        if(state == GLUT_UP){
            g_isLeftButtonOn = false;
        }else if(state == GLUT_DOWN){
            g_isLeftButtonOn = true;
        }
    }

    if(button == GLUT_RIGHT_BUTTON){
        if(state == GLUT_UP){
            g_isRightButtonOn = false;
        }else if(state == GLUT_DOWN){
            g_isRightButtonOn = true;
        }
    }
}

void glut_motion(int x, int y){
    static int px = -1, py = -1;
    if(g_isLeftButtonOn == true){
        if(px >= 0 && py >= 0){
            g_angle1 += (double)-(x - px)/20;
            g_angle2 += (double)(y - py)/20;
        }
        px = x;
        py = y;
    }else if(g_isRightButtonOn == true){
        if(px >= 0 && py >= 0){
            g_distance += (double)(y - py)/20;
        }
        px = x;
        py = y;
    }else{
        px = -1;
        py = -1;
    }
    glutPostRedisplay();
} 

void glut_display(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, 1.0, 0.1, 100);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    #if FLAG
    if (cos(g_angle2)>0){
        gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
            g_distance * sin(g_angle2),
            g_distance * cos(g_angle2) * cos(g_angle1),
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0);}
    else {
        gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
            g_distance * sin(g_angle2),
            g_distance * cos(g_angle2) * cos(g_angle1),
            0.0, 0.0, 0.0, 0.0, -1.0, 0.0);}
    #else
        gluLookAt(g_distance,
            g_distance,
            g_distance,
            0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        glRotated(g_angle1*20, 0.0, -10.0, 0.0);
        glRotated(g_angle2*20, 10.0, 0.0, 0.0);
    #endif

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    switch(g_display_mode){
        case 1:
        draw_pyramid();
        break;
        case 2:
        draw_cube();
        break;
        case 3:
        draw_gamecube();
        break;
    }

    glDisable(GL_DEPTH_TEST);

    glutSwapBuffers();
}

void draw_pyramid(){
    GLdouble pointO[] = {0.0, 1.0, 0.0};
    GLdouble pointA[] = {1.5, -1.0, 1.5};
    GLdouble pointB[] = {-1.5, -1.0, 1.5};
    GLdouble pointC[] = {-1.5, -1.0, -1.5};
    GLdouble pointD[] = {1.5, -1.0, -1.5};

    glColor3d(1.0, 0.0, 0.0);
    glBegin(GL_TRIANGLES);
    glVertex3dv(pointO);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glEnd();

    glColor3d(1.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES);
    glVertex3dv(pointO);
    glVertex3dv(pointB);
    glVertex3dv(pointC);
    glEnd();

    glColor3d(0.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES);
    glVertex3dv(pointO);
    glVertex3dv(pointC);
    glVertex3dv(pointD);
    glEnd();

    glColor3d(1.0, 0.0, 1.0);
    glBegin(GL_TRIANGLES);
    glVertex3dv(pointO);
    glVertex3dv(pointD);
    glVertex3dv(pointA);
    glEnd();

    glColor3d(1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointC);
    glVertex3dv(pointD);
    glEnd();
}

void draw_cube(){
    GLdouble pointA[] = {1.0, 1.0, 1.0};
    GLdouble pointB[] = {-1.0, 1.0, 1.0};
    GLdouble pointC[] = {-1.0, -1.0, 1.0};
    GLdouble pointD[] = {1.0, -1.0, 1.0};
    GLdouble pointE[] = {1.0, 1.0, -1.0};
    GLdouble pointF[] = {-1.0, 1.0, -1.0};
    GLdouble pointG[] = {-1.0, -1.0, -1.0};
    GLdouble pointH[] = {1.0, -1.0, -1.0};

    glColor3d(0.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointC);
    glVertex3dv(pointD);
    glEnd();

    glColor3d(1.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointF);
    glVertex3dv(pointE);
    glEnd();

    glColor3d(1.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointB);
    glVertex3dv(pointC);
    glVertex3dv(pointG);
    glVertex3dv(pointF);
    glEnd();

    glColor3d(0.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointE);
    glEnd();

    glColor3d(1.0, 1.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointC);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointG);
    glEnd();

    glColor3d(0.0, 1.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointE);
    glVertex3dv(pointF);
    glVertex3dv(pointG);
    glVertex3dv(pointH);
    glEnd();
}

void draw_gamecube(){
    GLdouble yellow[] = {1.0, 1.0, 0.0};
    GLdouble red[] = {1.0, 0.0, 0.0};
    GLdouble green[] = {0.0, 1.0, 0.0};
    GLdouble blue[] = {0.0, 0.0, 1.0};

    for(double i=-1.5; i<2; i++){
        glColor3dv(green);
        glBegin(GL_POLYGON);
        glVertex3d(1.5, 1.5, i);
        glVertex3d(1.5, -1.5, i);
        glVertex3d(0.5, -1.5, i);
        glVertex3d(0.5, 1.5, i);
        glEnd();
        glColor3dv(green);
        glBegin(GL_POLYGON);
        glVertex3d(0.5*i/abs(i), -1.5, i);
        glVertex3d(-0.5*i/abs(i), 0.0, i);
        glVertex3d(-0.5*i/abs(i), 1.5, i);
        glVertex3d(0.5*i/abs(i), 0.0, i);
        glEnd();
        glColor3dv(green);
        glBegin(GL_POLYGON);
        glVertex3d(-0.5, -1.5, i);
        glVertex3d(-1.5, -1.5, i);
        glVertex3d(-1.5, 1.5, i);
        glVertex3d(-0.5, 1.5, i);
        glEnd();
    }

    for(double i=-1.5; i< 2; i++){
        glColor3dv(blue);
        glBegin(GL_POLYGON);
        glVertex3d(i, 1.5, 1.5);
        glVertex3d(i, 1.5, 0.5);
        glVertex3d(i, -1.5, 0.5);
        glVertex3d(i, -1.5, 1.5);
        glEnd();
        glColor3dv(blue);
        glBegin(GL_POLYGON);
        glVertex3d(i, 1.5, 0.5*i/abs(i));
        glVertex3d(i, 0.0, 0.5*i/abs(i));
        glVertex3d(i, -1.5, -0.5*i/abs(i));
        glVertex3d(i, 0.0, -0.5*i/abs(i));
        glEnd();   
        glColor3dv(blue);
        glBegin(GL_POLYGON);
        glVertex3d(i, 1.5, -0.5);
        glVertex3d(i, 1.5, -1.5);
        glVertex3d(i, -1.5, -1.5);
        glVertex3d(i, -1.5, -0.5);
        glEnd();
    }

    for(double i=1.5; i>=-0.5; i-=2.0){
        for(double j=1.5; j>=-1.5; j-=3.0){
            for(double k=1.5; k>=-0.5; k-=2.0){
                glColor3dv(yellow);
                glBegin(GL_POLYGON);
                glVertex3d(i, j, k);
                glVertex3d(i-1.0, j, k);
                glVertex3d(i-1.0, j, k-1.0);
                glVertex3d(i, j, k-1.0);
                glEnd();
            }
        }
    }
    //
    glColor3dv(red);
    glBegin(GL_POLYGON);
    glVertex3d(-0.5, 1.5, 1.5);
    glVertex3d(-0.5, 1.5, 0.5);
    glVertex3d(0.5, 0.0, 0.5);
    glVertex3d(0.5, 0.0, 1.5);
    glEnd();

    glColor3dv(blue);
    glBegin(GL_POLYGON);
    glVertex3d(-0.5, 0.0, 1.5);
    glVertex3d(-0.5, 0.0, 0.5);
    glVertex3d(0.5, -1.5, 0.5);
    glVertex3d(0.5, -1.5, 1.5);
    glEnd();
    //
    glColor3dv(green);
    glBegin(GL_POLYGON);
    glVertex3d(1.5, 1.5, 0.5);
    glVertex3d(0.5, 1.5, 0.5);
    glVertex3d(0.5, 0.0, -0.5);
    glVertex3d(1.5, 0.0, -0.5);
    glEnd();

    glColor3dv(red);
    glBegin(GL_POLYGON);
    glVertex3d(1.5, 0.0 , 0.5);
    glVertex3d(0.5, 0.0 , 0.5);
    glVertex3d(0.5, -1.5, -0.5);
    glVertex3d(1.5, -1.5, -0.5);
    glEnd();
    //
    glColor3dv(red);
    glBegin(GL_POLYGON);
    glVertex3d(0.5, 1.5 , -0.5);
    glVertex3d(0.5, 1.5 , -1.5);
    glVertex3d(-0.5, 0.0, -1.5);
    glVertex3d(-0.5, 0.0, -0.5);
    glEnd();

    glColor3dv(blue);
    glBegin(GL_POLYGON);
    glVertex3d(0.5, 0.0 , -0.5);
    glVertex3d(0.5, 0.0 , -1.5);
    glVertex3d(-0.5, -1.5, -1.5);
    glVertex3d(-0.5, -1.5, -0.5);
    glEnd();
    //
    glColor3dv(green);
    glBegin(GL_POLYGON);
    glVertex3d(-0.5, 1.5, -0.5);
    glVertex3d(-1.5, 1.5, -0.5);
    glVertex3d(-1.5, 0.0, 0.5);
    glVertex3d(-0.5, 0.0, 0.5);
    glEnd();

    glColor3dv(red);
    glBegin(GL_POLYGON);
    glVertex3d(-0.5, 0.0 , -0.5);
    glVertex3d(-1.5, 0.0 , -0.5);
    glVertex3d(-1.5, -1.5, 0.5);
    glVertex3d(-0.5, -1.5, 0.5);
    glEnd();
}