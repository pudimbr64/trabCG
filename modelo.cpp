#define GLM_ENABLE_EXPERIMENTAL

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cgTypes.h"
#include "cgImage.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <sys/time.h>

/** Program variable. */
int program;
/** Vertex array object. */
unsigned int VAO;
/** Vertex buffer object. */
unsigned int VBO;

int type_primitive = GL_TRIANGLES;
int polygon_mode = GL_POINT;

int mode = 1;

bool type = true;

int z = 0;

double queuedMilliseconds = 0;

int win_height, win_width = 600;

float rotationAngleX, rotationAngleY, rotationAngleZ = 0.0f;
float translationX, translationY, translationZ = 0.0f;
float scaleX = 0.95f;
float scaleY = 0.95f;
float scaleZ = 0.95f;

/** Vertex shader. */
const char *vertex_code = "\n"
                          "#version 460 core\n"
                          "layout (location = 0) in vec3 position;\n"
                          "\n"
                          "uniform mat4 transform;\n"
                          "uniform mat4 view;\n"
                          "uniform mat4 projection;\n"
                          "\n"
                          "void main()\n"
                          "{\n"
                          "gl_Position = transform * view * projection * vec4(position, 1.0);\n"
                          "}\0";

/** Fragment shader. */
const char *fragment_code = "\n"
                            "#version 460 core\n"
                            "out vec4 FragColor;\n"
                            "\n"
                            "void main()\n"
                            "{\n"
                            "    FragColor = vec4(0.2f, 0.3f, 0.2f, 1.0f);\n"
                            "}\0";

float mapRow2Y(int r, int h)
{
    return (((h - 1.0f - r) / (h - 1.0f)) * 2.0f - 1.0f);
}

float mapColumn2X(int c, int w)
{
    return ((c / (w - 1.0f)) * 2.0f - 1.0f);
}

typedef struct colorStruct
{
    float r;
    float g;
    float b;

} colorStruct;

typedef struct Particle
{
    int typeId;
    float lifetime;
    bool hasUpdated;
    colorStruct color;

} Particle;

Particle world[600][600];
float vertices[600 * 600];

void emptify(int x, int y)
{
    world[x][y].typeId = 0;
    world[x][y].lifetime = 0;
    world[x][y].hasUpdated = true;
    world[x][y].color.r = 0;
    world[x][y].color.g = 0;
    world[x][y].color.b = 0;
}

void update_sand(int x, int y)
{
    if (world[x][y + 1].typeId == 0)
    {
        world[x][y + 1] = world[x][y];
        emptify(x, y);
    }
    else if (world[x - 1, y + 1]->typeId == 0)
    {
        world[x - 1][y + 1] = world[x][y];
        emptify(x, y);
    }
    else if (world[x + 1][y + 1].typeId == 0)
    {
        world[x + 1][y + 1] = world[x][y];
        emptify(x, y);
    }
}

void update_water(int x, int y)
{
    if (world[x + 1][y].typeId == 0)
    {
        world[x + 1][y] = world[x][y];
        emptify(x, y);
    }
    else if (world[x + 1][y - 1].typeId == 0)
    {
        world[x + 1][y - 1] = world[x][y];
        emptify(x, y);
    }
    else if (world[x + 1][y + 1].typeId == 0)
    {
        world[x + 1][y + 1] = world[x][y];
        emptify(x, y);
    }
    else if (world[x][y - 1].typeId == 0)
    {
        world[x][y - 1] = world[x][y];
        emptify(x, y);
    }
    else if (world[x][y + 1].typeId == 0)
    {
        world[x][y + 1] = world[x][y];
        emptify(x, y);
    }
}

void setup()
{
    int x, y;

    for (y = 599; y > 0; --y)
    {
        for (y = 0; x < 600; ++x)
        {
            int id = world[x][y].typeId;
            switch (id)
            {
            case 0:
                //nothing
                break;
            case 1:
                //do sand
                break;
            case 2:
                //do water
                break;
            }
        }
    }

    for (int i = 0; i < 600; i++)
    {
        for (int j = 0; j < 600; j++)
        {
            vertices[z] = mapRow2Y(i, j);
            z++;
        }
    }

    // Vertex array.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Vertex buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set attributes.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // Desabilita VAO
    glEnableVertexAttribArray(0);

    // Unbind Vertex Array Object.
    glBindVertexArray(0);
    glPointSize(3);
}

void initShaders()
{
    // Request a program and shader slots from GPU
    program = glCreateProgram();
    int vertex = glCreateShader(GL_VERTEX_SHADER);
    int fragment = glCreateShader(GL_FRAGMENT_SHADER);

    // Set shaders source
    glShaderSource(vertex, 1, &vertex_code, NULL);
    glShaderSource(fragment, 1, &fragment_code, NULL);

    // Compile shaders
    glCompileShader(vertex);
    glCompileShader(fragment);

    // Attach shader objects to the program
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);

    // Link the program
    glLinkProgram(program);

    // Get rid of shaders (not needed anymore)
    glDetachShader(program, vertex);
    glDetachShader(program, fragment);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Set the program to be used.
    glUseProgram(program);
}

/** 
 * Keyboard function.
 *
 * Called to treat pressed keys.
 *
 * @param key Pressed key.
 */

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 's':
        mode = 1;
        break;
    case 'S':
        mode = 1;
        break;
    case 'w':
        mode = 2;
        break;
    case 'W':
        mode = 2;
        break;
    }
    glutPostRedisplay();
}

void updateFrame()
{



    // // double timeDifference = startTime - timeNow;
    // queuedMilliseconds += timeDifference;
    // startTime = timeNow;
    // while (queuedMilliseconds >= 10)
    // {
    //     setup();
    //     queuedMilliseconds -= 10;
    // }
}

void mouseMove(int x, int y)
{
    if ((x < 600 && x > 0) && (y < 600 && y > 0))
    {
        switch (mode)
        {
        case 1: //sand
            if (world[x][y].typeId == 0)
            {
                world[x][y].color.r = 76;
                world[x][y].color.g = 70;
                world[x][y].color.b = 50;
                world[x][y].hasUpdated = false;
                world[x][y].lifetime = 0;
                world[x][y].typeId = 1;
            }
            else
            {
                printf("COISADO\n");
            }
            break;
        case 2: //water
            if (world[x][y].typeId == 0)
            {
                world[x][y].color.r = 2;
                world[x][y].color.g = 68;
                world[x][y].color.b = 89;
                world[x][y].hasUpdated = false;
                world[x][y].lifetime = 0;
                world[x][y].typeId = 2;
            }
            else
            {
                printf("COISADO\n");
            }
            break;
        }

        printf("%f\n", mapColumn2X(x, 600));
        printf("%f\n", mapRow2Y(y, 600));
        printf("NEXT\n");
    }
}

void display()
{
    /* Set RGBA color to "paint" cleared color buffer (background color). */
    glClearColor(0, 0, 0, 1.0);

    /* Clears color buffer to the RGBA defined values. */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    glBindVertexArray(VAO);

    // Translation.
    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(translationX, translationY, translationZ));
    // Rotation around z-axis.
    glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngleZ), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(rotationAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
    // Scale.
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(scaleX, scaleY, scaleZ));

    glm::mat4 M = glm::mat4(1.0f);
    M = T * Rz * Rx * Ry * S;

    // Retrieve location of tranform variable in shader.
    unsigned int loc = glGetUniformLocation(program, "transform");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(M));

    // Define view matrix.
    // glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    // Retrieve location of tranform variable in shader.
    loc = glGetUniformLocation(program, "view");
    // Send matrix to shader.
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(M));

    // Define projection matrix.
    // glm::mat4 projection = glm::ortho(glm::radians(0.0f), (win_width / (float)win_height), 0.1f, 100.0f);
    // Retrieve location of tranform variable in shader.
    loc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(M));

    // Draws the triangle.
    glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);

    glDrawArrays(type_primitive, 0, z / 3);
    glutSwapBuffers();
}
void reshape(int width, int height)
{
    win_width = width;
    win_height = height;
    glViewport(0, 0, width, height);
    glutPostRedisplay();
}

int main(int argc, char **argv)
{

    // cgMat2i imagem = cgReadPGMImage(argv[1]);

    glutInit(&argc, argv);
    glutInitContextVersion(4, 6);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Modelo");
    glewInit();

    // Init vertex data for the triangle.
    // setup();

    // Create shaders.
    initShaders();

    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMotionFunc(mouseMove);
    glutIdleFunc(updateFrame);
    glutMainLoop();
}
