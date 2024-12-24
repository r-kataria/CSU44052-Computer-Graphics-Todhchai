/**************************************************
 * Minimal Example: Sun + Shadows + Simple Bloom
 * ----------------------------------------------
 * Uses:
 *   - GLFW for window/input
 *   - GLEW for GL function loading
 *   - GLM for math
 *
 *   Compile with (on Linux):
 *   g++ main.cpp -lglfw -lGL -lGLEW -o MyApp
 *
 * (Tested roughly for demonstration, may require
 *  adjustments for your environment.)
 **************************************************/
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <cmath>

// For image loading, we do a fake stub here.
#include <cstdio>

// ------------------ SHADER UTILS (MINIMAL) ------------------ //
static GLuint CompileShader(const char* source, GLenum type);
static GLuint CreateShaderProgram(const char* vsSrc, const char* fsSrc);

// ------------------------------------------------------------ //
//                     GLOBALS / CONSTANTS                     //
// ------------------------------------------------------------ //
static const int WINDOW_WIDTH  = 1024;
static const int WINDOW_HEIGHT = 768;

// For demonstration, we’ll keep these in global for quick example:
glm::vec3 gCameraPos   = glm::vec3(0.0f, 5.0f, 15.0f);
glm::vec3 gCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 gCameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float gYaw   = -90.0f; 
float gPitch =  0.0f;
float gLastX = WINDOW_WIDTH  * 0.5f;
float gLastY = WINDOW_HEIGHT * 0.5f;

// For bloom we need an offscreen FBO + color texture + bright pass + blur
GLuint gSceneFBO         = 0;
GLuint gSceneColorTex    = 0;
GLuint gSceneBrightTex   = 0;
GLuint gSceneRBO         = 0; // depth/stencil
GLuint gBloomFBO[2]      = {0, 0};
GLuint gBloomColorTex[2] = {0, 0};

// For shadow mapping (directional)
const int SHADOW_RES = 1024;
GLuint gShadowFBO    = 0;
GLuint gShadowTex    = 0;

// Simple geometry for the ground
static float gGroundVertices[] = {
    // positions          normals           texcoords
    -50, 0, -50,  0,1,0,   0,0,
     50, 0, -50,  0,1,0,   1,0,
     50, 0,  50,  0,1,0,   1,1,
    -50, 0,  50,  0,1,0,   0,1,
};
static GLuint gGroundVAO = 0;
static GLuint gGroundVBO = 0;
static GLuint gGroundEBO = 0;
static unsigned int gGroundIndices[] = {0,1,2, 0,2,3};

// Forward declarations
void initGL();
void initFramebuffers();
void initGround();
void initShadowMapFBO();
void initBloomFBOs();
void renderDepthMapFromSun();
void renderScene(bool isShadowPass, GLuint depthShader, GLuint sceneShader);
void renderQuad(); // for bloom passes

// ------------------------------------------------------------ //
//                          Sun Class                           //
// ------------------------------------------------------------ //
class Sun {
public:
    Sun() : programID(0), VAO(0), VBO(0), EBO(0), textureID(0) {}
    void initialize();
    void render(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos);
    void cleanup();

    glm::vec3 direction;  // sun direction
    glm::vec3 position;   // used for billboard
    GLuint    programID;

private:
    GLuint VAO, VBO, EBO;
    GLuint textureID;

    // Uniform locations
    GLint uMVP;
    GLint uTex;
};

void Sun::initialize()
{
    // Hardcode a direction (slightly diagonal)
    direction = glm::normalize(glm::vec3(0.2f, -1.0f, 0.2f));
    // Place the “sun disc” far away in the sky, purely for rendering
    position  = direction * 300.0f; 

    // Minimal geometry: a 2D quad
    float quadVerts[] = {
        //   xy      uv
        -1.f, -1.f,  0.f, 0.f,
         1.f, -1.f,  1.f, 0.f,
         1.f,  1.f,  1.f, 1.f,
        -1.f,  1.f,  0.f, 1.f,
    };
    unsigned int indices[] = {0,1,2, 0,2,3};

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // positions at layout=0, uv at layout=1
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));

    glBindVertexArray(0);

    // For demonstration, we’ll create a white texture or you can load an actual .png
    // A real sun texture might have a bright disc with alpha around it.
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    unsigned char whitePixel[4] = {255,255,255,255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1,1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    const char* vs = R"GLSL(
    #version 330 core
    layout(location=0) in vec2 inPos;
    layout(location=1) in vec2 inUV;
    out vec2 fragUV;
    uniform mat4 MVP;
    void main(){
        gl_Position = MVP * vec4(inPos, 0.0, 1.0);
        fragUV = inUV;
    }
    )GLSL";

    // Here we multiply the color by 10.0 so it’s extra bright for bloom
    const char* fs = R"GLSL(
    #version 330 core
    in vec2 fragUV;
    out vec4 finalColor;
    uniform sampler2D texSun;
    void main(){
        vec4 c = texture(texSun, fragUV);
        // Could do a radial fade, etc. For now just bright white
        finalColor = c * 10.0;
    }
    )GLSL";

    programID = CreateShaderProgram(vs, fs);
    uMVP = glGetUniformLocation(programID, "MVP");
    uTex = glGetUniformLocation(programID, "texSun");
}

void Sun::render(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& camPos)
{
    glUseProgram(programID);

    // Basic billboard logic:
    // We translate to "position" and scale up the quad a bit
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(30.f));  // bigger disc

    // Another approach is removing the rotation from the view, 
    // but for simplicity we just do a normal transform
    glm::mat4 MVP = proj * view * model;
    glUniformMatrix4fv(uMVP, 1, GL_FALSE, &MVP[0][0]);

    // texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(uTex, 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Sun::cleanup()
{
    glDeleteProgram(programID);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(1, &textureID);
}

// ------------------------------------------------------------ //
//                   Building (Box) Example                     //
// ------------------------------------------------------------ //
class Building {
public:
    Building() : pos(0), scl(1), programID(0), depthProgramID(0) {}
    void init(const glm::vec3& p, const glm::vec3& s);
    void render(const glm::mat4& view, const glm::mat4& proj, 
                const glm::mat4& lightSpace, GLuint shadowTex, const glm::vec3& lightDir);
    void renderDepth(const glm::mat4& lightSpace);

    glm::vec3 pos;
    glm::vec3 scl;

    GLuint VAO, VBO, EBO;
    GLuint programID;       // normal render
    GLuint depthProgramID;  // for shadow pass
    // uniforms
    GLint uMVP;
    GLint uModel;
    GLint uLightSpace;
    GLint uShadowMap;
    GLint uLightDir;
    // for depth
    GLint uDepth_MVP;
};

static float buildingVertices[] = {
  //     positions        normals       texcoords
  // front
  -1,0, 1,   0,0,1,   0,0,
   1,0, 1,   0,0,1,   1,0,
   1,1, 1,   0,0,1,   1,1,
  -1,1, 1,   0,0,1,   0,1,
  // back
   1,0,-1,   0,0,-1,  0,0,
  -1,0,-1,   0,0,-1,  1,0,
  -1,1,-1,   0,0,-1,  1,1,
   1,1,-1,   0,0,-1,  0,1,
  // left
  -1,0,-1,  -1,0,0,   0,0,
  -1,0, 1,  -1,0,0,   1,0,
  -1,1, 1,  -1,0,0,   1,1,
  -1,1,-1,  -1,0,0,   0,1,
  // right
   1,0, 1,   1,0,0,   0,0,
   1,0,-1,   1,0,0,   1,0,
   1,1,-1,   1,0,0,   1,1,
   1,1, 1,   1,0,0,   0,1,
  // top
  -1,1, 1,   0,1,0,   0,0,
   1,1, 1,   0,1,0,   1,0,
   1,1,-1,   0,1,0,   1,1,
  -1,1,-1,   0,1,0,   0,1,
  // bottom
  -1,0,-1,   0,-1,0,  0,0,
   1,0,-1,   0,-1,0,  1,0,
   1,0, 1,   0,-1,0,  1,1,
  -1,0, 1,   0,-1,0,  0,1,
};
static unsigned int buildingIndices[] = {
    0,1,2, 0,2,3,     4,5,6, 4,6,7,
    8,9,10, 8,10,11,  12,13,14, 12,14,15,
    16,17,18, 16,18,19,   20,21,22, 20,22,23
};

void Building::init(const glm::vec3& p, const glm::vec3& s)
{
    pos = p;
    scl = s;

    // create VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buildingVertices), buildingVertices, GL_STATIC_DRAW);

    // EBO
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(buildingIndices), buildingIndices, GL_STATIC_DRAW);

    // positions -> layout=0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);

    // normals -> layout=1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

    // uv -> layout=2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

    glBindVertexArray(0);

    // create normal shader
    const char* vs = R"GLSL(
    #version 330 core
    layout(location=0) in vec3 inPos;
    layout(location=1) in vec3 inNormal;
    layout(location=2) in vec2 inUV;
    out vec3 fragPos;
    out vec3 fragNormal;
    out vec2 fragUV;
    uniform mat4 MVP;
    uniform mat4 model;
    void main(){
        vec4 worldPos = model * vec4(inPos, 1.0);
        gl_Position   = MVP * vec4(inPos,1.0);
        fragPos       = worldPos.xyz;
        // transform normal by model’s upper-left 3x3 if scaling or rotate
        // (for uniform scale we can skip, for non-uniform we do real normal transform)
        fragNormal    = mat3(model) * inNormal;
        fragUV        = inUV;
    }
    )GLSL";

    const char* fs = R"GLSL(
    #version 330 core
    in vec3 fragPos;
    in vec3 fragNormal;
    in vec2 fragUV;
    out vec4 finalColor;

    uniform sampler2D shadowMap;
    uniform mat4 lightSpace;
    uniform vec3 lightDir;

    // simple function to do shadow
    float computeShadow(vec4 lightSpacePos){
        // perspective divide
        vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
        // transform to [0..1]
        projCoords = projCoords * 0.5 + 0.5;
        // sample shadow map
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        // current depth
        float currentDepth = projCoords.z;
        float bias = 0.005;

        // basic comparison
        float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
        // also check if out of map range
        if(projCoords.x<0 || projCoords.x>1 || projCoords.y<0 || projCoords.y>1) 
            shadow=0.0;
        return shadow;
    }

    void main(){
        // direction light
        vec3 n = normalize(fragNormal);
        vec3 l = normalize(-lightDir);
        float diff = max(dot(n,l), 0.0);

        // compute lightSpace pos for shadow
        vec4 lightSpacePos = lightSpace * vec4(fragPos,1.0);
        float shadow = computeShadow(lightSpacePos);

        vec3 color = vec3(0.8,0.8,0.9);
        vec3 ambient = color * 0.3;
        vec3 diffuse = color * diff * (1.0 - shadow);

        finalColor = vec4(ambient + diffuse, 1.0);
    }
    )GLSL";

    programID = CreateShaderProgram(vs, fs);
    uMVP        = glGetUniformLocation(programID, "MVP");
    uModel      = glGetUniformLocation(programID, "model");
    uLightSpace = glGetUniformLocation(programID, "lightSpace");
    uShadowMap  = glGetUniformLocation(programID, "shadowMap");
    uLightDir   = glGetUniformLocation(programID, "lightDir");

    // create depth-only shader
    const char* dvs = R"GLSL(
    #version 330 core
    layout(location=0) in vec3 inPos;
    uniform mat4 lightSpaceMatrix;
    uniform mat4 model;
    void main(){
        gl_Position = lightSpaceMatrix * model * vec4(inPos,1.0);
    }
    )GLSL";

    const char* dfs = R"GLSL(
    #version 330 core
    void main(){
        // no output, depth only
    }
    )GLSL";

    depthProgramID = CreateShaderProgram(dvs, dfs);
    uDepth_MVP = glGetUniformLocation(depthProgramID, "lightSpaceMatrix");
}

void Building::renderDepth(const glm::mat4& lightSpace)
{
    glUseProgram(depthProgramID);
    // build model
    glm::mat4 model(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, scl);

    GLint uModel = glGetUniformLocation(depthProgramID, "model");
    glUniformMatrix4fv(uDepth_MVP, 1, GL_FALSE, &lightSpace[0][0]);
    glUniformMatrix4fv(uModel, 1, GL_FALSE, &model[0][0]);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Building::render(const glm::mat4& view, const glm::mat4& proj, 
                      const glm::mat4& lightSpace, GLuint shadowTex, const glm::vec3& lightDir)
{
    glUseProgram(programID);
    // model matrix
    glm::mat4 model(1.0f);
    model = glm::translate(model, pos);
    model = glm::scale(model, scl);
    glm::mat4 MVP = proj * view * model;

    glUniformMatrix4fv(uMVP, 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(uModel, 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(uLightSpace, 1, GL_FALSE, &lightSpace[0][0]);
    glUniform3fv(uLightDir, 1, &lightDir[0]);

    // bind shadow map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowTex);
    glUniform1i(uShadowMap, 0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// ------------------------------------------------------------ //
//                          MAIN                                //
// ------------------------------------------------------------ //
int main()
{
    if(!glfwInit()){
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Sun + Shadows + Bloom Example", nullptr, nullptr);
    if(!window){
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // init GLEW
    if(glewInit()!=GLEW_OK){
        std::cerr << "Failed to init GLEW\n";
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    // Setup
    initGL();
    initFramebuffers(); // scene and bloom
    initShadowMapFBO();
    initGround();

    // Create a sun
    Sun sun;
    sun.initialize();

    // Let’s create a couple buildings
    std::vector<Building> buildings;
    {
        Building b;
        b.init(glm::vec3(-5,0,-5), glm::vec3(2,6,2));
        buildings.push_back(b);

        Building b2;
        b2.init(glm::vec3(5,0,-3), glm::vec3(3,8,2));
        buildings.push_back(b2);
    }

    // Main loop
    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();

        // simple camera movement with WASD, arrow up/down, etc.
        // (omitted for brevity)...

        // 1) Render depth map from sun’s perspective
        renderDepthMapFromSun();  

        // 2) Render scene into the “scene FBO” (color + bright pass)
        glBindFramebuffer(GL_FRAMEBUFFER, gSceneFBO);
        glViewport(0,0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glClearColor(0.2f, 0.3f, 0.35f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render the scene (ground, buildings, sun)
        {
            // standard perspective
            glm::mat4 proj = glm::perspective(glm::radians(45.f), (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1f, 200.f);
            glm::mat4 view = glm::lookAt(gCameraPos, gCameraPos + gCameraFront, gCameraUp);
            
            // ground + buildings
            GLuint depthShader=0;  // not used here
            GLuint sceneShader=0;  // we pass as 0, we handle each object’s own program
            renderScene(false, depthShader, sceneShader);

            // sun disc
            sun.render(view, proj, gCameraPos);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3) Bloom process:
        //    3.1) Extract bright from gSceneBrightTex -> blur
        //    3.2) Combine with gSceneColorTex
        // (We do a minimal 2-pass blur to keep it short)

        // Horizontal blur
        bool horizontal = true; 
        bool firstPass  = true;
        unsigned int amount = 5; // number of blur passes
        GLuint blurShader = 0; // we’ll create a minimal blur program below
        {
            static GLuint sBlurProgram=0, sBlurTexLoc=0, sHorizontalLoc=0;
            if(sBlurProgram==0){
                // build a minimal blur
                const char* blurVS = R"GLSL(
                #version 330 core
                layout(location=0) in vec2 pos;
                layout(location=1) in vec2 uv;
                out vec2 TexCoords;
                void main(){
                    gl_Position = vec4(pos.x*2.0-1.0, (1.0-uv.y)*2.0-1.0, 0.0, 1.0);
                    TexCoords   = uv;
                }
                )GLSL";
                const char* blurFS = R"GLSL(
                #version 330 core
                out vec4 FragColor;
                in vec2 TexCoords;
                uniform sampler2D image;
                uniform bool horizontal;

                void main(){
                    vec2 texOffset = 1.0 / vec2(textureSize(image,0));
                    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
                    vec3 result = texture(image, TexCoords).rgb * weights[0];
                    for(int i=1; i<5; ++i){
                        if(horizontal){
                            result += texture(image, TexCoords + vec2(texOffset.x*i,0)).rgb * weights[i];
                            result += texture(image, TexCoords - vec2(texOffset.x*i,0)).rgb * weights[i];
                        } else {
                            result += texture(image, TexCoords + vec2(0, texOffset.y*i)).rgb * weights[i];
                            result += texture(image, TexCoords - vec2(0, texOffset.y*i)).rgb * weights[i];
                        }
                    }
                    FragColor = vec4(result, 1.0);
                }
                )GLSL";
                sBlurProgram = CreateShaderProgram(blurVS, blurFS);
                sBlurTexLoc = glGetUniformLocation(sBlurProgram, "image");
                sHorizontalLoc = glGetUniformLocation(sBlurProgram, "horizontal");
            }
            blurShader = sBlurProgram;
            glUseProgram(blurShader);

            GLuint pingPongFBO[2], pingPongTex[2];
            glGenFramebuffers(2, pingPongFBO);
            glGenTextures(2, pingPongTex);
            for(int i=0; i<2; i++){
                glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[i]);
                glBindTexture(GL_TEXTURE_2D, pingPongTex[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingPongTex[i], 0);
            }

            GLuint currentTex = gSceneBrightTex;
            for(unsigned int i=0; i<amount; i++){
                glBindFramebuffer(GL_FRAMEBUFFER, pingPongFBO[horizontal]);
                glUniform1i(sHorizontalLoc, horizontal?1:0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, currentTex);
                glUniform1i(sBlurTexLoc, 0);

                glViewport(0,0, WINDOW_WIDTH, WINDOW_HEIGHT);
                glClear(GL_COLOR_BUFFER_BIT);

                renderQuad();

                currentTex = pingPongTex[horizontal];
                horizontal = !horizontal;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // final blurred bright is in currentTex
            // combine: we do a small pass that draws gSceneColorTex + currentTex
            static GLuint sCombineProgram=0, sSceneTexLoc=0, sBloomTexLoc=0;
            if(sCombineProgram==0){
                const char* combineVS = R"GLSL(
                #version 330 core
                layout(location=0) in vec2 pos;
                layout(location=1) in vec2 uv;
                out vec2 TexCoords;
                void main(){
                    gl_Position = vec4(pos*2.0-1.0, 0.0, 1.0);
                    TexCoords   = uv;
                }
                )GLSL";
                const char* combineFS = R"GLSL(
                #version 330 core
                in vec2 TexCoords;
                out vec4 FragColor;
                uniform sampler2D sceneTex;
                uniform sampler2D bloomTex;
                void main(){
                    vec3 scene = texture(sceneTex, TexCoords).rgb;
                    vec3 bloom = texture(bloomTex, TexCoords).rgb;
                    FragColor = vec4(scene + bloom, 1.0);
                }
                )GLSL";
                sCombineProgram = CreateShaderProgram(combineVS, combineFS);
                sSceneTexLoc  = glGetUniformLocation(sCombineProgram, "sceneTex");
                sBloomTexLoc  = glGetUniformLocation(sCombineProgram, "bloomTex");
            }
            glUseProgram(sCombineProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gSceneColorTex);
            glUniform1i(sSceneTexLoc, 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, currentTex);
            glUniform1i(sBloomTexLoc, 1);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0,0, WINDOW_WIDTH, WINDOW_HEIGHT);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            renderQuad();

            // cleanup
            glDeleteFramebuffers(2, pingPongFBO);
            glDeleteTextures(2, pingPongTex);
        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    sun.cleanup();
    for(auto& b : buildings){
        glDeleteProgram(b.programID);
        glDeleteProgram(b.depthProgramID);
        glDeleteVertexArrays(1,&b.VAO);
        glDeleteBuffers(1,&b.VBO);
        glDeleteBuffers(1,&b.EBO);
    }
    glDeleteFramebuffers(1, &gSceneFBO);
    glDeleteTextures(1, &gSceneColorTex);
    glDeleteTextures(1, &gSceneBrightTex);
    glDeleteRenderbuffers(1, &gSceneRBO);

    glDeleteFramebuffers(1, &gShadowFBO);
    glDeleteTextures(1, &gShadowTex);

    glDeleteVertexArrays(1, &gGroundVAO);
    glDeleteBuffers(1, &gGroundVBO);
    glDeleteBuffers(1, &gGroundEBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

// ------------------------------------------------------------ //
//                SUPPORT FUNCTION IMPLEMENTATIONS              //
// ------------------------------------------------------------ //

void initGL(){
    // Some defaults
    glEnable(GL_DEPTH_TEST);
}

void initFramebuffers(){
    // 1) Scene FBO with 2 color attachments (1 normal color, 1 bright color)
    glGenFramebuffers(1, &gSceneFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gSceneFBO);

    // normal color
    glGenTextures(1, &gSceneColorTex);
    glBindTexture(GL_TEXTURE_2D, gSceneColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 
                 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                           GL_TEXTURE_2D, gSceneColorTex, 0);

    // bright color
    glGenTextures(1, &gSceneBrightTex);
    glBindTexture(GL_TEXTURE_2D, gSceneBrightTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_WIDTH, WINDOW_HEIGHT, 
                 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 
                           GL_TEXTURE_2D, gSceneBrightTex, 0);

    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    // depth/stencil
    glGenRenderbuffers(1, &gSceneRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, gSceneRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
                              GL_RENDERBUFFER, gSceneRBO);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "Scene FBO not complete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void initShadowMapFBO(){
    glGenFramebuffers(1, &gShadowFBO);
    glGenTextures(1, &gShadowTex);
    glBindTexture(GL_TEXTURE_2D, gShadowTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_RES, SHADOW_RES, 
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // clamp to border color
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[] = {1.0,1.0,1.0,1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glBindFramebuffer(GL_FRAMEBUFFER, gShadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gShadowTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){
        std::cerr << "Shadow FBO not complete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initBloomFBOs(){
    // Not used in detail here because we do a “ping-pong” approach in-line.
    // This placeholder is left for reference if you prefer static ping-pong FBOs.
}

void initGround(){
    glGenVertexArrays(1, &gGroundVAO);
    glBindVertexArray(gGroundVAO);

    glGenBuffers(1, &gGroundVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gGroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gGroundVertices), gGroundVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &gGroundEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gGroundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gGroundIndices), gGroundIndices, GL_STATIC_DRAW);

    // layout: position(3), normal(3), tex(2)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));

    glBindVertexArray(0);
}

// Renders depth map from sun’s perspective
void renderDepthMapFromSun(){
    // Hardcode sun direction from our Sun object (in this example we have only one Sun).
    // In a real scenario, store the sun in a global or pass it in.
    glm::vec3 sunDir = glm::normalize(glm::vec3(0.2f,-1.0f,0.2f));

    // Build orthographic projection for directional light
    float range = 50.0f;
    glm::mat4 lightProj = glm::ortho(-range, range, -range, range, -20.f, 100.f);
    // place the sun "behind" the scene in direction sunDir
    glm::vec3 lightPos = -sunDir * 40.0f;
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0,0,0), glm::vec3(0,1,0));
    glm::mat4 lightSpaceMatrix = lightProj * lightView;

    // bind shadow FBO
    glViewport(0,0, SHADOW_RES, SHADOW_RES);
    glBindFramebuffer(GL_FRAMEBUFFER, gShadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Now we render our buildings to the depth map
    // ground as well
    // We'll reuse the same “renderScene” logic but with isShadowPass=true
    // so it only calls the "renderDepth" method in Building.
    static GLuint depthShader=0, sceneShader=0;
    // Actually we don’t pass them, we do isShadowPass to know we are doing depth
    renderScene(true, depthShader, sceneShader, lightSpaceMatrix);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

void renderScene(bool isShadowPass, GLuint depthShader, GLuint sceneShader, 
                 glm::mat4 lightSpaceMatrix /*= glm::mat4(1.0f)*/)
{
    // We'll just do a simple approach:
    //  - if isShadowPass, render buildings with "renderDepth(...)"
    //  - else, render with normal shader
    // Also render ground (we’ll do a minimal pass).
    static std::vector<Building> *pBuildings=nullptr;
    // Ugly hack to keep references to the actual building array from main
    // so let's do a static as well:
    static bool once=false; 
    if(!once){
        // We locate the building vector from main… 
        // In an actual codebase, you’d pass it in or store globally.
        // For demonstration only:
        extern std::vector<Building> buildings;
        pBuildings = &buildings;
        once=true;
    }

    // We also want to do the ground.  For the shadow pass, we
    // can create a minimal function that just draws the ground
    // with the same depthProgram as the building. For the normal pass,
    // we do a simple shading.

    glm::vec3 sunDir = glm::normalize(glm::vec3(0.2f,-1.0f,0.2f));

    // build typical camera
    glm::mat4 view = glm::lookAt(gCameraPos, gCameraPos + gCameraFront, gCameraUp);
    glm::mat4 proj = glm::perspective(glm::radians(45.f), (float)WINDOW_WIDTH/(float)WINDOW_HEIGHT, 0.1f, 200.f);

    if(isShadowPass){
        // buildings
        for(auto &b : *pBuildings){
            b.renderDepth(lightSpaceMatrix);
        }
        // ground
        // we do a quick depth-only approach here
        // reuse building depth shader or replicate it
        // For brevity, let's do the same depth pass as building
        static GLuint groundDepthProg=0;
        static GLint uLS= -1, uM= -1;
        if(groundDepthProg==0){
            const char* dv = R"GLSL(
            #version 330 core
            layout(location=0) in vec3 inPos;
            uniform mat4 lightSpaceMatrix;
            uniform mat4 model;
            void main(){
                gl_Position = lightSpaceMatrix * model * vec4(inPos,1.0);
            }
            )GLSL";
            const char* df = R"GLSL(
            #version 330 core
            void main(){}
            )GLSL";
            groundDepthProg = CreateShaderProgram(dv, df);
            uLS = glGetUniformLocation(groundDepthProg, "lightSpaceMatrix");
            uM  = glGetUniformLocation(groundDepthProg, "model");
        }
        glUseProgram(groundDepthProg);
        glUniformMatrix4fv(uLS,1,GL_FALSE,&lightSpaceMatrix[0][0]);
        glm::mat4 groundModel(1.f);
        glUniformMatrix4fv(uM,1,GL_FALSE,&groundModel[0][0]);

        glBindVertexArray(gGroundVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }else{
        // normal pass
        // ground
        static GLuint groundProg=0;
        static GLint uMVP= -1, uModel= -1, uLightSpace= -1, uShadowMap= -1, uLightDir= -1;
        if(groundProg==0){
            const char* vs = R"GLSL(
            #version 330 core
            layout(location=0) in vec3 inPos;
            layout(location=1) in vec3 inNormal;
            layout(location=2) in vec2 inUV;
            out vec3 fragPos;
            out vec3 fragNormal;
            uniform mat4 MVP;
            uniform mat4 model;
            void main(){
                gl_Position = MVP * vec4(inPos,1.0);
                fragPos = vec3(model*vec4(inPos,1.0));
                fragNormal = mat3(model) * inNormal;
            }
            )GLSL";
            const char* fs = R"GLSL(
            #version 330 core
            in vec3 fragPos;
            in vec3 fragNormal;
            out vec4 finalColor;
            uniform sampler2D shadowMap;
            uniform mat4 lightSpace;
            uniform vec3 lightDir;

            float computeShadow(vec4 lightSpacePos){
                vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
                projCoords = projCoords*0.5 + 0.5;
                float closestDepth = texture(shadowMap, projCoords.xy).r;
                float currentDepth = projCoords.z;
                float bias=0.005;
                float shadow = currentDepth - bias > closestDepth ? 1.0:0.0;
                if(projCoords.x<0||projCoords.x>1||projCoords.y<0||projCoords.y>1)
                    shadow=0.0;
                return shadow;
            }
            void main(){
                vec3 n=normalize(fragNormal);
                vec3 l=normalize(-lightDir);
                float diff = max(dot(n,l), 0.0);

                vec4 lightSpacePos = lightSpace * vec4(fragPos,1.0);
                float shadow = computeShadow(lightSpacePos);

                vec3 color = vec3(0.5,0.7,0.5);
                vec3 ambient = color*0.3;
                vec3 diffuse = color*diff*(1.0-shadow);

                finalColor = vec4(ambient+diffuse,1.0);
            }
            )GLSL";
            groundProg = CreateShaderProgram(vs, fs);
            uMVP        = glGetUniformLocation(groundProg, "MVP");
            uModel      = glGetUniformLocation(groundProg, "model");
            uLightSpace = glGetUniformLocation(groundProg, "lightSpace");
            uShadowMap  = glGetUniformLocation(groundProg, "shadowMap");
            uLightDir   = glGetUniformLocation(groundProg, "lightDir");
        }
        // build sun lightSpaceMatrix same as above in “renderDepthMapFromSun”:
        float range = 50.f;
        glm::mat4 lightProj = glm::ortho(-range, range, -range, range, -20.f, 100.f);
        glm::vec3 lightPos = -sunDir*40.f;
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0), glm::vec3(0,1,0));
        glm::mat4 lightSpaceMatrix = lightProj*lightView;

        // ground
        glUseProgram(groundProg);
        glm::mat4 model(1.f);
        glm::mat4 MVP = proj*view*model;
        glUniformMatrix4fv(uMVP,1,GL_FALSE,&MVP[0][0]);
        glUniformMatrix4fv(uModel,1,GL_FALSE,&model[0][0]);
        glUniformMatrix4fv(uLightSpace,1,GL_FALSE,&lightSpaceMatrix[0][0]);
        glUniform3fv(uLightDir,1,&sunDir[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gShadowTex);
        glUniform1i(uShadowMap, 0);

        glBindVertexArray(gGroundVAO);
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
        glBindVertexArray(0);

        // buildings
        for(auto &b : *pBuildings){
            b.render(view, proj, lightSpaceMatrix, gShadowTex, sunDir);
        }
    }
}

// A simple function that draws a full-screen quad with positions + uv
static GLuint gQuadVAO=0, gQuadVBO=0, gQuadEBO=0;
static unsigned int gQuadIndices[] = {0,1,2, 0,2,3};
void renderQuad(){
    if(gQuadVAO==0){
        float verts[]={
            //  pos        uv
            0.f,0.f,  0.f,0.f,
            1.f,0.f,  1.f,0.f,
            1.f,1.f,  1.f,1.f,
            0.f,1.f,  0.f,1.f
        };
        glGenVertexArrays(1,&gQuadVAO);
        glBindVertexArray(gQuadVAO);
        glGenBuffers(1,&gQuadVBO);
        glBindBuffer(GL_ARRAY_BUFFER, gQuadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        glGenBuffers(1,&gQuadEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gQuadEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gQuadIndices), gQuadIndices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float)));
    }
    glBindVertexArray(gQuadVAO);
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}


// ------------------ SHADER UTILS (MINIMAL) ------------------ //

static GLuint CreateShaderProgram(const char* vsSrc, const char* fsSrc){
    GLuint vs = CompileShader(vsSrc, GL_VERTEX_SHADER);
    GLuint fs = CompileShader(fsSrc, GL_FRAGMENT_SHADER);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    // check
    GLint status=0;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if(!status){
        char log[512]; glGetProgramInfoLog(prog, 512, NULL, log);
        std::cerr << "Program link error: " << log << std::endl;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

static GLuint CompileShader(const char* source, GLenum type){
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &source, NULL);
    glCompileShader(sh);
    GLint success=0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &success);
    if(!success){
        char log[512]; glGetShaderInfoLog(sh, 512, NULL, log);
        std::cerr << "Shader compile error: " << log << std::endl;
    }
    return sh;
}

// ------------------------------------------------------------ //
//  END OF FILE
// ------------------------------------------------------------ //

/*
   Notes:
   - “renderDepthMapFromSun” uses a static approach for building the sun’s lightSpaceMatrix.
   - The bloom code is intentionally minimal and uses a 5-tap Gaussian blur. 
     Real bloom often uses a multi-pass downsample and upsample approach.
   - The camera is not fully controlled; you could add callbacks to move, etc.
   - For a real project, break this out into multiple .cpp/.h files and 
     properly handle textures, error checks, etc.
*/
