#include "BOX.h"
#include "auxiliar.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 
#include <IGL/IGlib.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>



//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Variables cámara
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.3f, 8.0f);
glm::vec3 targetPosition = glm::vec3(0.0f, 0.0f, -30.0f);
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

//Matrices
glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 model = glm::mat4(1.0f);

//Variables globales para la posición e intensidad de la luz
glm::vec3 lpos = glm::vec3(0.0f, 0.0f, 0.0f); // Posición inicial
glm::vec3 Id = glm::vec3(0.0f, 0.0f, 0.0f); // Intensidad inicial
GLuint lposLoc; // Identificador para la posición de la luz
GLuint IdLoc; // Identificador para la intensidad de la luz

//// Variables para la c�mara
//glm::mat4 viewC = glm::mat4(1.0f); // Matriz de vista inicial
//float alpha = 0.10f; // �ngulo de rotaci�n
//float inc = 0.1f;    // Incremento para zoom

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////

//Por definir
unsigned int vshader;
unsigned int fshader;
unsigned int program;


//Variables Uniform
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

int uColorTex;
int uEmiTex;


//VAO
unsigned int vao;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

//Texturas
unsigned int colorTexId;
unsigned int emiTexId;



//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname);
void initObj();
void destroy();


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)


	initContext(argc, argv);
	initOGL();
	//initShader("../shaders_P3/shader.v0.vert", "../shaders_P3/shader.v0.frag");
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
	initObj();

	glutMainLoop();


	destroy();

	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);// No tiene nada que ver con el contexto
	glutInitWindowPosition(0, 0);// 
	glutCreateWindow("Prácticas OGL");

	//Extensiones:
	GLenum is_ok = glewInit();
	if (GLEW_OK != is_ok)
	{
		std::cout << "Error: " << glewGetErrorString(is_ok) << std::endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	//Gestion de eventos:
	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);


}



void initOGL()
{
	glClearColor(1.0f, 0.2f, 0.2f, 0.0f);
	
	glEnable(GL_DEPTH_TEST);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);


	//Esto no tiene que ver con OpenGL
	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -6;

	view = glm::mat4(1.0f);
	view = glm::lookAt(cameraPosition, targetPosition, upVector);

}
void destroy()
{
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	glDeleteBuffers(1, &posVBO);
	glDeleteBuffers(1, &colorVBO);
	glDeleteBuffers(1, &normalVBO);
	glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);
	glDeleteVertexArrays(1, &vao);
}


void initShader(const char *vname, const char *fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}
	std::cout << "Todo OK" << std::endl;

	glUseProgram(program);

	lposLoc = glGetUniformLocation(program, "lpos");
	IdLoc = glGetUniformLocation(program, "Id");

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");

	if (uColorTex != -1)
	{
		glUniform1i(uColorTex, 0);
	}
	if (uEmiTex != -1)
	{
		glUniform1i(uEmiTex, 1);
	}

	//int borrar = glGetUniformLocation(program, "borrar");
	//int borrar = glGetAttribLocation(program, "inTexCoord");
	//int borrar = glGetAttribLocation(program, "inPos");
}


void initObj()
{
	/*
	glGenBuffers(1, &posVBO);
	glGenBuffers(1, &colorVBO);
	glGenBuffers(1, &normalVBO);
	glGenBuffers(1, &texCoordVBO);
	glGenBuffers(1, &triangleIndexVBO);
	*/
	unsigned int buff;
	glGenBuffers(1, &buff);
	
	glBindBuffer(GL_ARRAY_BUFFER, buff);
	glBufferData(GL_ARRAY_BUFFER, 
		(cubeNVertex * (3+3+3+2)) * sizeof(float)+ cubeNTriangleIndex * 3 * sizeof(unsigned int),
		NULL, GL_STATIC_DRAW); 

	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * cubeNVertex * sizeof(float), cubeVertexPos);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * cubeNVertex * sizeof(float), 3 * cubeNVertex * sizeof(float), cubeVertexColor);
	glBufferSubData(GL_ARRAY_BUFFER, (3+3) * cubeNVertex * sizeof(float), 3 * cubeNVertex * sizeof(float), cubeVertexNormal);
	glBufferSubData(GL_ARRAY_BUFFER, (3 + 3 + 3) * cubeNVertex * sizeof(float), 2 * cubeNVertex * sizeof(float), cubeVertexTexCoord);
	glBufferSubData(GL_ARRAY_BUFFER, 
		(3 + 3 + 3 + 2) * cubeNVertex * sizeof(float), 
		3 * cubeNTriangleIndex * sizeof(unsigned int), cubeTriangleIndex);
	
	
		//Conf Geom.
	glGenVertexArrays(1, &vao); 
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buff);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *) (3 * cubeNVertex * sizeof(float)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(6 * cubeNVertex * sizeof(float)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)(9 * cubeNVertex * sizeof(float)));
	glEnableVertexAttribArray(0); glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2); glEnableVertexAttribArray(3);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buff);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}


void initObj1()
{

	/*
	glGenBuffers(1, &posVBO);
	glGenBuffers(1, &colorVBO);
	glGenBuffers(1, &normalVBO);
	glGenBuffers(1, &texCoordVBO);
	glGenBuffers(1, &triangleIndexVBO);
	*/
	unsigned int buff[5];
	glGenBuffers(5, buff);
	posVBO = buff[0]; colorVBO = buff[1]; normalVBO = buff[2];
	texCoordVBO = buff[3]; triangleIndexVBO = buff[4];


	glBindBuffer(GL_ARRAY_BUFFER, posVBO);//activación como buffer de attrib.
	glBufferData(GL_ARRAY_BUFFER, // obj activo 
		cubeNVertex * 3 * sizeof(float), // tamaño en bytes
		cubeVertexPos, // puntero a los datos
		GL_STATIC_DRAW); //

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	//glBufferData(GL_ARRAY_BUFFER, cubeNVertex * 3 * sizeof(float), 
	//	cubeVertexColor, GL_STATIC_DRAW); 
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * 3 * sizeof(float),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, cubeNVertex * 3 * sizeof(float), cubeVertexColor);

	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * 3 * sizeof(float), cubeVertexNormal, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * 2 * sizeof(float), cubeVertexTexCoord, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNVertex * 2 * sizeof(float), cubeTriangleIndex, GL_STATIC_DRAW);


	//Conf Geom.
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, posVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);

	model = glm::mat4(1.0f);


}

GLuint loadShader(const char *fileName, GLenum type){ 

	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);
	
	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);

	glShaderSource(shader, 1,
		(const GLchar**)&source, (const GLint*)&fileLen);

	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compiló bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}

	return shader; 
}


unsigned int loadTex(const char *fileName){
	unsigned char* map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);
	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);


	glBindTexture(GL_TEXTURE_2D, texId);

//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
//		GL_UNSIGNED_BYTE, (GLvoid*)map);



	glTexStorage2D(GL_TEXTURE_2D, 4, GL_RGBA8, w, h);
	glTexSubImage2D(GL_TEXTURE_2D, 0,//Level
		0, 0, //offset
		w, h, // tamaño de los datos que subo
		GL_RGBA, GL_UNSIGNED_BYTE,
		(GLvoid*)map);

	delete[] map;

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	
	return texId;
}

void renderFunc()
{
	//Texturas
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);
	
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	
	
	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,	&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,	&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	////glActiveTexture(GL_TEXTURE0);
	////glBindTexture(GL_TEXTURE_2D, colorTexId);
	////glActiveTexture(GL_TEXTURE0 + 1);
	////glBindTexture(GL_TEXTURE_2D, emiTexId);

	//if (uColorTex != -1)
	//{
	//	glUniform1i(uColorTex, 0);
	//}
	//if (uEmiTex != -1)
	//{
	//	glUniform1i(uEmiTex, 1);
	//}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexId);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, emiTexId);


	glBindVertexArray(vao);
	
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3, GL_UNSIGNED_INT, (void*) (11 * sizeof(float)* cubeNVertex));




	glutSwapBuffers();
}


void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);


	//Creación de la matriz de projección que mantiene el aspecto de la ventana
	glm::mat4 projW = glm::mat4(1.0f);


	float a_ratio = float(width) / float(height); 
	float n = 1.0f;
	float f = 10.0f;

	projW[0][0] = 1.0f / glm::tan(3.141592f / 6.0f);
	projW[1][1] = proj[0][0] * a_ratio;
	projW[2][2] = (f + n) / (n - f);
	projW[2][3] = -1.f;
	projW[3][2] = 2.f * f * n / (n - f);

	proj = projW;

	glutPostRedisplay();

}

void idleFunc()
{
	model = glm::mat4(1.0f);
	static float angle = 0.0f;
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.001f;
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));

	glutPostRedisplay();

}

void keyboardFunc(unsigned char key, int x, int y)
{
		float move= 0.1f;
		float intensity = 0.1f;
		float rotation = 0.5f;

		switch (key) {

			
			case 'w':
				cameraPosition.z -= move;
				break;
			case 's':
				cameraPosition.z += move;
				break;
			case 'a':
				cameraPosition.x -= move;
				break;
			case 'd':
				cameraPosition.x += move;
				break;
			case 'e':
				targetPosition.x += rotation;
				break;
			case 'q':
				targetPosition.x -= rotation;
				break;
		

			//CAMBIOS DE LA LUZ
		case 'j': //Mover la luz a la izquierda
			lpos.x -= move;
			std::cout << "lpos: " << lpos.x << ", " << lpos.y << ", " << lpos.z << std::endl;
			break;
		case 'l': //Mover la luz a la derecha
			lpos.x += move;
			std::cout << "lpos: " << lpos.x << ", " << lpos.y << ", " << lpos.z << std::endl;
			break;
		case 'u': //Mover la luz arriba
			lpos.y += move;
			std::cout << "lpos: " << lpos.x << ", " << lpos.y << ", " << lpos.z << std::endl;
			break;
		case 'h': //Mover la luz abajo
			lpos.y -= move;
			std::cout << "lpos: " << lpos.x << ", " << lpos.y << ", " << lpos.z << std::endl;
			break;
		case 'k'://Mover la luz hacia atrás
			lpos.z -= move;
			std::cout << "lpos: " << lpos.x << ", " << lpos.y << ", " << lpos.z << std::endl;
			break;
		case 'i'://Mover la luz hacia delante
			lpos.z += move;
			std::cout << "lpos: " << lpos.x << ", " << lpos.y << ", " << lpos.z << std::endl;
			break;
		case '+': //Aumenta la intensidad
			Id += glm::vec3(intensity);
			std::cout << "Id: " << Id.x << ", " << Id.y << ", " << Id.z << std::endl;
			break;
		case '-': //Disminuye la intensidad
			Id -= glm::vec3(intensity);
			std::cout << "Id: " << Id.x << ", " << Id.y << ", " << Id.z << std::endl;
			break;
		}

		//IGlib::setViewMat(viewC);
		view = glm::lookAt(cameraPosition, targetPosition, upVector);
		// Actualizar los valores en el shader
		glUseProgram(program);
		glUniform3fv(lposLoc, 1, &lpos[0]);
		glUniform3fv(IdLoc, 1, &Id[0]);
		glUseProgram(0);




	
}






void mouseFunc(int button, int state, int x, int y){}









