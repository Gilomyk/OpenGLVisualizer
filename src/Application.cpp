#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>

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
#include "Scene/Stars.h"
#include "MaterialGenerator.h"
#include "Audio/AudioMapper.h"
#include "Audio/GUIControlPanel.h"

#define WIDTH 1920
#define HEIGHT 1080

// Ustawienia kamery
Camera camera(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 1000.0f);
bool cameraMode = false;

// Callback do zmiany rozmiaru okna
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// funkcja do pobierania zmiany czasu
float getDeltaTime() {
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float delta = float(currentTime - lastTime);
    lastTime = currentTime;
    if (delta < 0.01f) delta = 0.01f;
    if (delta > 0.033f) delta = 0.033f;
    return delta;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!cameraMode)
        return;

    static float lastX = 400, lastY = 300;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // odwrócone, bo OpenGL ma odwrotny Y

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!cameraMode)
        return;

    camera.ProcessMouseScroll((float)yoffset);
}

// funkcja do przetwarzania wejścia z klawiatury
void processInput(GLFWwindow* window) {
    float deltaTime = getDeltaTime();

    if (cameraMode) {
        // ruch kamery WSAD + SPACJA + SHIFT
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UPWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWNWARD, deltaTime);

        // centrowanie na (0,0,0)
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            camera.CenterOn(glm::vec3(0.0f));
        }
    }

    // blokada kursora myszy
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        cameraMode = false;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        cameraMode = true;
    }
}

// Funkcje generowania losowych liczb
// dyskretna z przedziału
float randf_step(float min, float max, float step) {
    static std::mt19937 rng(std::random_device{}());
    int count = static_cast<int>((max - min) / step) + 1;
    std::uniform_int_distribution<int> dist(0, count - 1);
    return min + dist(rng) * step;
}

// rzeczywista z przedziału (ale z zaokrągleniem)
float randf(float min, float max, int decimals = 2) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(min, max);

    float val = dist(rng);
    float factor = std::pow(10.0f, decimals);
    return std::round(val * factor) / factor;
}

std::vector<std::unique_ptr<Planet>> GenerateSystem(Texture* sunTexture, const AudioFrame& frame) {
    std::vector<std::unique_ptr<Planet>> planets;

    // Centralna gwiazda
    auto sun = std::make_unique<Planet>(
        50.0f, 50, 50,     // rozmiar sfery
        0.0f, glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f,  // brak orbity, brak prędkości
        nullptr,
        false              // bez orbity
    );
    sun->SetTexture(sunTexture);
	planets.push_back(std::move(sun));
    Planet* sunPtr = planets.back().get();

    // Liczba planet 2–5
    int numPlanets = (int)randf(2, 5);

    std::vector<std::pair<std::string, float>> bands = {
        {"sub_bass", frame.bands.sub_bass},
        {"bass", frame.bands.bass},
        {"low_mid", frame.bands.low_mid},
        {"mid", frame.bands.mid},
        {"high_mid", frame.bands.high_mid},
        {"presence", frame.bands.presence},
        {"brilliance", frame.bands.brilliance},
        {"air", frame.bands.air}
    };

    float baseOrbit = 80.0f;
    for (size_t i = 0; i < bands.size(); ++i) {
        float amplitude = bands[i].second; 

        // Rozmiar i odległość zależne od pasma
        float radius = 10.0f + amplitude * 0.002f;
        float orbitRadius = baseOrbit + i * 50.0f + amplitude * 0.001f;
        float orbitSpeed = 10.0f + 40.0f * (1.0f - (float)i / bands.size());
        float spinSpeed = randf(-100.0f, 100.0f);
        glm::vec3 tilt(randf(-10, 10), randf(0, 360), randf(-5, 5));
        Material material = MaterialGenerator::RandomMaterial();

        auto planet = std::make_unique<Planet>(
            radius, 50, 50, orbitRadius, tilt, orbitSpeed, spinSpeed, sunPtr, true
        );
        planet->SetMaterial(material);
        planets.push_back(std::move(planet));
    }

	return planets;
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

    // Ustawienia wejścia
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Włączenie głębi
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

	// Wyłączenie tylnych ścian
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW); // Ustawienie kierunku zgodnego z ruchem wskazówek zegara jako przód

    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Ustawienie funkcji mieszania dla przezroczystości
    GLCall(glEnable(GL_BLEND)); // Włączenie mieszania kolorów


    // Ładowanie parametrów audio
    AudioMapper audio;
    audio.LoadFromJSON("data/analysis_full.json");
	AudioFrame currentFrame = audio.GetFrame(0);

	// Planety - materiały
    Texture earthDiffuse("res/textures/earth_diff.png");
    Texture earthSpecular("res/textures/earth_spec.png");

    Texture sunDiffuse("res/textures/sun_diff.png");
    Texture sunSpecular("res/textures/sun_spec.png");

	// Generowanie układu słonecznego
    auto planets = GenerateSystem(&sunDiffuse, currentFrame);

    // Generowanie gwiazd
    Stars stars(2000, 500.0f);

	// Shader i renderer
    Shader sunShader("shaders/unlit_emissive.vert", "shaders/unlit_emissive.frag");
    Shader planetShader("shaders/sphereVertex.vert", "shaders/sphereFragment.frag");
	Shader orbitShader("shaders/orbit.vert", "shaders/orbit.frag");
    Shader trailShader("shaders/trail.vert", "shaders/trail.frag");
    Shader starShader("shaders/stars.vert", "shaders/stars.frag");
	Renderer renderer;

    // Inicjalizacja shaderów
    planetShader.Bind();
    planetShader.SetUniform1i("uDiffuseMap", 0);
    planetShader.SetUniform1i("uSpecularMap", 1);

    sunShader.Bind();
    sunShader.SetUniform1i("uDiffuseMap", 0);

	// Panel kontrolny GUI dla audio
    GUIControlPanel audioGUI;

    // Inicjalizacja GUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Włącz obsługę klawiatury
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Włącz obsługę gamepada
    io.WantCaptureMouse = true;

	ImGui::StyleColorsDark(); // Ustaw ciemny motyw

    ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330"); // Użyj wersji OpenGL 3.3

	ImGui::GetIO().FontGlobalScale = 1.0f; // Ustaw skalę czcionki na 1.0f

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Skalowanie XY
    static float earthScale = 1.0f;
	static float sunScale = 1.0f;

	// Indeks klatki audio
    static int frameIndex = 0;

    // 🔁 Pętla renderująca
    while (!glfwWindowShouldClose(window)) {
        float dt = getDeltaTime();

        processInput(window);
        camera.Update(dt);

		// Czyszczenie ekranu
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // GUI
        {
            ImGui::Begin("Debug Panel");
			ImGui::SliderFloat("Sun Scale", &sunScale, 0.1f, 10.0f);

			// Ustawienia kamery
			ImGui::Text("Camera Settings:");
			ImGui::Checkbox("Enable Acceleration", &camera.enableAcceleration);
			ImGui::SliderFloat("Acceleration", &camera.acceleration, 100.0f, 1000.0f);
			ImGui::SliderFloat("Max Speed", &camera.maxSpeed, 100.0f, 2000.0f);
			ImGui::SliderFloat("Mouse Sensitivity", &camera.MouseSensitivity, 0.01f, 1.0f);


            ImGui::Checkbox("Demo Window", &show_demo_window); // Przełącznik okna demo
            ImGui::Checkbox("Another Window", &show_another_window); // Przełącznik innego okna

            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edytor koloru

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();

            audioGUI.DrawImGUI();
        }

		// Test parametrów za pomocą GUI

        std::vector<std::pair<std::string, float>> newBands = {
            {"sub_bass", audioGUI.bands.sub_bass},
            {"bass", audioGUI.bands.bass},
            {"low_mid", audioGUI.bands.low_mid},
            {"mid", audioGUI.bands.mid},
            {"high_mid", audioGUI.bands.high_mid},
            {"presence", audioGUI.bands.presence},
            {"brilliance", audioGUI.bands.brilliance},
            {"air", audioGUI.bands.air}
        };

        for (size_t i = 1; i < planets.size(); ++i) {
            float newAmplitude = newBands[i-1].second;

            // Rozmiar i odległość zależne od pasma
            float scale = 1.0f + newAmplitude * 0.001f;
            float orbitRadius = 80.0f + i * 50.0f + newAmplitude * 0.002f;

			planets[i]->SetScale(glm::vec3(scale));
			planets[i]->SetOrbitRadius(orbitRadius);
		}

		// Animacja
		// Cały układ słoneczny
        for (auto& planet : planets) {
            planet->Update(dt);

			if (planet.get() == planets[0].get()) { // słońce
				glm::vec3 scaleB(sunScale, sunScale, sunScale);
				planet->SetScale(scaleB);
				planet->DrawSun(sunShader, renderer, camera);
				continue;
			}

            planet->DrawPlanet(planetShader, renderer, camera);

            if (planet->GetOrbit())
                planet->GetOrbit()->DrawOrbit(orbitShader, renderer, camera, planet->GetParent() ? planet->GetParent()->GetPosition() : glm::vec3(0.0f));

            //if (planet->GetTrail())
            //    planet->GetTrail()->Draw(trailShader, renderer, camera, true); // true = linia
        }
        stars.Draw(starShader, camera);

        // Gwiazdy
        
		// Debug print co sekundę
        /*static float debugTimer = 0.0f;
        debugTimer += dt;
        if (debugTimer > 1.0f) {
            for (auto& planet : planets) {
                planet->DebugPrint();
            }
            debugTimer = 0.0f;
        }*/

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
