#include <SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>
#include <vector>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define INITIAL_COLLIDING 1
#define SPEED 0.0f

/******************************************************************************
 * struct World
 *
 * Structure containing the entire application context. An alternative to using
 * many global variables, this structure will be passed into most functions in
 * this application that require access to the global state.
 ******************************************************************************/
struct World
{
  SDL_Window *window;                     // The main SDL window.
  SDL_Renderer *renderer;                 // Renderer for SDL window.
  SDL_GLContext glContext;                // SDL OpenGL context.

  glm::mat4 projectionMat;                // Unused.
  glm::mat4 viewMat;                      // Unused.

  GLuint shaderProgram;                   // Simple shader program
  GLuint vertexShader;                    // Handle to vertex shader.
  GLuint fragmentShader;                  // Handle to fragment shader.
  GLint modelMatUniform;                  // Position of in_Model in shader.

  std::vector<glm::vec3> objectPositions; // List of vertices describing shape.
  GLuint objectVao;                       // VAO of object.
  GLuint objectPositionVbo;               // VBO containing position.
  glm::mat4 objectModelMat;               // Matrix for Object's position.

  std::vector<glm::vec3> wallPositions;   // List of vertices describing wall.
  GLuint wallVao;                         // VAO of wall.
  GLuint wallPositionVbo;                 // VBO containing walls position.
};

/******************************************************************************
 * triangle_intersect
 *
 * A fast triangle-triangle intersect test based on the work by Tomas Moller.
 * This function returns true if the specified two triangles overlap or touch.
 ******************************************************************************/
bool triangle_intersect(glm::vec3 a1, glm::vec3 b1, glm::vec3 c1,
                        glm::vec3 a2, glm::vec3 b2, glm::vec3 c2);

/******************************************************************************
 * update
 *
 * Move objects around the scene and perform collision tests.
 ******************************************************************************/
void update(World *world)
{
  world->objectModelMat = glm::translate(world->objectModelMat,
    glm::vec3(SPEED, 0, 0));

  glm::vec3 a1 = world->objectPositions.at(0);
  glm::vec3 b1 = world->objectPositions.at(1);
  glm::vec3 c1 = world->objectPositions.at(2);

  // ?

  glm::vec3 a2 = world->wallPositions.at(0);
  glm::vec3 b2 = world->wallPositions.at(1);
  glm::vec3 c2 = world->wallPositions.at(2);

  if(triangle_intersect(a1, b1, c1, a2, b2, c2) == true)
  {
    SDL_SetWindowTitle(world->window, "Colliding!");
  }
  else
  {
    SDL_SetWindowTitle(world->window, "Not Colliding");
  }
}

/******************************************************************************
 * display
 *
 * If needed, update the graphics card with recent data and draw the scene.
 ******************************************************************************/
void display(World *world)
{
  glClearColor(100.0f / 255.0f,
               149.0f / 255.0f,
               237.0f / 255.0f,
               1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(world->shaderProgram);

  glUniformMatrix4fv(world->modelMatUniform, 1, GL_FALSE,
    glm::value_ptr(world->objectModelMat));

  glBindVertexArray(world->objectVao);
  glDrawArrays(GL_TRIANGLES, 0, world->objectPositions.size());

  glUniformMatrix4fv(world->modelMatUniform, 1, GL_FALSE,
    glm::value_ptr(glm::mat4(1.0f)));

  glBindVertexArray(world->wallVao);
  glDrawArrays(GL_TRIANGLES, 0, world->wallPositions.size());
}

/*
                                  /   \       
 _                        )      ((   ))     (
(@)                      /|\      ))_((     /|\
|-|                     / | \    (/\|/\)   / | \                      (@)
| | -------------------/--|-voV---\`|'/--Vov-|--\---------------------|-|
|-|                         '^`   (o o)  '^`                          | |
| |                               `\Y/'                               |-|
|-|                                                                   | |
| |                          Here be dragons!                         |-|
|-|                                                                   | |
| |                                                                   |-|
|_|___________________________________________________________________| |
(@)              l   /\ /         ( (       \ /\   l                `\|-|
                 l /   V           \ \       V   \ l                  (@)
                 l/                _) )_          \I
                                   `\ /'
                                     `
*/

void error(std::string message)
{
  std::cout << "Error: " << message << std::endl;
  exit(EXIT_FAILURE);
}

#define STRINGIFY(A) #A

int main(int argc, char* argv[])
{
  World *world = (World*)calloc(1, sizeof(*world));

  //
  // Initialize SDL, bind an OpenGL context and initialize glew.
  //

  if(SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    error("Failed to initialize SDL");
  }

  SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
    SDL_WINDOW_OPENGL, &world->window, &world->renderer);

  if(world->window == NULL || world->renderer == NULL)
    error("Failed to create window and renderer");

  world->glContext = SDL_GL_CreateContext(world->window);
  if(world->glContext == NULL) error("Failed to create OpenGL context");

  glewInit();

  //
  // Allocate and initialize OpenGL structures.
  //

  glGenVertexArrays(1, &world->objectVao);
  if(world->objectVao == 0) error("Failed to create VAO");
  glBindVertexArray(world->objectVao);

  glGenBuffers(1, &world->objectPositionVbo);
  if(world->objectPositionVbo == 0) error("Failed to create VBO");
  glBindBuffer(GL_ARRAY_BUFFER, world->objectPositionVbo);

  glm::vec3 objectPositions[] =
  {
#ifdef INITIAL_COLLIDING
    glm::vec3(-0.5f, 0.75f, 0),
    glm::vec3(-0.20f, 0.25f, 0),
    glm::vec3(-0.80f, 0.25f, 0)
#else
    glm::vec3(-0.75, 1, 0),
    glm::vec3(-0.5f, 0.5f, 0),
    glm::vec3(-1, 0.5f, 0)
#endif
  };

  world->objectPositions.assign(objectPositions,
    objectPositions + sizeof(objectPositions) / sizeof(glm::vec3));

  glBufferData(GL_ARRAY_BUFFER,
    sizeof(world->objectPositions[0]) * world->objectPositions.size(),
    &world->objectPositions[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenVertexArrays(1, &world->wallVao);
  if(world->wallVao == 0) error("Failed to create VAO");
  glBindVertexArray(world->wallVao);

  glGenBuffers(1, &world->wallPositionVbo);
  if(world->wallPositionVbo == 0) error("Failed to create VBO");
  glBindBuffer(GL_ARRAY_BUFFER, world->wallPositionVbo);

  glm::vec3 wallPositions[] =
  {
    glm::vec3(0, 0.9f, 0),
    glm::vec3(0.9f, -0.9f, 0),
    glm::vec3(-0.9f, -0.9f, 0)
  };

  world->wallPositions.assign(wallPositions,
    wallPositions + sizeof(wallPositions) / sizeof(glm::vec3));

  glBufferData(GL_ARRAY_BUFFER,
    sizeof(world->wallPositions[0]) * world->wallPositions.size(),
    &world->wallPositions[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  //
  // Allocate and initialize OpenGL shader program.
  //

  world->vertexShader = glCreateShader(GL_VERTEX_SHADER);

  GLchar *vertexSrc = STRINGIFY(
    #version 120\n

    uniform mat4 in_Model;

    attribute vec3 in_Position;

    void main()
    {
      gl_Position = in_Model * vec4(in_Position, 1.0);
    }
  );

  glShaderSource(world->vertexShader, 1, &vertexSrc, 0);
  glCompileShader(world->vertexShader);
  int compiled = 0;
  glGetShaderiv(world->vertexShader, GL_COMPILE_STATUS, &compiled);
  if(compiled == GL_FALSE) error("Failed to compile vertex shader");

  world->fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  GLchar *fragmentSrc = STRINGIFY(
    #version 120\n

    void main()
    {
      gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    }
  );

  glShaderSource(world->fragmentShader, 1, &fragmentSrc, 0);
  glCompileShader(world->fragmentShader);
  glGetShaderiv(world->fragmentShader, GL_COMPILE_STATUS, &compiled);
  if(compiled == GL_FALSE) error("Failed to compile fragment shader");

  world->shaderProgram = glCreateProgram();
  if(world->shaderProgram == 0) error("Failed to create shader program");

  glAttachShader(world->shaderProgram, world->vertexShader);
  glAttachShader(world->shaderProgram, world->fragmentShader);

  glBindAttribLocation(world->shaderProgram, 0, "in_Position");

  glLinkProgram(world->shaderProgram);

  int linked = 0;
  glGetProgramiv(world->shaderProgram, GL_LINK_STATUS, &linked);
  if(linked == GL_FALSE) error("Failed to link shader program");

  world->modelMatUniform = glGetUniformLocation(
    world->shaderProgram, "in_Model");

  if(world->modelMatUniform == -1) error("Failed to obtain uniform");

  //
  // Set initial object positions
  //

  world->objectModelMat = glm::mat4(1.0f);

  //
  // Run the main game loop
  //

  SDL_Event event = {};
  bool quit = false;

  while(quit == false)
  {
    while(SDL_PollEvent(&event) != 0)
    {
      if(event.type == SDL_QUIT)
      {
        quit = true;
      }
    }

    update(world);
    display(world);

    SDL_GL_SwapWindow(world->window);
  }

  return 0;
}

bool triangle_intersect(glm::vec3 a1, glm::vec3 b1, glm::vec3 c1,
                        glm::vec3 a2, glm::vec3 b2, glm::vec3 c2)
{
  int NoDivTriTriIsect(float V0[3],float V1[3],float V2[3],
                       float U0[3],float U1[3],float U2[3]);

  if(NoDivTriTriIsect((float*)&a1, (float*)&b1, (float*)&c1,
                      (float*)&a2, (float*)&b2, (float*)&c2) == 1)
  {
    return true;
  }

  return false;
}

/*
  a1 = glm::vec3(world->objectModelMat * glm::vec4(a1, 1.0f));
  b1 = glm::vec3(world->objectModelMat * glm::vec4(b1, 1.0f));
  c1 = glm::vec3(world->objectModelMat * glm::vec4(c1, 1.0f));
*/
