#include "../Externals/Include/Include.h"
#include "../Externals/Include/assimp/Importer.hpp"  
#include "../Externals/Include/assimp/scene.h"      
#include "../Externals/Include/assimp/postprocess.h"

#include "../VC14/Model.h"
#include "../VC14/Texture.h"
#include "../VC14/Camera.h"

#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3

GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;
float perspect_rad = 0.0f;

using namespace glm;
using namespace std;

mat4 view;
mat4 projection;
mat4 model;

GLint um4p;
GLint um4mv;
GLint color;

Model objModel;
GLuint program;

const int WINDOW_WIDTH = 600, WINDOW_HEIGHT = 600;
GLfloat lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
GLfloat pressed_X = WINDOW_WIDTH / 2.0f, pressed_Y = WINDOW_HEIGHT / 2.0f; // record press coordinate
GLfloat deltaTime = 16.0f;
bool MouseLeftPressed = false;
bool firstMouseMove = true;
Camera camera(glm::vec3(0.0f, 1.0f, 3.0f));

char** loadShaderSource(const char* file)
{
    FILE* fp = fopen(file, "rb");
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *src = new char[sz + 1];
    fread(src, sizeof(char), sz, fp);
    src[sz] = '\0';
    char **srcp = new char*[1];
    srcp[0] = src;
    return srcp;
}

void freeShaderSource(char** srcp)
{
    delete[] srcp[0];
    delete[] srcp;
}

// define a simple data structure for storing texture image raw data
/*typedef struct _TextureData
{
    _TextureData(void) :
        width(0),
        height(0),
        data(0)
    {
    }

    int width;
    int height;
    unsigned char* data;
} TextureData;*/

// load a png image and return a TextureData structure with raw data
// not limited to png format. works with any image format that is RGBA-32bit
/*TextureData loadPNG(const char* const pngFilepath)
{
    TextureData texture;
    int components;

    // load the texture with stb image, force RGBA (4 components required)
    stbi_uc *data = stbi_load(pngFilepath, &texture.width, &texture.height, &components, 4);

    // is the image successfully loaded?
    if (data != NULL)
    {
        // copy the raw data
        size_t dataSize = texture.width * texture.height * 4 * sizeof(unsigned char);
        texture.data = new unsigned char[dataSize];
        memcpy(texture.data, data, dataSize);

        // mirror the image vertically to comply with OpenGL convention
        for (size_t i = 0; i < texture.width; ++i)
        {
            for (size_t j = 0; j < texture.height / 2; ++j)
            {
                for (size_t k = 0; k < 4; ++k)
                {
                    size_t coord1 = (j * texture.width + i) * 4 + k;
                    size_t coord2 = ((texture.height - j - 1) * texture.width + i) * 4 + k;
                    std::swap(texture.data[coord1], texture.data[coord2]);
                }
            }
        }

        // release the loaded image
        stbi_image_free(data);
    }

    return texture;
}*/

void LoadModel(const string &objFilePath) 
{
	if (!objModel.loadModel(objFilePath)) {
		printf("Fail to load model\n");
		return;
	}
}

void My_Init()
{
    glClearColor(0.0f, 0.6f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	program = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");
	string objFilePath = "./Arabic+City.obj";

	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	/*shaderLog(vertexShader);
	shaderLog(fragmentShader);*/

	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	glLinkProgram(program);
	um4p = glGetUniformLocation(program, "um4p");
	um4mv = glGetUniformLocation(program, "um4mv");

	glUseProgram(program);

	LoadModel(objFilePath);
}

void My_Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	model = mat4();

	glUseProgram(program);

	projection = glm::perspective(camera.mouse_zoom,
		(GLfloat)(WINDOW_WIDTH) / WINDOW_HEIGHT, 1.0f, 10000.0f);
	view = camera.getViewMatrix(); 

	glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view * model));
	glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(projection));

	objModel.Draw(program);

    glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);

	float viewportAspect = (float)width / (float)height;
	projection = perspective(radians(60.0f), viewportAspect, 0.1f, 1000.0f);
	view = lookAt(vec3(perspect_rad, 0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
}

void My_Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(timer_speed, My_Timer, val);
}

void My_Mouse(int button, int state, int x, int y)
{
	if (button == 3 || button == 4)
	{
		if (state == GLUT_UP) {
			printf("Scroll (pressed) %s At (%d, %d)\n", (button == 3)? "Up" : "Down", x, y);
		}
		else {
			printf("Scroll (released) %s At (%d, %d)\n", (button == 3) ? "Up" : "Down", x, y);
		}
	}
	else 
	{
		if (button == GLUT_LEFT_BUTTON && !MouseLeftPressed && state == GLUT_DOWN) {
			pressed_X = x;
			pressed_Y = y;
			MouseLeftPressed = true;
			printf("Left Mouse Button is pressed at (%d, %d)\n", x, y);
		}
		else if (button == GLUT_LEFT_BUTTON && MouseLeftPressed && state == GLUT_UP) {
			GLfloat xoffset = x - pressed_X;
			GLfloat yoffset = pressed_Y - y;

			camera.handleMouseHoldMove(xoffset, yoffset);
			MouseLeftPressed = false;
			printf("Left Mouse Button is released at (%d, %d)\n", x, y);
		}
	}
}

void My_Move(int x, int y) {
	printf("Mouse moving... starting at (%d, %d)\n", x, y);
	if (firstMouseMove)
	{
		lastX = x;
		lastY = y;
		firstMouseMove = false;
	}

	GLfloat xoffset = x - lastX;
	GLfloat yoffset = lastY - y;

	lastX = x;
	lastY = y;

	camera.handleMouseMove(xoffset, yoffset);
}

void My_Keyboard(unsigned char key, int x, int y)
{
	if (key == 'w') {
		camera.handleKeyPress(FORWARD, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 's') {
		camera.handleKeyPress(BACKWARD, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'a') {
		camera.handleKeyPress(LEFT, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'd') {
		camera.handleKeyPress(RIGHT, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'z') {
		camera.handleKeyPress(UP, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'x') {
		camera.handleKeyPress(DOWN, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
	else if (key == 'r') {
		camera.handleKeyPress(RESET, deltaTime);
		printf("Button %c is pressed at (%d, %d)\n", key, x, y);
	}
}

void My_SpecialKeys(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		perspect_rad += 1.0f;
		My_Reshape(600, 600);
		break;
	case GLUT_KEY_DOWN:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		perspect_rad -= 1.0f;
		My_Reshape(600, 600);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch(id)
	{
	case MENU_TIMER_START:
		if(!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
    // Change working directory to source code path
    chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("AS2_Framework"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif
    glPrintContextInfo();
	My_Init();

	// Create a menu and bind it to mouse right button.
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);

	glutMouseFunc(My_Mouse);
	glutMotionFunc(My_Move);
	glutPassiveMotionFunc(My_Move);

	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0); 

	// Enter main event loop.
	glutMainLoop();

	return 0;
}