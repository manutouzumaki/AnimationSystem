// Libraries...
#include "../imgui/imgui.h"
#include "../imgui/backends/imgui_impl_sdl.h"
#include "../imgui/backends/imgui_impl_opengl3.h"

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

struct Material
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    f32 shininess;
};

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

    // setup dear imgiu context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsLight();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 130");
    
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glEnable(GL_DEPTH_TEST); 
    stbi_set_flip_vertically_on_load(true);
    
    Model lightMesh("../assets/test.obj");

#if 1
    Model testModel("../assets/cowboy/model.dae");
    Animation testAnimation("../assets/cowboy/model.dae", &testModel);
    Animator animator(&testAnimation);
#else
    Model testModel("../assets/model/boblampclean.md5mesh");
    Animation testAnimation("../assets/model/boblampclean.md5mesh", &testModel);
    Animator animator(&testAnimation);
#endif

    u32 vertexShader = CompileShaderFromFile("../src/shaders/vertex.glsl", GL_VERTEX_SHADER);
    u32 fragmentShader = CompileShaderFromFile("../src/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
    u32 shaderProgram = CreateShaderProgram(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    u32 ligthVertexShader = CompileShaderFromFile("../src/shaders/ligthVertex.glsl", GL_VERTEX_SHADER);
    u32 ligthFragmentShader = CompileShaderFromFile("../src/shaders/ligthFragment.glsl", GL_FRAGMENT_SHADER);
    u32 ligthShaderProgram = CreateShaderProgram(ligthVertexShader, ligthFragmentShader);
    glDeleteShader(ligthVertexShader);
    glDeleteShader(ligthFragmentShader);

    // create matrices and bind it to the shaders
    glm::mat4 persProjMatrix = glm::perspective(glm::radians(60.0f), (f32)WINDOW_WIDTH/(f32)WINDOW_HEIGHT, 0.1f, 100.0f);
    glm::mat4 orthProjMatrix = glm::ortho(-WINDOW_WIDTH*0.5f, WINDOW_WIDTH*0.5f, WINDOW_HEIGHT*0.5f, -WINDOW_HEIGHT*0.5f, 0.1f, 100.0f);


#if 1
    glm::vec3 eye = glm::vec3(2, 3, 8);
    glm::vec3 target = glm::vec3(0, 0, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);
#else
    glm::vec3 eye = glm::vec3(0, 35, 80);
    glm::vec3 target = glm::vec3(0, 35, 0);
    glm::vec3 up = glm::vec3(0, 1, 0);
#endif
    glm::mat4 viewMatrix = glm::lookAt(eye, target, up);

    glm::vec3 ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
    f32 shininessColor = 2.0f;

    glm::vec3 ambientLightColor = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuseLightColor = glm::vec3(0.5f, 0.5f, 0.5f);
    glm::vec3 specularLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    glUseProgram(shaderProgram);
    
    i32 proj = glGetUniformLocation(shaderProgram, "proj");
    i32 view = glGetUniformLocation(shaderProgram, "view");
    i32 world = glGetUniformLocation(shaderProgram, "world");
    i32 tex = glGetUniformLocation(shaderProgram, "texture0");
    i32 gBones = glGetUniformLocation(shaderProgram, "gBones");
    i32 viewPos = glGetUniformLocation(shaderProgram, "viewPos");

    i32 ambientMat = glGetUniformLocation(shaderProgram, "material.ambient");
    i32 diffuseMat = glGetUniformLocation(shaderProgram, "material.diffuse");
    i32 specularMat = glGetUniformLocation(shaderProgram, "material.specular");
    i32 shininessMat = glGetUniformLocation(shaderProgram, "material.shininess");

    i32 positionLight = glGetUniformLocation(shaderProgram, "light.position");
    i32 ambientLight = glGetUniformLocation(shaderProgram, "light.ambient");
    i32 diffuseLight = glGetUniformLocation(shaderProgram, "light.diffuse");
    i32 specularLight = glGetUniformLocation(shaderProgram, "light.specular");

    glUniformMatrix4fv(proj, 1, false, &persProjMatrix[0][0]);
    glUniformMatrix4fv(view, 1, false, &viewMatrix[0][0]);
    glUniform3fv(viewPos, 1, &eye[0]);

    glUseProgram(ligthShaderProgram);
    
    i32 projLigth = glGetUniformLocation(ligthShaderProgram, "proj");
    i32 viewLigth = glGetUniformLocation(ligthShaderProgram, "view");
    i32 worldLigth = glGetUniformLocation(ligthShaderProgram, "world");
    i32 ligthColor = glGetUniformLocation(ligthShaderProgram, "ligthColor");
    
    glUniformMatrix4fv(projLigth, 1, false, &persProjMatrix[0][0]);
    glUniformMatrix4fv(viewLigth, 1, false, &viewMatrix[0][0]);

    bool show_demo_window = true;

    b8 running = true;
    u32 lastTime = 0;
    while(running)
    {
        u32 currenTime = SDL_GetTicks();
        f32 dt = (currenTime - lastTime) / 1000.0f;
        lastTime = currenTime;

        local_persist float angle = 0.0f;
        glm::vec3 lightPosV = glm::vec3(glm::vec3(4.0f*sinf(angle), 2*sinf((angle*8)), 4.0f*cosf(angle)));
        angle += dt;
        
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if(event.type == SDL_QUIT)
            {
                running = false;
            }
        }
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        glUseProgram(shaderProgram);

        ImGui::Begin("Material");
        ImGui::ColorEdit3("ambient", (f32 *)&ambientColor);
        ImGui::ColorEdit3("diffuse", (f32 *)&diffuseColor);
        ImGui::ColorEdit3("specular", (f32 *)&specularColor);
        if(ImGui::Button("-"))
            shininessColor  /= 2;
        ImGui::SameLine();
        if(ImGui::Button("+"))
            shininessColor *= 2;
        ImGui::SameLine();
        ImGui::Text("shininess %f", shininessColor);
        ImGui::End();

        glUniform3fv(ambientMat, 1, &ambientColor[0]);
        glUniform3fv(diffuseMat, 1, &diffuseColor[0]);
        glUniform3fv(specularMat, 1, &specularColor[0]);
        glUniform1f(shininessMat, shininessColor);

        ImGui::Begin("Light");
        ImGui::ColorEdit3("ambient", (f32 *)&ambientLightColor);
        ImGui::ColorEdit3("diffuse", (f32 *)&diffuseLightColor);
        ImGui::ColorEdit3("specular", (f32 *)&specularLightColor);
        ImGui::End();

        glUniform3fv(ambientLight, 1, &ambientLightColor[0]);
        glUniform3fv(diffuseLight, 1, &diffuseLightColor[0]);
        glUniform3fv(specularLight, 1, &specularLightColor[0]);
        
        glUseProgram(ligthShaderProgram);
        glUniform3fv(ligthColor, 1, &ambientLightColor[0]);
        
        animator.UpdateAnimation(dt);
        auto transforms = animator.GetFinalBoneMatrices();
        for(i32 i = 0; i < transforms.size(); ++i)
        {
            glUniformMatrix4fv(gBones, (i32)transforms.size(), false, &transforms[0][0][0]);
        }
        
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(ligthShaderProgram);
        
        glm::mat4 worldMatrix = glm::mat4(1.0f);
        worldMatrix = glm::translate(worldMatrix, lightPosV);
        worldMatrix = glm::scale(worldMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
        glUniformMatrix4fv(worldLigth, 1, false, &worldMatrix[0][0]);
        lightMesh.Draw(ligthShaderProgram);

        glUseProgram(shaderProgram);
        
        glUniform3fv(positionLight, 1, &lightPosV[0]);

        worldMatrix = glm::mat4(1.0f);
        worldMatrix = glm::translate(worldMatrix, glm::vec3(0.0f, -4.0f, 0.0f));
        worldMatrix = glm::rotate(worldMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        worldMatrix = glm::scale(worldMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(world, 1, false, &worldMatrix[0][0]);
        testModel.Draw(shaderProgram);

        glUseProgram(0);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        

        SDL_GL_SwapWindow(window);

    }

    /* Shutdown all subsystems */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    glDeleteProgram(shaderProgram);

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    printf("Quiting SDL.\n");
    SDL_Quit();
    printf("Quiting....\n");

    return 0;
}
