// Libraries...
#include <glad/glad.h>
#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <assimp/Importer.hpp>
#include <assimp/Scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"

// Includes
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
// Headers
#include "defines.h"
#include "shaders.cpp"

#include "model.cpp"
#include "bone.cpp"
#include "animation.cpp"
#include "animator.cpp"

int main(int argc, char *argv[]) 
{
    printf("Initializing SDL.\n");
    
    /* Initialize defaults, Video and Audio */
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    { 
        printf("Could not initialize SDL: %s.\n", SDL_GetError());
        return 1;
    }

    printf("SDL initialized.\n");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window *window = SDL_CreateWindow("AnimationSystem",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);    
    if(!window)
    {
        printf("Error Creating Window\n");
        return 1; 
    }   
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    if(!gladLoadGL())
    {
        printf("Error Initializing glad\n");
        return 1;
    }
    
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_DEPTH_TEST); 
    stbi_set_flip_vertically_on_load(true); 
    
#if 1
    Model testModel("../assets/cowboy/model.dae");
    Animation testAnimation("../assets/cowboy/model.dae", &testModel);
    Animator animator(&testAnimation);
#else
    Model testModel("../assets/model/boblampclean.md5mesh");
    Animation testAnimation("../assets/model/boblampclean.md5mesh", &testModel);
    Animator animator(&testAnimation);
#endif

    // build and compile shader program
    u32 vertexShader = CompileShaderFromFile("../src/shaders/vertex.glsl", GL_VERTEX_SHADER);
    u32 fragmentShader = CompileShaderFromFile("../src/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
    // link shaders
    u32 shaderProgram = CreateShaderProgram(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // create matrices and bind it to the shaders
    glm::mat4 persProjMatrix = glm::perspective(glm::radians(60.0f), (f32)WINDOW_WIDTH/(f32)WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 orthProjMatrix = glm::ortho(-WINDOW_WIDTH*0.5f, WINDOW_WIDTH*0.5f, WINDOW_HEIGHT*0.5f, -WINDOW_HEIGHT*0.5f, 0.1f, 100.0f);

    glm::mat4 worldMatrix = glm::mat4(1.0f);
    worldMatrix = glm::rotate(worldMatrix, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    worldMatrix = glm::scale(worldMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
#if 1
    glm::vec3 eye = glm::vec3(0, 5, 10);
    glm::vec3 target = glm::vec3(0, 5, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);
#else
    glm::vec3 eye = glm::vec3(0, 35, 80);
    glm::vec3 target = glm::vec3(0, 35, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);
#endif
    glm::mat4 viewMatrix = glm::lookAt(eye, target, up);

    glUseProgram(shaderProgram);
    
    i32 proj = glGetUniformLocation(shaderProgram, "proj");
    i32 view = glGetUniformLocation(shaderProgram, "view");
    i32 world = glGetUniformLocation(shaderProgram, "world");
    i32 tex = glGetUniformLocation(shaderProgram, "texture0");
    i32 gBones = glGetUniformLocation(shaderProgram, "gBones");

    glUniformMatrix4fv(proj, 1, false, &persProjMatrix[0][0]);
    glUniformMatrix4fv(view, 1, false, &viewMatrix[0][0]);
    glUniformMatrix4fv(world, 1, false, &worldMatrix[0][0]);

    b8 running = true;
    u32 lastTime = 0;
    while(running)
    {
        u32 currenTime = SDL_GetTicks();
        f32 dt = (currenTime - lastTime) / 1000.0f;
        lastTime = currenTime;
        
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        animator.UpdateAnimation(dt);

        auto transforms = animator.GetFinalBoneMatrices();
        for(i32 i = 0; i < transforms.size(); ++i)
        {
            glUniformMatrix4fv(gBones, (i32)transforms.size(), false, &transforms[0][0][0]);
        }
        
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shaderProgram);
        
        testModel.Draw(shaderProgram);
       
        SDL_GL_SwapWindow(window);

    }

    /* Shutdown all subsystems */ 
    glDeleteProgram(shaderProgram);

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    printf("Quiting SDL.\n");
    SDL_Quit();
    printf("Quiting....\n");

    return 0;
}
