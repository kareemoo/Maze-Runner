#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <sstream>  
#include <stdlib.h>
#include <glut.h>
#include <iostream>
#include <windows.h>
#include <queue>
#include <vector>
#include <playsoundapi.h>
using namespace std;

#define DEG2RAD(a) (a * 0.0174532925)
#define abs(a) (a>=0?a:-a)
int WIDTH = 1280;
int HEIGHT = 720;

char title[] = "Maze Runner";

struct triple {
	int x, y, z;
};

vector<vector<int>> maze1 =
{ {1,1,1,1,1,0,1,1,1,1,1},
	{1,0,0,0,0,0,0,0,1,2,1},
	{1,0,1,0,1,1,1,0,1,0,1},
	{1,0,1,0,0,0,1,0,0,0,1},
	{1,0,1,1,1,0,1,1,1,1,1},
	{1,0,0,0,1,0,0,0,0,2,1},
	{1,1,1,0,1,1,1,1,1,0,1},
	{1,2,1,0,1,0,0,0,1,0,1},
	{1,0,1,0,1,0,1,0,1,0,1},
	{1,0,0,0,1,0,1,0,0,0,1},
	{1,1,1,1,1,3,1,1,1,1,1} };

vector<vector<int>> maze2 =
{ {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1},
  {1,0,0,0,1,0,1,0,1,0,0,0,0,0,1},
  {1,0,1,0,0,0,1,0,1,0,1,1,1,0,1},
  {1,0,1,0,1,0,0,0,0,0,0,0,0,0,1},
  {1,0,1,0,1,1,1,0,1,1,1,1,1,0,1},
  {1,0,0,0,0,0,1,0,0,0,1,5,0,0,1},
  {1,0,1,1,1,0,1,1,1,1,1,0,1,1,1},
  {1,0,0,0,1,0,0,0,0,0,0,0,1,0,1},
  {1,0,1,0,1,0,1,0,1,0,1,1,1,0,1},
  {1,0,1,0,0,0,1,0,1,0,0,0,0,0,1},
  {1,0,1,1,1,0,1,0,1,1,1,0,1,0,1},
  {1,0,1,0,0,0,1,0,1,0,0,0,1,0,1},
  {1,5,1,0,1,0,1,0,1,0,1,1,1,0,1},
  {1,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
  {1,1,1,1,1,1,1,0,1,1,1,1,1,1,1} };

bool visited[15][15];

bool moveUp, moveDown, moveRight, moveLeft;
bool win = false, lose = false;
bool passLevel1 = false;
bool afsha = false;
int keys = 0;

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

double moveX = -5, moveZ, enemyX = -7, enemyZ;
int enemyLastX = -7, enemyLastZ;
int playerLastX = -5, playerLastZ;
double angle = -90, enemyAngle = 90;
int prev_x, prev_y;
double t = 0;
int level = 1;
double playerSpeed = 0.4;
int powerUpTime = 0;

class Vector3f {
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) {
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f& v) {
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f& v) {
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n) {
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n) {
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit() {
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v) {
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera {
public:
	Vector3f eye, center, up;
	Vector3f fromEyeToCenter = Vector3f(1, 0, 0);

	Camera(float eyeX = 1.0f, float eyeY = 1.0f, float eyeZ = 1.0f, float centerX = 0.0f, float centerY = 0.0f, float centerZ = 0.0f, float upX = 0.0f, float upY = 1.0f, float upZ = 0.0f) {
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d) {
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d) {
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d) {
		Vector3f view = (center - eye).unit();
		view.y = 0;
		view.x = 0;
		view.z = max(view.z, 1);
		eye = eye + view * d;
		center = center + view * d;
	}
	void rotateXRevTenfa3(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		eye = eye + view;
	}

	void rotateX(float a) {

		fromEyeToCenter = fromEyeToCenter * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		//center = eye + fromEyeToCenter;

	}
	void rotateXRev(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		eye = eye + view;
	}

	void rotateY(float a) {
		Vector3f right = up.cross(fromEyeToCenter).unit();
		fromEyeToCenter = fromEyeToCenter * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = fromEyeToCenter.cross(up);
		//center = eye + fromEyeToCenter;
	}
	void rotateYRev(float a) {
		//cout << "rotating Y"<<'\n';
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		eye = eye + view;
	}

	void rotateZ(float a) {
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		//up = (up * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a))).unit();
	}
	void rotateZRev(float a) {
		Vector3f view = (eye - center).unit();
		Vector3f right = up.cross(view).unit();
		//up = (up * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a))).unit();
	}

	void look(float Ex, float Ey, float Ez) {
		eye.x = Ex;
		eye.y = Ey;
		eye.z = Ez;
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z
		);
	}

	void topView() {
		eye = Vector3f(0, 20, 0.1);
		center = Vector3f(0, 0, 0);
		up = Vector3f(0, 1, 0);
	}

};

Camera camera;

// Textures
GLuint night;
GLuint sky;
GLTexture tex_grass;
GLTexture tex_sand;
GLTexture tex_wall;
GLTexture tex_bushes;
GLTexture tex_gate;

// Models
Model_3DS model_enemy;
Model_3DS model_man;
Model_3DS model_key;
Model_3DS model_power;

bool thirdPerson = false;

//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();

	//camera.look(moveX, 1, moveZ);
	//*******************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

void setupCamera() {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 640 / 480, 0.001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//cout << "in setup" << "\n";
	if (thirdPerson) {

		camera.center.x = moveX;
		camera.center.z = moveZ;
		camera.center.y = 1;
		camera.eye = camera.center - camera.fromEyeToCenter;
		camera.look(camera.eye.x, camera.eye.y, camera.eye.z);

	}
	else {

		camera.look(moveX, 1, moveZ);
		camera.center = camera.eye + camera.fromEyeToCenter;
	}
}

// track

bool valid2(int x, int y) {
	return x >= 0 && x < 15 && y >= 0 && y < 15 && (maze2[x][y] == 0 || maze2[x][y] == 5);
}

bool valid1(int x, int y) {
	return x >= 0 && x < 11 && y >= 0 && y < 11 && (maze1[x][y] == 0 || maze1[x][y] == 2);
}

int next_x[4] = { 0,0,1,-1 };
int next_y[4] = { 1,-1,0,0 };

int bfs(int sx, int sy, int ex, int ey) {
	if (sx == ex && sy == ey)
		return 0;
	queue<triple> q;
	q.push({ sx,sy,0 });
	int ans = 10000;
	while (!q.empty()) {
		triple u = q.front();
		visited[u.x][u.y] = true;
		if (u.x == ex && u.y == ey)
			ans = min(ans, u.z);
		q.pop();
		for (int i = 0; i < 4; i++) {
			int xNext = u.x + next_x[i], yNext = u.y + next_y[i];
			if (valid2(xNext, yNext) && !visited[xNext][yNext]) {
				q.push({ xNext, yNext, u.z + 1 });
			}
		}
	}
	return ans;
}

void track() {
	int sx = enemyLastX + 7, sz = enemyLastZ + 7, ex = playerLastX + 7, ez = playerLastZ + 7;

	if (sx == ex && sz == ez)
		return;
	int moveTo = -1, min = 10000;
	for (int i = 0; i < 4; i++) {
		if (valid2(sx + next_x[i], sz + next_y[i])) {
			memset(visited, 0, sizeof(visited[0][0]) * 15 * 15);
			int dist = bfs(sx + next_x[i], sz + next_y[i], ex, ez);
			if (dist < min) {
				moveTo = i;
				min = dist;
			}
		}
	}

	//cout << moveX << ' ' << moveZ << ' ' << playerLastX << ' ' << playerLastZ << '\n';
	//cout << enemyX << ' ' << enemyZ << ' ' << enemyLastX << ' ' << enemyLastZ << ' ' << min << '\n';
	if (moveTo == 0) {
		if (enemyZ + 0.05 >= enemyLastZ + 1) {
			enemyAngle = 0;
			enemyLastZ++;
		}
		enemyZ += 0.05;
	}
	else if (moveTo == 1) {
		if (enemyZ - 0.05 <= enemyLastZ - 1) {
			enemyAngle = 180;
			enemyLastZ--;
		}
		enemyZ -= 0.05;
	}
	else if (moveTo == 2) {
		if (enemyX + 0.05 >= enemyLastX + 1) {
			enemyAngle = 90;
			enemyLastX++;
		}
		enemyX += 0.05;
	}
	else if (moveTo == 3) {
		if (enemyX - 0.05 <= enemyLastX - 1) {
			enemyAngle = -90;
			enemyLastX--;
		}
		enemyX -= 0.05;
	}
}

//=======================================================================
// Colision
//=======================================================================
void handleColision(int level) {
	//cout << playerLastX << ' ' << playerLastZ << '\n';
	//cout << moveX << ' ' << moveZ << ' ' << moveUp << '\n';
	if (moveUp) {
		double xcomp = camera.center.x - camera.eye.x;
		double zcomp = camera.center.z - camera.eye.z;
		int nextX = playerLastX, nextZ = playerLastZ;
		if (moveX + playerSpeed * xcomp >= playerLastX + 0.4) {
			nextX = playerLastX + 1;
		}
		else if (moveX + playerSpeed * xcomp <= playerLastX - 0.4) {
			nextX = playerLastX - 1;
		}
		if (moveZ + playerSpeed * zcomp >= playerLastZ + 0.4) {
			nextZ = playerLastZ + 1;
		}
		else if (moveZ + playerSpeed * zcomp <= playerLastZ - 0.4) {
			nextZ = playerLastZ - 1;
		}
		if (level == 1 ? valid1(nextX + 5, nextZ + 5) : valid2(nextX + 7, nextZ + 7)) {
			moveX += playerSpeed * xcomp;
			moveZ += playerSpeed * zcomp;
			if (thirdPerson) {
				camera.eye.x += playerSpeed * xcomp;
				camera.eye.z += playerSpeed * zcomp;
			}
			else {
				camera.center.x += playerSpeed * xcomp;
				camera.center.z += playerSpeed * zcomp;
			}
			if (moveX >= playerLastX + 0.4) {
				playerLastX++;
			}
			if (moveZ >= playerLastZ + 0.4) {
				playerLastZ++;
			}
			if (moveX <= playerLastX - 0.4) {
				playerLastX--;
			}
			if (moveZ <= playerLastZ - 0.4) {
				playerLastZ--;
			}
		}
		//cout << xcomp << ' ' << zcomp << '\n';
	}
}

//=======================================================================
// Display Function
//=======================================================================
void RenderGround(double l, GLTexture tex)
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(1, 1, 1);

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing

	glBindTexture(GL_TEXTURE_2D, tex.texture[0]);	// Bind the ground texture

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-l, 0, -l);
	glTexCoord2f(0, 1);
	glVertex3f(-l, 0, l);
	glTexCoord2f(1, 1);
	glVertex3f(l, 0, l);
	glTexCoord2f(1, 0);
	glVertex3f(l, 0, -l);
	glEnd();
	glPopMatrix();

	glColor3f(1, 1, 1);
	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

}

void drawWall(int i, int j, GLTexture tex, int l) {
	glBindTexture(GL_TEXTURE_2D, tex.texture[0]);
	glPushMatrix();

	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0);
	glVertex3f(i - l - .5, 1, j - l - .5);
	glTexCoord2f(1, 0);
	glVertex3f(i - l + .5, 1, j - l - .5);
	glTexCoord2f(1, 1);
	glVertex3f(i - l + .5, 1, j - l + .5);
	glTexCoord2f(0, 1);
	glVertex3f(i - l - .5, 1, j - l + .5);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0);
	glVertex3f(i - l + .5, 0, j - l - .5);
	glTexCoord2f(1, 0);
	glVertex3f(i - l + .5, 1, j - l - .5);
	glTexCoord2f(1, 1);
	glVertex3f(i - l + .5, 1, j - l + .5);
	glTexCoord2f(0, 1);
	glVertex3f(i - l + .5, 0, j - l + .5);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0, 0);
	glVertex3f(i - l - .5, 0, j - l - .5);
	glTexCoord2f(1, 0);
	glVertex3f(i - l - .5, 1, j - l - .5);
	glTexCoord2f(1, 1);
	glVertex3f(i - l - .5, 1, j - l + .5);
	glTexCoord2f(0, 1);
	glVertex3f(i - l - .5, 0, j - l + .5);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0, 0);
	glVertex3f(i - l - .5, 0, j - l + .5);
	glTexCoord2f(1, 0);
	glVertex3f(i - l + .5, 0, j - l + .5);
	glTexCoord2f(1, 1);
	glVertex3f(i - l + .5, 1, j - l + .5);
	glTexCoord2f(0, 1);
	glVertex3f(i - l - .5, 1, j - l + .5);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3f(0, 0, -1);
	glTexCoord2f(0, 0);
	glVertex3f(i - l - .5, 0, j - l - .5);
	glTexCoord2f(1, 0);
	glVertex3f(i - l + .5, 0, j - l - .5);
	glTexCoord2f(1, 1);
	glVertex3f(i - l + .5, 1, j - l - .5);
	glTexCoord2f(0, 1);
	glVertex3f(i - l - .5, 1, j - l - .5);
	glEnd();

	glPopMatrix();
}

void drawKey(int i, int j) {
	glPushMatrix();
	glTranslated(i, 0.5, j);
	glColor3f(1, 1, 1);
	glRotated(90, 1, 0, 1);
	glScaled(0.01, 0.01, 0.01);
	model_key.Draw();
	glPopMatrix();
}

void drawPower(int i,int j) {
	glPushMatrix();
	glTranslated(i, 0.5, j);
	//glColor3f(1, 1, 1);
	//glRotated(90, 1, 0, 1);
	glScaled(0.001, 0.001, 0.001);
	model_power.Draw();
	glPopMatrix();
}

void drawWallsAndKeys(vector<vector<int>> maze, GLTexture tex) {
	//glDisable(GL_LIGHTING);
	glColor3f(1, 1, 1);
	for (int i = 0; i < maze.size(); i++) {
		for (int j = 0; j < maze[i].size(); j++) {
			if (maze[i][j] == 1) {
				drawWall(i, j, tex, maze.size() / 2);
			}
			if (maze[i][j] == 2) {
				drawKey(i - 5, j - 5);
			}
			if (maze[i][j] == 5) {
				drawPower(i - 7, j - 7);
			}
		}
	}
	//glEnable(GL_LIGHTING);
}

void handleKeyboard(unsigned char k, int x, int y) {
	if (k == 'w' || k == 'W')
		moveUp = true;
	if (k == '3')
		thirdPerson = !thirdPerson;
	if(k == 'f' || k == 'F')
		exit(0);
	glutPostRedisplay();
}

void handleKeyboardUp(unsigned char k, int x, int y) {
	if (k == 'w' || k == 'W')
		moveUp = false;

	glutPostRedisplay();
}

void beginLevel2() {
	level = 2;
	t = 0;
	moveX = -7;
	moveZ = 0;
	playerLastX = -7;
	playerLastZ = 0;
	angle = -90;
	passLevel1 = false;
}

void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (!win && lose) {
		if (!afsha) {
			sndPlaySound(TEXT("zapsplat_horror_monster_throw_hit_impact_against_wall_002_88763.wav"), SND_ASYNC);
			afsha = true;
		}
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		double w = glutGet(GLUT_WINDOW_WIDTH);
		double h = glutGet(GLUT_WINDOW_HEIGHT);
		glOrtho(0, w, 0, h, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glColor3f(1, 1, 1);
		glRasterPos3f(520, 360, 0);
		string s = "**** GAME OVER ****";
		for (int i = 0; i < s.size(); i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
		}
	}
	else {

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		double w = glutGet(GLUT_WINDOW_WIDTH);
		double h = glutGet(GLUT_WINDOW_HEIGHT);
		glOrtho(0, w, 0, h, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (level == 1) {
			glColor3f(0, 0, 0);
			glRasterPos3f(300, 700, 0);
			stringstream ss;
			string s = "keys: ";
			string tmp;
			ss << keys;
			ss >> tmp;
			s.append(tmp);
			s.append("/3");
			for (int i = 0; i < s.size(); i++) {
				glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
			}
			//cout << t << '\n';
			glRasterPos3f(900, 700, 0);
			int seconds = 60 - t / 100;
			string s2 = "time left: ";
			stringstream ss2;
			string tmp2;
			ss2 << seconds;
			ss2 >> tmp2;
			//cout << seconds << '\n';
			s2.append(tmp2);
			s2.append("sec");
			for (int i = 0; i < s2.size(); i++) {
				glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s2[i]);
			}
		}
		else if (!win && level == 2) {
			glColor3f(1, 1, 1);
			glRasterPos3f(400, 700, 0);
			string s3 = "Run for your life, ENEMY IS COMING";
			for (int i = 0; i < s3.size(); i++) {
				glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s3[i]);
			}
		}

		setupCamera();

		if (passLevel1)
			beginLevel2();

		if (level == 1) {

			GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
			GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
			glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
			glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);
			if (keys == 3)
				maze1[10][5] = 0;
			if (maze1[10][5] == 0 && playerLastX == 5 && playerLastZ == 0)
				passLevel1 = true;
			if (maze1[playerLastX + 5][playerLastZ + 5] == 2) {
				maze1[playerLastX + 5][playerLastZ + 5] = 0;
				keys++;
				sndPlaySound(TEXT("zapsplat_multimedia_game_sound_coins_collect_several_at_once_001_40812.wav"), SND_ASYNC);
			}
			//Ground
			RenderGround(5.5, tex_grass);

			// night sky
			glPushMatrix();
			GLUquadricObj* qobj;
			qobj = gluNewQuadric();
			glColor3f(1, 1, 1);
			glBindTexture(GL_TEXTURE_2D, sky);
			gluQuadricTexture(qobj, true);
			gluQuadricNormals(qobj, GL_SMOOTH);
			gluSphere(qobj, 60, 100, 100);
			gluDeleteQuadric(qobj);
			glPopMatrix();

			// walls 
			drawWallsAndKeys(maze1, tex_bushes);

			glDisable(GL_TEXTURE_2D);

			// player
			glPushMatrix();
			glTranslated(moveX, 0, moveZ);
			glColor3f(1, 1, 1);
			glRotated(angle, 0, 1, 0);
			glScaled(0.5, 0.5, 0.5);
			model_man.Draw();
			glPopMatrix();

			// gate
			glPushMatrix();
			glBindTexture(GL_TEXTURE_2D, tex_gate.texture[0]);
			glBegin(GL_QUADS);
			glNormal3f(-1, 0, 0);
			glTexCoord2f(0, 0);
			glVertex3f(4.5, 0, -0.5);
			glTexCoord2f(1, 0);
			glVertex3f(4.5, 1, -0.5);
			glTexCoord2f(1, 1);
			glVertex3f(4.5, 1, 0.5);
			glTexCoord2f(0, 1);
			glVertex3f(4.5, 0, 0.5);
			glEnd();
			glPopMatrix();
		}
		else if (level == 2) {
			if (win && !lose) {
				glMatrixMode(GL_PROJECTION);
				glLoadIdentity();
				double w = glutGet(GLUT_WINDOW_WIDTH);
				double h = glutGet(GLUT_WINDOW_HEIGHT);
				glOrtho(0, w, 0, h, -1, 1);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();

				glColor3f(1, 1, 1);
				glRasterPos3f(520, 360, 0);
				string s = "**** VICTORY ****";
				for (int i = 0; i < s.size(); i++) {
					glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, s[i]);
				}
			}
			else {
				if (maze2[playerLastX + 7][playerLastZ + 7] == 5) {
					maze2[playerLastX + 7][playerLastZ + 7] = 0;
					powerUpTime = 200;
					sndPlaySound(TEXT("goal.wav"), SND_ASYNC);
				}
				GLfloat Ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
				GLfloat Position[] = { camera.eye.x, camera.eye.y, camera.eye.z, 1 };
				GLfloat Position2[] = { camera.center.x - 0.5 * camera.fromEyeToCenter.x, camera.center.y - 0.5 * camera.fromEyeToCenter.y, camera.center.z - 0.5 * camera.fromEyeToCenter.z, 1 };
				GLfloat Direction[] = { camera.fromEyeToCenter.x, camera.fromEyeToCenter.y, camera.fromEyeToCenter.z };

				glLightfv(GL_LIGHT0, GL_AMBIENT, Ambient);
				glLightfv(GL_LIGHT0, GL_POSITION, thirdPerson ? Position2 : Position);
				glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 20.0);
				glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, Direction);


				// green light
				GLfloat AmbientR[] = { 0.0f, 1.0f, 0.0f, 1.0f };
				GLfloat PositionR[] = { 7, 2, 0, 1 };
				GLfloat DirectionR[] = { 0, -1, 0 };

				glEnable(GL_LIGHT1);
				glLightfv(GL_LIGHT1, GL_AMBIENT, AmbientR);
				glLightfv(GL_LIGHT1, GL_POSITION, PositionR);
				glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 20.0);
				glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, DirectionR);


				//Ground
				RenderGround(7.5, tex_sand);

				// night sky
				glPushMatrix();
				GLUquadricObj* qobj;
				qobj = gluNewQuadric();
				glColor3f(1, 1, 1);
				glBindTexture(GL_TEXTURE_2D, night);
				gluQuadricTexture(qobj, true);
				gluQuadricNormals(qobj, GL_SMOOTH);
				gluSphere(qobj, 60, 100, 100);
				gluDeleteQuadric(qobj);
				glPopMatrix();

				// walls 
				drawWallsAndKeys(maze2, tex_wall);

				glDisable(GL_TEXTURE_2D);

				// player
				glPushMatrix();
				glTranslated(moveX, 0, moveZ);
				glColor3f(1, 1, 1);
				glRotated(angle, 0, 1, 0);
				glScaled(0.5, 0.5, 0.5);
				model_man.Draw();
				glPopMatrix();

				// enemy
				if (t > 200) {
					glPushMatrix();
					glTranslated(enemyX, 0, enemyZ);
					glColor3f(1, 1, 1);
					glRotated(enemyAngle, 0, 1, 0);
					glScaled(0.5, 0.5, 0.5);
					model_enemy.Draw();
					glPopMatrix();
				}
			}
		}
	}

	glutSwapBuffers();
}

void Mouse(int x, int y) {
	SetCursorPos(100 + 1280 / 2, 100 + 720 / 2);
	int dx = x - (1280 / 2);
	int dy = y - (720 / 2);
	prev_x = x;
	prev_y = y;
	float a = 0.15;
	//cout << dx << '\n';
	angle += -a * dx;

	camera.rotateX(-a * dy);
	camera.rotateY(-a * dx);
	glutPostRedisplay();
}

void Timer(int x) {
	if (playerLastX == 7 && playerLastZ == 0)
		win = true;
	if ((level == 1 && t > 6000) || (level == 2 && t > 200 && enemyLastX == playerLastX && enemyLastZ == playerLastZ))
		lose = true;
	if (powerUpTime <= 0) {
		playerSpeed = 0.04;
	}
	else {
		playerSpeed = 0.06;
	}
	handleColision(level);
	if (t > 200 && level == 2)
		track();
	t += 1;
	powerUpTime--;
	glutPostRedisplay();
	glutTimerFunc(10, Timer, 0);
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_enemy.Load("Models/enemy/Skelet.3DS");
	model_man.Load("Models/man/Man.3DS");
	model_key.Load("Models/key/key3.3DS");
	model_power.Load("Models/Pegas/Pegas-sculp.3DS");

	// Loading texture files
	tex_sand.Load("Textures/sand.bmp");
	tex_wall.Load("Textures/stone.bmp");
	tex_grass.Load("Textures/grass.bmp");
	tex_bushes.Load("Textures/bushes.bmp");

	loadBMP(&sky, "Textures/sky.bmp", true);
	loadBMP(&night, "Textures/night.bmp", true);
	tex_gate.Load("Textures/door.bmp");
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 100);

	glutCreateWindow(title);
	SetCursorPos(100 + 1280 / 2, 100 + 720 / 2);
	ShowCursor(false);
	playerSpeed = 0.04;

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(handleKeyboard);
	glutKeyboardUpFunc(handleKeyboardUp);

	glutPassiveMotionFunc(Mouse);

	glutTimerFunc(0, Timer, 0);
	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}