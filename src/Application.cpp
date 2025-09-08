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
#include "Scene/Orbit.h"

#define WIDTH 800
#define HEIGHT 600


// Callback do zmiany rozmiaru okna
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

float getDeltaTime() {
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float delta = float(currentTime - lastTime);
    lastTime = currentTime;
    return delta;
}

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

	// Planety
    Texture earthTex("res/textures/earth.png");
	Texture sunTex("res/textures/sun.png");

    float r = 120.0f; // promień orbity
    float omega = 50.0f; // prędkość kątowa Ziemi wokół Słońca
    float omega2 = -200.0f; // prędkość kątowa Ziemi wokół własnej osi
	glm::vec3 tilt1(15.0f, 0.0f, 0.0f); // nachylenie orbity Ziemi

	Planet sun(50.0f, 50, 50, 0.0f, glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f, nullptr, &sunTex);
    Planet earth(30.0f, 50, 50, r, tilt1, omega, omega2, &sun, &earthTex, true);

    std::vector<Planet*> planets;
    planets.push_back(&sun);
    planets.push_back(&earth);

	// 📐 Ustawienia kamery
    Camera camera(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);

	// Shader i renderer
    Shader sunShader("shaders/unlit_emissive.vert", "shaders/unlit_emissive.frag");
    Shader planetShader("shaders/sphereVertex.vert", "shaders/sphereFragment.frag");
	Shader orbitShader("shaders/orbit.vert", "shaders/orbit.frag");
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

    // Zmienne obrotu kamery (najlepsze pod widok)
    float camAlpha = 0.0f;   // kąt poziomy
    float camBeta = 0.4f;   // kąt pionowy
    float camR = 350.0f; // promień orbity

	// Skalowanie XY
    static float earthScale = 1.0f;
	static float sunScale = 1.0f;

    // 🔁 Pętla renderująca
    while (!glfwWindowShouldClose(window)) {
		// Czyszczenie ekranu
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // GUI
        {
            ImGui::SliderFloat("Earth Scale", &earthScale, 0.1f, 10.0f);
			ImGui::SliderFloat("Sun Scale", &sunScale, 0.1f, 10.0f);

			ImGui::SliderFloat("Camera Radius", &camR, 50.0f, 500.0f);
			ImGui::SliderFloat("Camera Alpha", &camAlpha, 0.0f, 2.0f * 3.14159f);
			ImGui::SliderFloat("Camera Beta", &camBeta, -1.5f, 1.5f);

            ImGui::Checkbox("Demo Window", &show_demo_window); // Przełącznik okna demo
            ImGui::Checkbox("Another Window", &show_another_window); // Przełącznik innego okna

            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edytor koloru

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        }

		// Animacja
        float dt = getDeltaTime();

        sun.DrawSun(sunShader, renderer, camera);
        for (auto& planet : planets) {
            if (planet != &sun)
                planet->DrawPlanet(planetShader, renderer, camera);
            if (planet->GetOrbit())
				planet->GetOrbit()->DrawOrbit(orbitShader, renderer, camera);
        }

        for (auto& planet : planets)
            planet->Update(dt);

		// Skalowanie planet
        glm::vec3 scale(earthScale, earthScale, earthScale);
        glm::vec3 scaleB(sunScale, sunScale, sunScale);

		earth.SetScale(scale);
        sun.SetScale(scaleB);

		// Aktualizacja pozycji kamery
        float x = camR * cos(camBeta) * cos(camAlpha);
        float y = camR * sin(camBeta);
        float z = camR * cos(camBeta) * sin(camAlpha);

        camera.SetPosition(glm::vec3(x, y, z));
        camera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

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
