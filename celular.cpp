#define GLM_ENABLE_EXPERIMENTAL

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
int polygon_mode = GL_FILL;

int mode = 1;

bool type = false;

int z = 0;

double queuedMilliseconds = 0;
int indexI = 0, indexJ = 0;
int indexX = 0, indexY = 0;

clock_t startTime = clock();

float lightR = 1.0f;
float lightG = 1.0f;
float lightB = 1.0f;

int win_height = 1024, win_width = 768;

float rotationAngleX = 0, rotationAngleY = 0, rotationAngleZ = 180;
float translationX, translationY, translationZ = 0.0f;
float scaleX = 1.0f;
float scaleY = 1.0f;
float scaleZ = 1.0f;

/** Vertex shader. */
const char *vertex_code = "\n"
                          "#version 460 core\n"
                          "layout (location = 0) in vec3 position;\n"
                          "layout (location = 1) in vec3 color;\n"
                          "\n"
                          "uniform mat4 transform;\n"
                          "uniform mat4 view;\n"
                          "uniform mat4 projection;\n"
                          "\n"
                          "out vec3 vColor;\n"
                          "void main()\n"
                          "{\n"
                          "gl_Position = transform * view * projection * vec4(position, 1.0);\n"
                          "vColor = color;\n"
                          "}\0";

/** Fragment shader. */
const char *fragment_code = "\n"
                            "#version 460 core\n"
                            "in vec3 vColor;\n"
                            "out vec4 FragColor;\n"
                            "uniform vec3 lightColor;\n"
                            "\n"
                            "void main()\n"
                            "{\n"
                            "    FragColor = vec4(vColor*lightColor, 1.0f);\n"
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

Particle world[256][256];
float vertices[256 * 256 * 36];

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
    if (world[x - 1][y].typeId == 0)
    {
        world[x - 1][y] = world[x][y];
        emptify(x, y);
    }
    else if (world[x - 1][y - 1].typeId == 0)
    {
        world[x - 1][y - 1] = world[x][y];
        emptify(x, y);
    }
    else if (world[x - 1][y + 1].typeId == 0)
    {
        world[x + 1][y + 1] = world[x][y];
        emptify(x, y);
    }
    if (world[x - 1][y].typeId == 2)
    {
        Particle aux;
        aux = world[x - 1][y];
        world[x - 1][y] = world[x][y];
        world[x][y] = aux;
    }
}

void update_water(int x, int y)
{
    if (world[x - 1][y].typeId == 0)
    {
        world[x - 1][y] = world[x][y];
        emptify(x, y);
    }
    else if (world[x - 1][y - 1].typeId == 0)
    {
        world[x - 1][y - 1] = world[x][y];
        emptify(x, y);
    }
    else if (world[x - 1][y + 1].typeId == 0)
    {
        world[x - 1][y + 1] = world[x][y];
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

    for (x = 0; x < 256; ++x)
    {
        for (y = 0; y < 256; ++y)
        {
            emptify(x, y);
            printf("typeId %d, %d: %d\n", x, y, world[x][y].typeId);
        }
    }

    // Vertex array.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Vertex buffer
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);

    // Set attributes.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

    // Desabilita VAO
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Unbind Vertex Array Object.
    glBindVertexArray(0);
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
    case 'e':
        mode = 3;
        break;
    case 'E':
        mode = 3;
        break;
    case 'i':
        rotationAngleY = ((rotationAngleY + 1.0f) < 360.0f) ? rotationAngleY + 1.0f : 360.0 - rotationAngleY + 1.0f;
        break;
    case 'I':
        rotationAngleY = ((rotationAngleY + 1.0f) < 360.0f) ? rotationAngleY + 1.0f : 360.0 - rotationAngleY + 1.0f;
        break;
    case 'k':
        rotationAngleY = ((rotationAngleY - 1.0f) < 0) ? rotationAngleY + 360.0f - 1.0f : rotationAngleY - 1.0f;
        break;
    case 'K':
        rotationAngleY = ((rotationAngleY - 1.0f) < 0) ? rotationAngleY + 360.0f - 1.0f : rotationAngleY - 1.0f;
        break;
    case 'l':
        rotationAngleX = ((rotationAngleX + 1.0f) < 360.0f) ? rotationAngleX + 1.0f : 360.0 - rotationAngleX + 1.0f;
        break;
    case 'L':
        rotationAngleX = ((rotationAngleX + 1.0f) < 360.0f) ? rotationAngleX + 1.0f : 360.0 - rotationAngleX + 1.0f;
        break;
    case 'j':
        rotationAngleX = ((rotationAngleX - 1.0f) < 0) ? rotationAngleX + 360.0f - 1.0f : rotationAngleX - 1.0f;
        break;
    case 'J':
        rotationAngleX = ((rotationAngleX - 1.0f) < 0) ? rotationAngleX + 360.0f - 1.0f : rotationAngleX - 1.0f;
        break;
    case 'p':
        rotationAngleZ = ((rotationAngleZ + 1.0f) < 360.0f) ? rotationAngleZ + 1.0f : 360.0 - rotationAngleZ + 1.0f;
        break;
    case 'P':
        rotationAngleZ = ((rotationAngleZ + 1.0f) < 360.0f) ? rotationAngleZ + 1.0f : 360.0 - rotationAngleZ + 1.0f;
        break;
    case 'n':
        rotationAngleZ = ((rotationAngleZ - 1.0f) < 0) ? rotationAngleZ + 360.0f - 1.0f : rotationAngleZ - 1.0f;
        break;
    case 'N':
        rotationAngleZ = ((rotationAngleZ - 1.0f) < 0) ? rotationAngleZ + 360.0f - 1.0f : rotationAngleZ - 1.0f;
        break;
    case '.':
        if (type)
        {
            lightR = 1.0f;
            lightG = 1.0f;
            lightB = 1.0f;
            type = !type;
        }
        else
        {
            lightR = .5f;
            lightG = .5f;
            lightB = .5f;
            type = !type;
        }
        break;
    case 27:
        exit(0);
    }
    glutPostRedisplay();
}

void updateFrame()
{
    clock_t timeNow = clock();
    queuedMilliseconds += double(timeNow - startTime) / 1000.0;

    startTime = timeNow;

    while (queuedMilliseconds >= 60)
    {
        z = 0;

        for (indexX = 1; indexX < 257; ++indexX)
        {
            for (indexY = 1; indexY < 257; ++indexY)
            {
                int id = world[indexX][indexY].typeId;
                switch (id)
                {
                case 0:
                    //nothing
                    break;
                case 1:
                    //do sand
                    update_sand(indexX, indexY);
                    break;
                case 2:
                    //do water
                    update_water(indexX, indexY);
                    break;
                case 3:
                    //wood
                    //do nothing
                    break;
                }
            }
        }

        for (indexI = 0; indexI < 256; indexI++)
        {
            for (indexJ = 0; indexJ < 256; indexJ++)
            {
                if (world[indexI][indexJ].typeId != 0)
                {
                    //primeiro triangulo
                    vertices[z] = mapRow2Y(indexI, 256 + 1);
                    vertices[z + 1] = mapColumn2X(indexJ + 1, 256 + 1);
                    vertices[z + 2] = 0;
                    vertices[z + 3] = world[indexI][indexJ].color.r;
                    vertices[z + 4] = world[indexI][indexJ].color.g;
                    vertices[z + 5] = world[indexI][indexJ].color.b;
                    vertices[z + 6] = mapRow2Y(indexI, 256 + 1);
                    vertices[z + 7] = mapColumn2X(indexJ, 256 + 1);
                    vertices[z + 8] = 0;
                    vertices[z + 9] = world[indexI][indexJ].color.r;
                    vertices[z + 10] = world[indexI][indexJ].color.g;
                    vertices[z + 11] = world[indexI][indexJ].color.b;
                    vertices[z + 12] = mapRow2Y(indexI + 1, 256 + 1);
                    vertices[z + 13] = mapColumn2X(indexJ, 256 + 1);
                    vertices[z + 14] = 0;
                    vertices[z + 15] = world[indexI][indexJ].color.r;
                    vertices[z + 16] = world[indexI][indexJ].color.g;
                    vertices[z + 17] = world[indexI][indexJ].color.b;
                    //segundo triangulo

                    vertices[z + 18] = mapRow2Y(indexI + 1, 256 + 1);
                    vertices[z + 19] = mapColumn2X(indexJ, 256 + 1);
                    vertices[z + 20] = 0;
                    vertices[z + 21] = world[indexI][indexJ].color.r;
                    vertices[z + 22] = world[indexI][indexJ].color.g;
                    vertices[z + 23] = world[indexI][indexJ].color.b;
                    vertices[z + 24] = mapRow2Y(indexI + 1, 256 + 1);
                    vertices[z + 25] = mapColumn2X(indexJ + 1, 256 + 1);
                    vertices[z + 26] = 0;
                    vertices[z + 27] = world[indexI][indexJ].color.r;
                    vertices[z + 28] = world[indexI][indexJ].color.g;
                    vertices[z + 29] = world[indexI][indexJ].color.b;
                    vertices[z + 30] = mapRow2Y(indexI, 256 + 1);
                    vertices[z + 31] = mapColumn2X(indexJ + 1, 256 + 1);
                    vertices[z + 32] = 0;
                    vertices[z + 33] = world[indexI][indexJ].color.r;
                    vertices[z + 34] = world[indexI][indexJ].color.g;
                    vertices[z + 35] = world[indexI][indexJ].color.b;
                    z += 36;
                }
            }
        }

        // Vertex array.
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Vertex buffer
        // glGenBuffers(1, &VBO);
        // glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Set attributes.
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));

        // Desabilita VAO
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        // Unbind Vertex Array Object.
        glBindVertexArray(0);
        glPointSize(5);
        glutPostRedisplay();
        // doUpdate();
        queuedMilliseconds -= 60;
        z = 0;
    }
}

void mouseMove(int x, int y)
{
    if ((x < 1024 && x > 0) && (y < 768 && y > 0))
    {
        printf("x: %d  y: %d\n", x / 4, y / 3);
        switch (mode)
        {
        case 1: //sand
            if (world[x / 4][y / 3].typeId == 0)
            {
                world[x / 4][y / 3].color.r = (76 / 255.0f);
                world[x / 4][y / 3].color.g = (70 / 255.0f);
                world[x / 4][y / 3].color.b = (50 / 255.0f);
                world[x / 4][y / 3].hasUpdated = false;
                world[x / 4][y / 3].lifetime = 0;
                world[x / 4][y / 3].typeId = 1;
            }
            else
            {
                printf("COISADO\n");
            }
            break;
        case 2: //water
            if (world[x / 4][y / 3].typeId == 0)
            {
                world[x / 4][y / 3].color.r = (2 / 255.0f);
                world[x / 4][y / 3].color.g = (68 / 255.0f);
                world[x / 4][y / 3].color.b = (89 / 255.0f);
                world[x / 4][y / 3].hasUpdated = false;
                world[x / 4][y / 3].lifetime = 0;
                world[x / 4][y / 3].typeId = 2;
            }
            else
            {
                printf("COISADO\n");
            }
            break;

        case 3: //wood
            if (world[x / 4][y / 3].typeId == 0)
            {
                world[x / 4][y / 3].color.r = (93 / 255.0f);
                world[x / 4][y / 3].color.g = (67 / 255.0f);
                world[x / 4][y / 3].color.b = (44 / 255.0f);
                world[x / 4][y / 3].hasUpdated = false;
                world[x / 4][y / 3].lifetime = 0;
                world[x / 4][y / 3].typeId = 3;
            }
            else
            {
                printf("COISADO\n");
            }
            break;
        }

        printf("%f\n", mapColumn2X(x, 1024));
        printf("%f\n", mapRow2Y(y, 768));
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
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (win_width / (float)win_height), 0.1f, 100.0f);
    // Retrieve location of tranform variable in shader.
    loc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(M));

    loc = glGetUniformLocation(program, "lightColor");
    glUniform3f(loc, lightR, lightG, lightB);

    // Draws the triangle.
    glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);

    glDrawArrays(type_primitive, 0, 256 * 256 * 36);
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
    glutInitWindowSize(1024, 768);
    glutCreateWindow("Celular Automata");
    glewInit();

    // Init vertex data for the triangle.
    setup();
    // doUpdate();

    // Create shaders.
    initShaders();

    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);
    glutMotionFunc(mouseMove);
    glutIdleFunc(updateFrame);
    glutMainLoop();
}
