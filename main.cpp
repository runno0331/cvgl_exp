#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <random>
#include <vector>
#include <utility>
#include <GL/glut.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define WINDOW_X (1000)
#define WINDOW_Y (1000)
#define WINDOW_NAME "main"
// 効果音
#define SHOOT "./sound/shoot1.wav"
#define BURST "./sound/burst.wav"
#define COUNTDOWN "./sound/countdown.wav"
#define COUNTDOWN_END "./sound/countdown_end.wav"
// 色相によるマスク処理時範囲
#define MIN_HSVCOLOR cv::Scalar(0, 60, 80)
#define MAX_HSVCOLOR cv::Scalar(10, 160, 240)
//自宅
/*
#define MIN_HSVCOLOR cv::Scalar(165, 40, 35)
#define MAX_HSVCOLOR cv::Scalar(180, 150, 200)
*/

#define TEXTURE_HEIGHT (130)
#define TEXTURE_WIDTH_NO (80)

void die(std::string s){
    printf("%s\n", s.c_str());
    exit(1);
}
// 初期化関数
void init_GL(int argc, char *argv[]);
void init_frame(int argc, char *argv[]);
void init_textures();
void init();
void set_textures();
void set_mode(int argc, char *argv[]);
void set_callback_functions();
// 操作系
void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);
void glut_idle();
// 方向加速度計算
void calculate_acceleration();
// 描画関数
void draw_targets();
void draw_field();
void draw_circle();
void draw_string(char* str, int x, int y);
void draw_score();
void draw_result();
void draw_startscreen();
void draw_pausescreen();
void draw_start();
// 衝突判定
void break_target();
// 効果音再生
void play_sound(const char* filename);
// fps測定用
void check_framerate();

// グローバル変数
GLdouble box_size = 40.0; // フィールドの箱の1辺の長さ/2
double g_angle1 = 0.0;
double g_angle2 = 0.0;
bool g_isLeftButtonOn = false;
bool shoot_flag = false; // 射撃時
char buf[100];

time_t start_time, current_time, detected_start, pause_start;
int score = 0;
int time_limit = 60;
int pause_time = 0; // ポーズ累積時間

int frame_x = 640, frame_y = 480; // フレームサイズ
int px = WINDOW_X / 2, py = WINDOW_Y / 2; // ポインタ位置

std::chrono::system_clock::time_point now_time, prev_time, prevprev_time; // 微分用時間
std::vector<cv::Point2d> place; // 過去の座標
// 前処理用画像
cv::VideoCapture cap;
cv::Mat frame;
cv::Mat tmp_img;
cv::Mat msk_img, hsv_img, result;

GLuint g_TextureHandles[25] = {};

int mouse_flag = 0; // マウス操作
int preprocess_flag = 0; // 前処理表示
int hand_flag = 0; // 手認識

/*
0: スタート画面
1: プレイモード
2: 結果出力モード
3: ポーズモード
4: スタートカウントダウン
*/
int g_display_mode = 0; 

std::random_device rnd; // 乱数生成器

class Target{
private:
    double vx, vy, vz;
    GLfloat facecolors[6][4] = {{1.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 1.0}, {0.0, 0.0, 1.0, 1.0},
                                {1.0, 1.0, 0.0, 1.0}, {0.0, 1.0, 1.0, 1.0}, {1.0, 0.0, 1.0, 1.0}};
    
public:
    double x, y, z, radius;
    double v; // スコア用
    void init();
    void move();
    GLfloat facecolor[4] = {};
public:
    bool operator<(const Target& other)const;
};

void Target::init(){
    x = rnd() % (int)(box_size * 2) - box_size;
    y = rnd() % (int)(box_size * 2);
    z = rnd() % (int)(box_size * 2) - box_size;
    vx = (double)(rnd() % 2 * 2.0 - 1.0) * 0.02*(double)(rnd()%5);
    vy = (double)(rnd() % 2 * 2.0 - 1.0) * 0.02*(double)(rnd()%5);
    vz = (double)(rnd() % 2 * 2.0 - 1.0) * 0.02*(double)(rnd()%5);
    v = (vx*vx + vy*vy + vz*vz) * 1000;
    radius = 0.5 * (double)(rnd() % 6 + 2);
    int i = rnd()%6;
    for(int j=0; j<4; j++){
        facecolor[j] = facecolors[i][j];
    }
}

void Target::move(){
    x += vx; y += vy; z += vz; // 位置更新
    // フィールド内か判定
    if(x > box_size + radius){
        x = -box_size - radius;
    }else if(x < -box_size - radius){
        x = box_size + radius;
    }
    if(y > 2.0 * box_size + radius + 1.0){
        y = -radius - 1.0;
    }else if(y < -radius - 1.0){
        y = 2.0 * box_size + radius + 1.0;
    }
    if(z > box_size + radius){
        z = -box_size - radius;
    }else if(z < -box_size - radius){
        z = box_size + radius;
    }
}

// カメラ位置からの距離の比較
bool Target::operator<(const Target& other)const{
    double d1, d2;
    d1 = x*x + (y-1.0)*(y-1.0) + z*z;
    d2 = other.x*other.x + (other.y-1.0)*(other.y-1.0) + other.z*other.z;
    return d1 < d2;
}
// 的たち
std::vector<Target> targets;

int main(int argc, char *argv[]){
    init_GL(argc, argv);
    set_mode(argc, argv);
    init();
    init_textures();
    set_callback_functions();
    if(!mouse_flag){
        init_frame(argc, argv);
    }
    glutMainLoop();

    return 0;
}
// OpenCV関連のフレーム・ウィンドウ初期化
void init_frame(int argc, char *argv[]){
    if(argc > 1){
        char *in_name = argv[1];
        if('0' <= in_name[0] && in_name[0] <= '9' && in_name[1] == '\0'){
            cap.open(in_name[0] - '0'); // 番号指定
        }else{
            cap.open(in_name); // ファイル名
        }
    }else{
        cap.open(0); // デフォルト
    }

    if(!cap.isOpened()) die("cannot open video");
    if(preprocess_flag){
        // 前処理過程の画像
        //cv::namedWindow("frame", 1);
        //cv::namedWindow("hsv", 1);
        cv::namedWindow("mask", 1);
        //cv::namedWindow("result", 1);
        cv::namedWindow("contour");
        cv::moveWindow("mask", 1200, 0);
        cv::moveWindow("contour", 1200, 500);
    }

    // フレーム取得
    cap >> frame;
    if(frame.empty()) die("empty frame");

    // フレームサイズの取得
    cv::Size s = frame.size();
    frame_x = s.width; frame_y = s.height; 

    tmp_img.create(s, CV_32FC3);
    result.create(s, CV_32FC3);
    msk_img.create(s, CV_8UC1);
}

void init_GL(int argc, char *argv[]){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(WINDOW_X, WINDOW_Y);
    glutCreateWindow(WINDOW_NAME);
}

void init_textures(){
    glGenTextures(25, g_TextureHandles);
    for(int i = 0; i < 22; i++){
        glBindTexture(GL_TEXTURE_2D, g_TextureHandles[i]);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        if(i == 11 || i == 14 || i == 19){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1200, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }else if(i == 12){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 650, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }else if(i == 13 || i == 17 || i == 21){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 500, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }else if(i == 15){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }else if(i == 16){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 2*TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }else if(i == 18){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1300, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }else if(i == 20){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 900, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }else{
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH_NO, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    set_textures();
}

void init(){
    glClearColor(0.0, 0.0, 0.0, 0.0); // 背景の塗りつぶし色を指定
    place.clear();
    // 的初期化
    targets.clear();
    Target new_target;
    for(int i=0; i<20; i++){ // 10個の的
        new_target.init();
        targets.push_back(new_target);
    }
    // 視点の初期化
    g_angle1 = 0; g_angle2 = 0;
    px = WINDOW_X / 2; py = WINDOW_Y / 2;
    // スコアの初期化
    score = 0;
    pause_time = 0;
}

void set_mode(int argc, char *argv[]){
    if(argc > 2){
        int mode_n = atoi(argv[2]);
        if(0<= mode_n && mode_n < 3){
            mouse_flag = mode_n & 1;
            mode_n >>= 1;
            preprocess_flag = mode_n & 1;
        }
    }
}

void set_textures(){
    const char* inputFileNames[22] = {"./img/no0.png", "./img/no1.png", "./img/no2.png", "./img/no3.png", 
                                      "./img/no4.png", "./img/no5.png", "./img/no6.png", "./img/no7.png", 
                                      "./img/no8.png", "./img/no9.png", "./img/blank.png", "./img/restart.png",
                                      "./img/score.png", "./img/result.png", "./img/wave.png", "./img/click.png",
                                      "./img/title.png", "./img/pause.png", "./img/show.png", "./img/resume.png",
                                      "./img/detected.png", "./img/start.png"};
    for(int i = 0; i < 22; i++){
        cv::Mat input = cv::imread(inputFileNames[i], 1);
        // BGR −> RGB の変換
        cv::cvtColor(input, input, CV_BGR2RGB);
        glBindTexture(GL_TEXTURE_2D, g_TextureHandles[i]);
        if(i == 11 || i == 14 || i == 19){
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                    (1200 - input.cols) / 2,
                    (TEXTURE_HEIGHT - input.rows) / 2,
                    input.cols, input.rows,
                    GL_RGB, GL_UNSIGNED_BYTE, input.data);
        }else if(i == 12){
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                    (650 - input.cols) / 2,
                    (TEXTURE_HEIGHT - input.rows) / 2,
                    input.cols, input.rows,
                    GL_RGB, GL_UNSIGNED_BYTE, input.data);
        }else if(i == 13 || i == 17 || i == 21){
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                    (500 - input.cols) / 2,
                    (TEXTURE_HEIGHT - input.rows) / 2,
                    input.cols, input.rows,
                    GL_RGB, GL_UNSIGNED_BYTE, input.data);
        }else if(i == 15){
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                    (800 - input.cols) / 2,
                    (TEXTURE_HEIGHT - input.rows) / 2,
                    input.cols, input.rows,
                    GL_RGB, GL_UNSIGNED_BYTE, input.data);
        }else if(i == 16){
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                    (800 - input.cols) / 2,
                    (2*TEXTURE_HEIGHT - input.rows) / 2,
                    input.cols, input.rows,
                    GL_RGB, GL_UNSIGNED_BYTE, input.data);
        }else if(i == 18){
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                    (1300 - input.cols) / 2,
                    (TEXTURE_HEIGHT - input.rows) / 2,
                    input.cols, input.rows,
                    GL_RGB, GL_UNSIGNED_BYTE, input.data);
        }else if(i == 20){
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                    (900 - input.cols) / 2,
                    (TEXTURE_HEIGHT - input.rows) / 2,
                    input.cols, input.rows,
                    GL_RGB, GL_UNSIGNED_BYTE, input.data);
        }else{
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                            (TEXTURE_WIDTH_NO - input.cols) / 2,
                            (TEXTURE_HEIGHT - input.rows) / 2,
                            input.cols, input.rows,
                            GL_RGB, GL_UNSIGNED_BYTE, input.data);
        }
    }    
}

void set_callback_functions(){
    glutDisplayFunc(glut_display);
    glutKeyboardFunc(glut_keyboard);
    if(mouse_flag){
        glutMouseFunc(glut_mouse);
        glutMotionFunc(glut_motion);
        glutPassiveMotionFunc(glut_motion);
    }
    glutIdleFunc(glut_idle);
}

void glut_display(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, 1.0, 0.1, 150);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if(g_display_mode == 0){
        gluLookAt(0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
        draw_startscreen();
        glPopMatrix();
        glFlush();
    }else if(g_display_mode == 1 || g_display_mode == 4){
        gluLookAt(0.0, 1.0, 0.0,
                cos(g_angle2/180.0*M_PI) * sin(g_angle1/180.0*M_PI),
                sin(g_angle2/180.0*M_PI)+1.0,
                cos(g_angle2/180.0*M_PI) * cos(g_angle1/180.0*M_PI),
                0.0, 1.0, 0.0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        
        // 照準
        glPushMatrix();
        draw_circle();
        glPopMatrix;

		if(g_display_mode == 4){
			glPushMatrix();
			draw_start();
			glPopMatrix();
		}

        // 照明位置
        GLfloat light_y = box_size * 2.0 - 5.0, bs = box_size - 1.0;
        // 照明設定
        int lights[] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2, GL_LIGHT3};
        GLfloat light_direction[] = {0.0, 1.0, 0.0};
        GLfloat lightpos[][4] = {{-bs, light_y, -bs, 1.0},
                                {-bs, light_y, bs, 1.0},
                                {bs, light_y, -bs, 1.0},
                                {bs, light_y, bs, 1.0}};
        GLfloat diffuse[] = {1.0, 1.0, 1.0, 1.0};
        glEnable(GL_LIGHTING);
        for(int i=0; i<4; i++){
            glEnable(lights[i]);
            glLightfv(lights[i], GL_POSITION, lightpos[i]);
            glLightfv(lights[i], GL_DIFFUSE, diffuse);
            glLightf(lights[i], GL_LINEAR_ATTENUATION, 0.01);
        }
        draw_field();
        if(g_display_mode == 1){
            draw_targets();

            glPushMatrix();
            draw_score();
            glPopMatrix();
        }

        glFlush();

        // 照明解除
        for(int i=0; i<4; i++){
            glDisable(lights[i]);
        }
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

    }else if(g_display_mode == 2){
        gluLookAt(0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
        draw_result();
        glPopMatrix();
        glFlush();
    }else if(g_display_mode == 3){
        gluLookAt(0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPushMatrix();
        draw_pausescreen();
        glPopMatrix();
        glFlush();
    }
    
    glutSwapBuffers();
}

void glut_keyboard(unsigned char key, int x, int y){
    switch (key)
    {
    case 'q':
    case 'Q':
    case '\033':
        exit(0);
    }
    if(g_display_mode == 1){
        if(key == 'p' || key == 'P'){
            time(&pause_start);
            g_display_mode = 3;
        }
    }
    if(g_display_mode == 2){
        if(key == 'r' || key == 'R'){
            init();
            g_display_mode = 4;
			time(&start_time);
        }
    }
    if(g_display_mode == 3){
        if(key == 'r' || key == 'R'){
            init();
            g_display_mode = 4;
			time(&start_time);
        }
        if(key == 's' || key == 'S'){
            time(&current_time);
            pause_time += (int)current_time - (int)pause_start;
            place.clear();
            g_display_mode = 1;
        }
    }

    glutPostRedisplay();
}

void glut_mouse(int button, int state, int x, int y){
    if(g_display_mode == 1){
        if (button == GLUT_LEFT_BUTTON){
            if (state == GLUT_UP){
                g_isLeftButtonOn = false;
                shoot_flag = false;
            }else if (state == GLUT_DOWN){
                if(!g_isLeftButtonOn){
                    if(g_display_mode == 1){
                        place.push_back(cv::Point2f(px, py));
                        break_target();
                        place.erase(place.begin());
                        glutPostRedisplay();
                        shoot_flag = true;
                    }
                }else{
                    shoot_flag = false;
                }
                g_isLeftButtonOn = true;
            }
        }
    }else if(g_display_mode == 0){
        if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
            g_display_mode = 4;
            time(&start_time);
        }
    }
}

void glut_motion(int x, int y){
    px = x;
    py = y;
}

void glut_idle(){
    // check_framerate();

    if(g_display_mode == 1){
        for (int i = 0; i < targets.size(); i++){
            targets[i].move();
        }
        if(px<30){
            g_angle1++;
        }else if(WINDOW_X - 30 < px){
            g_angle1--;
        }
        if(py<30){
            g_angle2++;
            if(g_angle2 > 90) g_angle2 = 90;
        }else if(WINDOW_Y - 30 < py){
            g_angle2--;
            if(g_angle2 < 0) g_angle2 = 0;
        }
    }
    glutPostRedisplay();
    
    if(!mouse_flag){
        cap >> frame;
        if(frame.empty()) die("empty frame");

        tmp_img = frame.clone();

        // 平滑化
        cv::Size ksize = cv::Size(5, 5);
        cv::GaussianBlur(tmp_img, tmp_img, ksize, 0);
        //cv::bilateralFilter(tmp_img1, tmp_img, -1, 50, 5);

        // HSV色空間に変換
        cv::cvtColor(tmp_img, hsv_img, CV_BGR2HSV);

        // HSV色空間における肌色の検出
        cv::inRange(hsv_img, MIN_HSVCOLOR, MAX_HSVCOLOR, msk_img);


        //　マスク領域の膨張・縮小
        cv::erode(msk_img, msk_img, cv::Mat(), cv::Point(-1,-1), 2);
        cv::dilate(msk_img, msk_img, cv::Mat(), cv::Point(-1,-1), 3);

        // 輪郭の長さから手を探す
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(msk_img, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        std::vector<std::vector<cv::Point>> contours_subset;
        for(int i=0; i<contours.size();i++){
            double area = contourArea(contours.at(i));
            //printf("%f\n" ,area);
            if(area>7000 && area<50000){
                contours_subset.push_back(contours.at(i));
            }
        }

        msk_img = cv::Mat::zeros(msk_img.rows, msk_img.cols, CV_8UC1);
        drawContours(msk_img,contours_subset,0,cv::Scalar(255),-1);

        hand_flag = contours_subset.size();
        // 手が一定時間認識できなければポーズ
        if(g_display_mode == 1){
            if(hand_flag){
                time(&detected_start);
            }else{
                time(&current_time);
                if((int)current_time - (int)detected_start > 2){
                    time(&pause_start);
                    detected_start = current_time;
                    g_display_mode = 3;
                }
            }
        }

        // ポーズからの復帰
        if(g_display_mode == 3){
            if(hand_flag){
                time(&current_time);
                if((int)current_time - (int)detected_start > 1){
                    time(&current_time);
                    pause_time += (int)current_time - pause_start;
                    detected_start = current_time;
                    place.clear();
                    g_display_mode = 1;
                }
            }else{
                time(&detected_start);
            }
        }

        // マスク処理後画像の生成
        tmp_img = frame.clone();
        result = cv::Scalar(0.0, 0.0, 0.0);
        tmp_img.copyTo(result, msk_img);

        if(preprocess_flag){
            //cv::imshow("frame", frame);
            //cv::imshow("hsv", hsv_img);
            cv::imshow("mask", msk_img);
            //cv::imshow("result", result);
        }

        cv::Mat contour_img;
        contour_img = frame.clone();

        int contour_index = 0, max_level = 0;

        /// 輪郭の描画
        // 画像，輪郭，描画輪郭指定インデックス，色，太さ，種類，階層構造，描画輪郭の最大レベル
        cv::drawContours(contour_img, contours_subset, contour_index, cv::Scalar(0, 0, 200), 2, CV_AA);
        double hand_area = 100.0;

        // マスク画像の重心を指示位置とする
        cv::Moments mu = cv::moments(msk_img, true);
        cv::Point2f mc = cv::Point2f(mu.m10/mu.m00, mu.m01/mu.m00);

        px = (1.2*(float)WINDOW_X - 1.4*((float)mc.x/(float)frame_x)*(float)WINDOW_X);
        py = 1.4*(((float)mc.y/(float)frame_y)*(float)WINDOW_Y) - 0.2*(float)WINDOW_Y;
        cv::Point2f position = cv::Point2f(px, py);
        place.push_back(position);
        calculate_acceleration();

        cv::circle(contour_img, mc, (int)sqrt(hand_area/3), cv::Scalar(100), 2, 4);
        cv::circle(contour_img, mc, 10, cv::Scalar(100), -1);
        if(preprocess_flag){
            cv::imshow("contour", contour_img);
        }
        char key = cv::waitKey(10);
        
        glutPostRedisplay();
    }
}

void calculate_acceleration(){
    now_time = std::chrono::system_clock::now();
    if(place.size() > 2){
		if(g_display_mode == 1){
			double acceleration;
			double time0 = (double)(std::chrono::duration_cast<std::chrono::microseconds>(prev_time - prevprev_time).count() / 1000.0);
			double time1 = (double)(std::chrono::duration_cast<std::chrono::microseconds>(now_time - prev_time).count() / 1000.0);
			acceleration = (((double)place[2].y-(double)place[1].y)*time0 - ((double)place[1].y-(double)place[0].y)*time1) / (time0*time0*time1);
            if(acceleration < 1 && acceleration > 0.09 && place[2].y - place[1].y > 0){
                // printf("shoot %lf\n", acceleration);
				break_target();
				shoot_flag = true;
			}else{
				shoot_flag = false;
			}
		}else if(g_display_mode == 0){
            double time0 = (double)(std::chrono::duration_cast<std::chrono::microseconds>(now_time - prevprev_time).count() / 1000.0);
			if(2 < abs(place[0].x - place[2].x)/time0 && abs(place[0].x - place[2].x)/time0 < 5){
                g_display_mode = 4;
                time(&start_time);
				//printf("%lf\n",  abs(place[0].x - place[2].x)/time0);
			}
            //printf("%lf\n",  abs(place[0].x - place[2].x)/time0);
		}
        place.erase(place.begin());
    }
    prevprev_time = prev_time;
    prev_time = now_time;
}

void draw_targets(){
    for (int i = 0; i < targets.size(); i++){
        glPushMatrix();
        glMaterialfv(GL_FRONT, GL_DIFFUSE, targets[i].facecolor);
        glTranslatef(targets[i].x, targets[i].y, targets[i].z);
        glutSolidSphere(targets[i].radius, 50, 50);
        glPopMatrix();
    }
}

void draw_field(){
    const GLfloat ground_color[][4] = {{0.6, 0.6, 0.6, 1.0}, {0.3, 0.3, 0.3, 1.0}, {0.8, 0.8, 0.8, 1.0}, {0.5, 0.5, 0.5, 1.0}};

    glBegin(GL_QUADS);
    glNormal3d(0.0, 1.0, 0.0);
    for (int j = -box_size; j < box_size; j+=2){
        for (int i = -box_size; i < box_size; i+=2){
            glMaterialfv(GL_FRONT, GL_DIFFUSE, ground_color[(i + j)/2 & 1]);
            glVertex3d((GLdouble)i, -1.0, (GLdouble)j);
            glVertex3d((GLdouble)i, -1.0, (GLdouble)(j + 2));
            glVertex3d((GLdouble)(i + 2), -1.0, (GLdouble)(j + 2));
            glVertex3d((GLdouble)(i + 2), -1.0, (GLdouble)j);
        }
    }
    glEnd();
    
    glBegin(GL_QUADS);
    glNormal3d(0.0, -1.0, 0.0);
    for (int j = -box_size; j < box_size; j+=2){
        for (int i = -box_size; i < box_size; i+=2){
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ground_color[(i + j)/2 & 1]);
            glVertex3d((GLdouble)i, box_size*2.0+1.0, (GLdouble)j);
            glVertex3d((GLdouble)i, box_size*2.0+1.0, (GLdouble)(j + 2));
            glVertex3d((GLdouble)(i + 2), box_size*2.0+1.0, (GLdouble)(j + 2));
            glVertex3d((GLdouble)(i + 2), box_size*2.0+1.0, (GLdouble)j);
        }
    }
    glEnd();

    GLdouble pointA[] = {box_size, -1.0, box_size};
    GLdouble pointB[] = {box_size, -1.0, -box_size};
    GLdouble pointC[] = {-box_size, -1.0, -box_size};
    GLdouble pointD[] = {-box_size, -1.0, box_size};
    GLdouble pointE[] = {box_size, box_size*2.0+1.0, box_size};
    GLdouble pointF[] = {box_size, box_size*2.0+1.0, -box_size};
    GLdouble pointG[] = {-box_size, box_size*2.0+1.0, -box_size};
    GLdouble pointH[] = {-box_size, box_size*2.0+1.0, box_size};
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ground_color[2]);
    glNormal3d(-1.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointF);
    glVertex3dv(pointE);
    glEnd();

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ground_color[2]);
    glNormal3d(1.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex3dv(pointC);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointG);
    glEnd();

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ground_color[3]);
    glNormal3d(0.0, 0.0, 1.0);
    glBegin(GL_QUADS);
    glVertex3dv(pointB);
    glVertex3dv(pointC);
    glVertex3dv(pointG);
    glVertex3dv(pointF);
    glEnd();

    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ground_color[3]);
    glNormal3d(0.0, 0.0, -1.0);
    glBegin(GL_QUADS);
    glVertex3dv(pointA);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointE);
    glEnd();
}

void draw_circle(){
    double x = -((double)px*2.0/WINDOW_X - 1.0)*0.027;
    double y = -((double)py*2.0/WINDOW_Y - 1.0)*0.027;
    
    glPushMatrix();
    if(shoot_flag){ glColor3d(1.0, 1.0, 0.0);}
    else{ glColor3d(0.0, 0.0, 1.0);}
    glTranslated(0.0, 1.0, 0.0);
    glTranslated(0.1*cos(g_angle2/180.0*M_PI) * sin(g_angle1/180.0*M_PI),
              0.1*sin(g_angle2/180.0*M_PI),
              0.1*cos(g_angle2/180.0*M_PI) * cos(g_angle1/180.0*M_PI));
    // 狙い位置のウィンドウ左右方向の変化
    glTranslated(x * cos(g_angle1/180.0*M_PI),
                 0.0,
                -x * sin(g_angle1/180.0*M_PI));
    // 狙い位置のウィンドウ上下方向の変化
    glTranslated(- y * sin(g_angle2/180.0*M_PI) * sin(g_angle1/180.0*M_PI),
              y * cos(g_angle2/180.0*M_PI),
              - y * sin(g_angle2/180.0*M_PI) * cos(g_angle1/180.0*M_PI));

    glutSolidSphere(0.0005, 50, 50);
    glPopMatrix();
}

void draw_string(char str[], int x, int y){
    glDisable(GL_LIGHTING);
    // 平行投影にする
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_X, WINDOW_Y, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 画面上にテキスト描画
    glRasterPos2f(x, y);
    int size = (int)strlen(str);
    for(int i = 0; i < size; ++i){
        char ic = str[i];
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, ic);
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void draw_score(){
    static int previous_time = -1;
    time(&current_time);
    int time_left = time_limit + pause_time - (int)current_time + (int)start_time;

    if(previous_time != time_left){
        if(time_left == 0) play_sound(COUNTDOWN_END);
        else if(0 < time_left && time_left < 5) play_sound(COUNTDOWN);
    }
    
    if(time_left < 10){ glColor3d(1.0, 0.0, 0.0);}
    else if(time_left < time_limit / 2){ glColor3d(1.0, 1.0, 0.0);}
    else{ glColor3d(1.0, 1.0, 1.0);}
    
    sprintf(buf, "Time left: %ds", time_left);
    draw_string(buf, 10, 30);
    glColor3d(1.0, 1.0, 1.0);
    sprintf(buf, "Score: %d", score);
    draw_string(buf, 10, 55);

    if(time_left < 0){
		g_display_mode = 2;
		previous_time = -1;
	} 
    previous_time = time_left;
}

void draw_result(){
	glColor3d(1.0, 1.0, 1.0);
    glTranslatef(20.0, 0.0, 0.0);

    // 文字描画 
    // restart
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_TextureHandles[13]); // result
    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(0.0, 4.2, 5.0);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(0.0, 1.6, 5.0);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(0.0, 1.6, -5.0);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(0.0, 4.2, -5.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_TextureHandles[12]); // score
    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(0.0, 1.0, 1.5);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(0.0, 0.0, 1.5);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(0.0, 0.0, -3.5);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(0.0, 1.0, -3.5);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // スコア描画
    int temp = score;
    for(double i=0; i<8; i+=0.9){
        int num = temp%10;
        glEnable(GL_TEXTURE_2D);
        if(temp == 0 && i != 0){ // blank
            glBindTexture(GL_TEXTURE_2D, g_TextureHandles[10]);
        }else{
            glBindTexture(GL_TEXTURE_2D, g_TextureHandles[num]);
        }
        glBegin(GL_QUADS);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(0.0, 0.0, 0.8*(4.0-i)-0.2);
        glTexCoord2d(1.0, 1.0);
        glVertex3d(0.0, -1.3, 0.8*(4.0-i)-0.2);
        glTexCoord2d(0.0, 1.0);
        glVertex3d(0.0, -1.3, 0.8*(3.0-i)-0.2);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(0.0, 0.0, 0.8*(3.0-i)-0.2);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        temp /= 10;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_TextureHandles[11]); // restart
    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(0.0, -2.0, 2.5);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(0.0, -2.8, 2.5);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(0.0, -2.8, -2.5);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(0.0, -2.0, -2.5);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void draw_startscreen(){
	glColor3d(1.0, 1.0, 1.0);
	glTranslatef(20.0, 0.0, 0.0);
    // 文字描画 
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_TextureHandles[16]); // title
    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(0.0, 3.6, 5.0);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(0.0, 0.0, 5.0);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(0.0, 0.0, -5.0);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(0.0, 3.6, -5.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

	double dx = 0.0;

	glEnable(GL_TEXTURE_2D);
	if(mouse_flag){
        glBindTexture(GL_TEXTURE_2D, g_TextureHandles[15]); // click
    }else{
        glBindTexture(GL_TEXTURE_2D, g_TextureHandles[14]); // wave
		dx = 0.5;
    }
    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(0.0, -1.8, 2.5+dx);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(0.0, -2.6, 2.5+dx);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(0.0, -2.6, -2.5-dx);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(0.0, -1.8, -2.5-dx);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    if(hand_flag && !mouse_flag){
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_TextureHandles[20]); // detected
        glBegin(GL_QUADS);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(0.0, -3.8, 2.5);
        glTexCoord2d(1.0, 1.0);
        glVertex3d(0.0, -4.6, 2.5);
        glTexCoord2d(0.0, 1.0);
        glVertex3d(0.0, -4.6, -2.5);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(0.0, -3.8, -2.5);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
}

void draw_pausescreen(){
    glColor3d(1.0, 1.0, 1.0);
	glTranslatef(20.0, 0.0, 0.0);
    // 文字描画 
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_TextureHandles[17]); // pause
    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(0.0, 3.2, 5.0);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(0.0, 0.6, 5.0);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(0.0, 0.6, -5.0);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(0.0, 3.2, -5.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    double dx = 0.0;
    glEnable(GL_TEXTURE_2D);
	if(mouse_flag){
        glBindTexture(GL_TEXTURE_2D, g_TextureHandles[19]); // press s to resume
    }else{
        glBindTexture(GL_TEXTURE_2D, g_TextureHandles[18]); // show
		dx = 0.5;
    }
    glBegin(GL_QUADS);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(0.0, -1.8, 2.5+dx);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(0.0, -2.6, 2.5+dx);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(0.0, -2.6, -2.5-dx);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(0.0, -1.8, -2.5-dx);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    if(hand_flag && !mouse_flag){
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_TextureHandles[20]); // detected
        glBegin(GL_QUADS);
        glTexCoord2d(1.0, 0.0);
        glVertex3d(0.0, -3.8, 2.5);
        glTexCoord2d(1.0, 1.0);
        glVertex3d(0.0, -4.6, 2.5);
        glTexCoord2d(0.0, 1.0);
        glVertex3d(0.0, -4.6, -2.5);
        glTexCoord2d(0.0, 0.0);
        glVertex3d(0.0, -3.8, -2.5);
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
}

void draw_start(){
	glColor4d(1.0, 1.0, 1.0, 0.0);
    static int previous_time = -1;
    time(&current_time);
    int time_diff = (int)current_time - (int)start_time;

    if(previous_time != time_diff){
        if(time_diff == 3) play_sound(COUNTDOWN_END);
        else if(0 <= time_diff && time_diff < 3) play_sound(COUNTDOWN);
    }

    // texture
	double dx = 0.0;
	glTranslatef(0, 1.0, 20.0);
	glEnable(GL_TEXTURE_2D);
	if(time_diff == 3){ // start
		glBindTexture(GL_TEXTURE_2D, g_TextureHandles[21]);
		dx = 2.0;
	}else if(0 <= time_diff && time_diff < 3){ // num
		glBindTexture(GL_TEXTURE_2D, g_TextureHandles[3-time_diff]);
	}
	glBegin(GL_QUADS);
    glTexCoord2d(1.0, 0.0);
    glVertex3d(-1.0-dx, 3.6, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex3d(-1.0-dx, 0.0, 0.0);
    glTexCoord2d(0.0, 1.0);
    glVertex3d(1.0+dx, 0.0, 0.0);
    glTexCoord2d(0.0, 0.0);
    glVertex3d(1.0+dx, 3.6, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
    
    if(time_diff > 3){
        place.clear();
        g_display_mode = 1;
        time(&start_time);
    }
    previous_time = time_diff;
}

void break_target(){
    play_sound(SHOOT);
    double x = -((double)place[0].x*2.0/WINDOW_X - 1.0)*0.027;
    double y = -((double)place[0].y*2.0/WINDOW_Y - 1.0)*0.027;
    // 射線の方向ベクトル
    double ux, uy, uz;
    ux = 0.1*cos(g_angle2/180.0*M_PI) * sin(g_angle1/180.0*M_PI) + x * cos(g_angle1/180.0*M_PI) - y * sin(g_angle2/180.0*M_PI) * sin(g_angle1/180.0*M_PI);
    uy = 0.1*sin(g_angle2/180.0*M_PI) + y * cos(g_angle2/180.0*M_PI);
    uz = 0.1*cos(g_angle2/180.0*M_PI) * cos(g_angle1/180.0*M_PI) - x * sin(g_angle1/180.0*M_PI) - y * sin(g_angle2/180.0*M_PI) * cos(g_angle1/180.0*M_PI);
    double threshold;
    threshold = sqrt(ux*ux + uy*uy + uz*uz);
    std::sort(targets.begin(), targets.end(), [](Target a, Target b) -> int{return(a < b);});
    for(int i=0; i<targets.size(); i++){
        // カメラ位置と的中心を結ぶベクトルと射線方向ベクトルとのクロス積
        double cross_scalar, cmx, cmy, cmz;
        cmx = (targets[i].y-1.0) * uz - targets[i].z * uy;
        cmy = targets[i].z * ux - targets[i].x * uz;
        cmz = targets[i].x * uy - (targets[i].y-1.0) * ux;
        cross_scalar = sqrt(cmx*cmx + cmy*cmy + cmz*cmz);
        if(cross_scalar < threshold * targets[i].radius){ // hit
            targets.erase(targets.begin()+i);
            play_sound(BURST);
            Target new_target;
            new_target.init();
            targets.push_back(new_target);
            score += (4.0-targets[i].radius)*targets[i].v*(targets[i].x*targets[i].x+targets[i].y*targets[i].y+targets[i].z*targets[i].z)/10 + 1;
            break;
        }
    }
}

void play_sound(const char filename[]){
	static char cmd[256];

	if (strlen(filename) > 200) return;	// バッファオーバーフロー防止
	sprintf(cmd, "paplay %s &> /dev/null &", filename);
	system(cmd);
}

void check_framerate(){
    time_t test_t;
    static int test_c = 0, test_t0;
        time(&test_t);
    if(test_c == 0){
        test_t0 = (int)test_t;
    }else if(test_t0 != (int)test_t){
        printf("%d\n", test_c);
        test_c=0;
        test_t0 = (int)test_t;
    }
    test_c++;
}