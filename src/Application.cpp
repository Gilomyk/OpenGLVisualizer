#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Renderer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "GL/Mesh.h"

#include "GL/SphereGen.h"

#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/quaternion.hpp>   // glm::quat, glm::radians -> quaternion
#include <glm/gtx/quaternion.hpp>   // glm::toMat4

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"
#include "Core/Camera.h"
#include "Scene/Planet.h"

#define WIDTH 800
#define HEIGHT 600


// Callback do zmiany rozmiaru okna
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

//void DrawObject(const glm::vec3& translation, const glm::vec3& scale,
//    Shader& shader, Renderer& renderer,
//    VertexArray& va, IndexBuffer& ib, const Camera& camera)
//{
//    glm::mat4 model = glm::mat4(1.0f);
//    model = glm::translate(model, translation);
//    model = glm::scale(model, scale);
//
//    glm::mat4 mvp = camera.GetProjectionMatrix() * camera.GetViewMatrix() * model;
//
//    shader.Bind();
//    shader.SetUniformMat4f("u_MVP", mvp);
//    renderer.Draw(va, ib, shader);
//}


int main() {

	// 🔧 Inicjalizacja GLFW i GLAD
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "VAO Triangle", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD init failed\n";
        return -1;
    }

    // Włączenie głębi
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

	// Wyłączenie tylnych ścian
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW); // Ustawienie kierunku zgodnego z ruchem wskazówek zegara jako przód

    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Ustawienie funkcji mieszania dla przezroczystości
    GLCall(glEnable(GL_BLEND)); // Włączenie mieszania kolorów

	// 1. obiekt - sfera
    Texture earthTex("res/textures/earth.png");

    Planet earth(30.0f, 50, 50, &earthTex);

    //SphereGen sphereA(30.0f, 20, 20);

    //VertexBufferLayout layoutA;
    //layoutA.Push<float>(3); // 3 floaty — pozycja (x,y,z)
    //layoutA.Push<float>(3); // 3 floaty — normalne (nx, ny, nz)
    //layoutA.Push<float>(2); // 2 floaty — tekstura (u,v)

    //Mesh sphereMeshA(
    //    sphereA.GetVertices().data(),
    //    static_cast<unsigned int>(sphereA.GetVertices().size()),
    //    sphereA.GetIndices().data(),
    //    static_cast<unsigned int>(sphereA.GetIndices().size()),
    //    layoutA,
    //    &earthTex
    //);


	// 📐 Ustawienia kamery
    Camera camera(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);

    camera.SetPosition(glm::vec3(0.0f, 0.0f, 200.0f));
    camera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

	// Shader i renderer
    Shader shader("shaders/vertex.vert", "shaders/fragment.frag");
    Shader sphereShader("shaders/sphereVertex.vert", "shaders/sphereFragment.frag");
	Renderer renderer;

    // Inicjalizacja GUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Włącz obsługę klawiatury
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Włącz obsługę gamepada

	ImGui::StyleColorsDark(); // Ustaw ciemny motyw

    ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330"); // Użyj wersji OpenGL 3.3

	ImGui::GetIO().FontGlobalScale = 1.0f; // Ustaw skalę czcionki na 1.0f

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glm::vec3 translationA(0.0f, 0.0f, 0.0f);
    glm::vec3 translationB(0.0f, 0.0f, 0.0f);

    glm::vec3 scaleA(1.0f, 1.0f, 1.0f);
    glm::vec3 scaleB(1.0f, 1.0f, 1.0f);

	glm::vec3 rotationA(0.0f, 0.0f, 0.0f);
	glm::vec3 rotationB(0.0f, 0.0f, 0.0f);


    float r = 0.0f;

    // 🔁 Pętla renderująca
    while (!glfwWindowShouldClose(window)) {
		// Czyszczenie ekranu
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Sphere uniform
        //{
        //    glm::mat4 model = glm::mat4(1.0f); // macierz jednostkowa
        //    model = glm::translate(model, translationB);
        //    model = glm::scale(model, scaleB); // zmniejsza 2x

        //    glm::quat q = glm::quat(glm::radians(rotationB));
        //    model *= glm::toMat4(q);

        //    glm::mat4 mvp = camera.GetProjectionMatrix() * camera.GetViewMatrix() * model;

        //    sphereShader.Bind();
        //    sphereShader.SetUniformMat4f("u_MVP", mvp);

        //    renderer.Draw(sphereMeshA, sphereShader);
        //}

        // Sphere uniform with lightning
        /*{
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, translationB);
            model = glm::scale(model, scaleB);
            model *= glm::toMat4(glm::quat(glm::radians(rotationB)));

            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

            shader.Bind();
            shader.SetUniformMat4f("uModel", model);
            shader.SetUniformMat4f("uView", camera.GetViewMatrix());
            shader.SetUniformMat4f("uProjection", camera.GetProjectionMatrix());
            shader.SetUniformMat3f("uNormalMatrix", normalMatrix);

            shader.SetUniform3fv("uLightPos", glm::vec3(100.0f, 100.0f, 100.0f));
            shader.SetUniform3fv("uLightAmbient", glm::vec3(0.2f, 0.2f, 0.2f));
            shader.SetUniform3fv("uLightDiffuse", glm::vec3(0.7f, 0.7f, 0.7f));
            shader.SetUniform3fv("uLightSpecular", glm::vec3(1.0f, 1.0f, 1.0f));

            shader.SetUniform3fv("uViewPos", camera.GetPosition());
            shader.SetUniform1f("uShininess", 32.0f);

            renderer.Draw(sphereMeshA, sphereShader);
        }*/


        earth.SetPosition(translationB);
        earth.SetScale(scaleB);
        earth.SetRotation(rotationB);

        earth.Draw(sphereShader, renderer, camera);

        if (r >= 1.0) r = 0.0;
        r += 0.05f;

        {
            static float f = 0.0f;

            ImGui::SliderFloat3("Translation A", &translationA.r, 0.0f, (float)WIDTH);
            ImGui::SliderFloat3("Translation B", &translationB.r, 0.0f, (float)WIDTH);

			ImGui::SliderFloat3("Scale A", &scaleA.r, 0.1f, 10.0f);
			ImGui::SliderFloat3("Scale B", &scaleB.r, 0.1f, 10.0f);

			ImGui::SliderFloat3("Rotation A", &rotationA.r, 0.0f, 360.0f);
			ImGui::SliderFloat3("Rotation B", &rotationB.r, 0.0f, 360.0f);

			ImGui::Checkbox("Demo Window", &show_demo_window); // Przełącznik okna demo
			ImGui::Checkbox("Another Window", &show_another_window); // Przełącznik innego okna

			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edytor koloru

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
