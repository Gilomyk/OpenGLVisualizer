#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>

#include <windows.h>
#include <Mmsystem.h>
//these two headers are already included in the <Windows.h> header
#pragma comment(lib, "Winmm.lib")

#include "Graphics/Renderer.h"
#include "Graphics/Buffers/VertexArray.h"
#include "Graphics/Buffers/VertexBuffer.h"
#include "Graphics/Buffers/VertexBufferLayout.h"
#include "Graphics/Buffers/IndexBuffer.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture.h"
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
#include "Graphics/MaterialGenerator.h"
#include "Audio/AudioMapper.h"
#include "Audio/GUIControlPanel.h"
#include "Audio/Envelope.h"
#include "Core/CameraController.h"
#include <glm/gtc/type_ptr.hpp>

#define WIDTH 1920
#define HEIGHT 1080
#define AUDIO_FRAMERATE 40

// Ustawienia kamery i kontrolera kamery
Camera camera(45.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 2000.0f);
CameraController camController(&camera);
bool cameraMode = false;
bool firstMouse = true;
float lastX = 400, lastY = 300;

bool testGUIMode = true;
bool enableOrbit = false;
float startTime = 0.0f;
float renderTime = 0.0f;

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
        firstMouse = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        cameraMode = true;
        firstMouse = true;
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

std::vector<std::unique_ptr<Planet>> GenerateSystem(Texture* sunTexture, Texture* earthTexture, const AudioFrame& frame) {
    std::vector<std::unique_ptr<Planet>> planets;

    // Centralna gwiazda
    auto sun = std::make_unique<Planet>(
        50.0f, 50, 50,     // rozmiar sfery
        0.0f, glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f,  // brak orbity, brak prędkości
        nullptr,
        false              // bez orbity
    );
	sun->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));
    sun->SetTexture(sunTexture);
	planets.push_back(std::move(sun));
    Planet* sunPtr = planets.back().get();

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
		Material baseMaterial = material;

        auto planet = std::make_unique<Planet>(
            radius, 50, 50, orbitRadius, tilt, orbitSpeed, spinSpeed, sunPtr, enableOrbit
        );
        planet->SetTexture(earthTexture); // brak tekstury na razie
        planet->SetMaterial(material);
		planet->SetBaseDiffuse(material.diffuse);
		planet->SetBaseShininess(material.shininess);
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "AudioPlanetVisualizer", NULL, NULL);
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

	glfwSwapInterval(1); // vsync

	// Włączenie renderowania punktów w shaderze
	GLCall(glEnable(GL_PROGRAM_POINT_SIZE));

    // Włączenie głębi
	glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

	// Wyłączenie tylnych ścian
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW); // Ustawienie kierunku zgodnego z ruchem wskazówek zegara jako przód

    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Ustawienie funkcji mieszania dla przezroczystości
    GLCall(glEnable(GL_BLEND)); // Włączenie mieszania kolorów

	// Inicjalizacja audio
    AudioMapper audio;
    AudioFrame currentFrame = AudioFrame();

    // Panel kontrolny GUI dla audio
    GUIControlPanel audioGUI;

    // Ładowanie parametrów audio
    if (!testGUIMode) {
        std::cout << "Audio reactive mode enabled.\n";
        audio.LoadFromJSON("data/analysis_Tchaikovsky-Waltz-of-the-Flowers_rms_weighted.json");
        currentFrame = audio.GetFrame(0);
    }
    else {
        std::cout << "Test GUI mode enabled.\n";
        audio.LoadFromJSON("data/analysis_TEST_rms_weighted", true);
		audioGUI.setBandRanges(audio.GetBandStats());
    }

	// Envelopy dla zdarzeń rytmicznych
    Envelope beatEnv(0.05f, 0.1f, 0.6f, 0.3f);
    Envelope onsetEnv(0.02f, 0.05f, 0.5f, 0.2f);


	// Planety - materiały
    Texture earth("res/textures/earth_diff.png");
    Texture earthSpecular("res/textures/earth_spec.png");

    Texture sun("res/textures/sun_diff.png");
    Texture sunSpecular("res/textures/sun_spec.png");

	// Generowanie układu słonecznego
    auto planets = GenerateSystem(&sun, &earth, currentFrame);

    // Generowanie gwiazd
    Stars stars(2000, 750.0f);

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


	// Indeks klatki audio
    static int frameIndex = 0;

	// Parametry regulowane przez GUI


    // Parametry światła słonecznego zależne od audio
    static float sunBaseScale = 1.0f;       // podstawowy rozmiar słońca
	static float sunScaleSensitivity = 1.0f;  // czułość skalowania słońca
	static float planetBaseScale = 1.0f;    // podstawowy rozmiar innych planet
	static float planetScaleSensitivity = 1.0f;           // czułość skalowania innych planet
    static float lightBaseIntensity = 1.0f;  // intensywność światła
	static float textureBlend = 1.0f;        // mieszanie tekstur
	static float cameraSpeedIntensity = 1.0f;         // prędkość kamery

	// Odtwarzanie dźwięku
    if (testGUIMode == false) {
        // WAV
        //PlaySound(TEXT("res/audio/GWIAZDY.wav"), NULL, SND_FILENAME | SND_ASYNC);

		// MP3
        mciSendString(L"open \"res/audio/Tchaikovsky-Waltz-of-the-Flowers.mp3\" type mpegvideo alias music", NULL, 0, NULL);
        mciSendString(L"play music", NULL, 0, NULL);
    }

    // 🔁 Pętla renderująca
    while (!glfwWindowShouldClose(window)) {
		if (startTime == 0.0f)
			startTime = glfwGetTime();
		renderTime = glfwGetTime() - startTime;

        float dt = getDeltaTime();

        // Aktualizacja kamery

		float localTempoVal = audio.MapValue(AudioVisualParam::CAMERA_SPEED, frameIndex);
		float cameraSpeed = localTempoVal * cameraSpeedIntensity * 500.0;

        processInput(window);
        if (!cameraMode) {
            if (!testGUIMode) {
                camController.Update(dt, planets, cameraSpeed);
            }
        }
        else {
            camera.Update(dt);
        }

        // === OBLICZENIE KOLORU TŁA ===
        float atmosphereAlphaV = audio.MapValue(AudioVisualParam::ATMOSPHERE_ALPHA, frameIndex);
        glm::vec3 darkSpace = glm::vec3(0.00f, 0.00f, 0.00f);
        glm::vec3 brightSky = glm::vec3(0.10f, 0.13f, 0.21f);
		glm::vec3 bgColor = glm::mix(darkSpace, brightSky, atmosphereAlphaV * 2.0); // (ATMOSPHERE_ALPHA)

        // === CZYSZCZENIE BUFORÓW ===
        glClearColor(bgColor.r, bgColor.g, bgColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.Clear();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // GUI
        {
            ImGui::Begin("Debug Panel");

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

			ImGui::Separator();
			ImGui::Text("Audio Frame Index: %d / %d", frameIndex, audio.GetFrameCount());
			ImGui::Text("RMS: %.3f", currentFrame.rms);
			ImGui::Text("Bands:");
			ImGui::Text(" Sub Bass: %.3f", currentFrame.bands.sub_bass);
			ImGui::Text(" Bass: %.3f", currentFrame.bands.bass);
			ImGui::Text(" Low Mid: %.3f", currentFrame.bands.low_mid);
			ImGui::Text(" Mid: %.3f", currentFrame.bands.mid);
			ImGui::Text(" High Mid: %.3f", currentFrame.bands.high_mid);
			ImGui::Text(" Presence: %.3f", currentFrame.bands.presence);
			ImGui::Text(" Brilliance: %.3f", currentFrame.bands.brilliance);
			ImGui::Text(" Air: %.3f", currentFrame.bands.air);
			ImGui::Separator();
			ImGui::Text(" Is Onset: %s", currentFrame.is_onset ? "Yes" : "No");
			ImGui::Text(" Onset Level: %.3f", onsetEnv.GetValue());
			ImGui::Text(" Onset State: %d", (int)onsetEnv.GetState());
			ImGui::Text(" Is Beat: %s", currentFrame.is_beat ? "Yes" : "No");
			ImGui::Text(" Beat Level: %.3f", beatEnv.GetValue());
			ImGui::Text(" Beat State: %d", (int)beatEnv.GetState());
            ImGui::End();

            if (testGUIMode) {
                audioGUI.DrawImGUI();
            }

			// Kontrola parametrów globalnych
			ImGui::Begin("Global Parameters");
            ImGui::SliderFloat("Sun Base Scale", &sunBaseScale, 0.1f, 5.0f);
			ImGui::SliderFloat("Sun Scale Sensitivity", &sunScaleSensitivity, 0.0f, 20.0f);
			ImGui::SliderFloat("Planet Base Scale", &planetBaseScale, 0.1f, 5.0f);
            ImGui::SliderFloat("Scale Sensitivity", &planetScaleSensitivity, 0.0f, 20.0f);
            ImGui::SliderFloat("Light Base Intensity", &lightBaseIntensity, 0.05f, 1.0f);
			ImGui::SliderFloat("Camera Speed Intensity", &cameraSpeedIntensity, 0.0f, 5.0f);
            ImGui::SliderFloat("Texture Blend", &textureBlend, 0.0f, 1.0f);
            beatEnv.ImGuiControls("Beat Envelope");
            onsetEnv.ImGuiControls("Onset Envelope");
			ImGui::End();

            //ImGui::Begin("Parameter control");
            //ImGui::SliderFloat("Scale Sensitivity", &scaleSensitivity, 0.0f, 20.0f);
            //ImGui::Separator();
            //ImGui::Text("Global Light:");
            ////ImGui::Text("Sun Scale Dynamic: %.2f", sunScaleDynamic);
            //ImGui::End();

        }

		// Test parametrów za pomocą GUI
		if (testGUIMode) {
            frameIndex = 0;
			currentFrame.rms = audioGUI.rms;
			currentFrame.bands.sub_bass = audioGUI.bands.sub_bass;
			currentFrame.bands.bass = audioGUI.bands.bass;
			currentFrame.bands.low_mid = audioGUI.bands.low_mid;
			currentFrame.bands.mid = audioGUI.bands.mid;
			currentFrame.bands.high_mid = audioGUI.bands.high_mid;
			currentFrame.bands.presence = audioGUI.bands.presence;
			currentFrame.bands.brilliance = audioGUI.bands.brilliance;
            currentFrame.bands.air = audioGUI.bands.air;
            currentFrame.is_onset = audioGUI.is_onset;
			currentFrame.is_beat = audioGUI.is_beat;
            
			audio.UpdateFrameDirectly(currentFrame);
            audio.UpdateSmoothedBands(frameIndex);
        }
        // Aktualizacja klatki audio na podstawie czasu
        else {
            frameIndex = (int)(renderTime * AUDIO_FRAMERATE);
            if (frameIndex >= audio.GetFrameCount())
                frameIndex = 0;
			currentFrame = audio.GetFrame(frameIndex); // for GUI display

            audio.UpdateSmoothedBands(frameIndex);
        }


        // Pobieranie zmapowanych wartości audio
        float sunEmissionVal = audio.MapValue(AudioVisualParam::SUN_EMISSION, frameIndex);
        float sunScaleVal = audio.MapValue(AudioVisualParam::SUN_SCALE, frameIndex);
        float orbitShakeVal = audio.MapValue(AudioVisualParam::ORBIT_SHAKE, frameIndex);
		float planetColorLowMidVal = audio.MapValue(AudioVisualParam::PLANET_COLOR_LOW_MID, frameIndex);
        float planetColorMidVal = audio.MapValue(AudioVisualParam::PLANET_COLOR_MID, frameIndex);
        float specularVal = audio.MapValue(AudioVisualParam::SPECULAR_INTENSITY, frameIndex);
        float noiseAmountVal = audio.MapValue(AudioVisualParam::NOISE_AMOUNT, frameIndex);
		float starFlickerVal = audio.MapValue(AudioVisualParam::STAR_FLICKER, frameIndex);
		// float atmosphereAlphaV = audio.MapValue(AudioVisualParam::ATMOSPHERE_ALPHA, frameIndex); // już pobrane wcześniej
		float onsetVal = audio.MapValue(AudioVisualParam::ONSET_FLASH, frameIndex);
		float beatVal = audio.MapValue(AudioVisualParam::BEAT_INTENSITY, frameIndex);

		// użycie globalnego współczynnik światła
		sunEmissionVal *= lightBaseIntensity;
		specularVal *= lightBaseIntensity;
		starFlickerVal *= lightBaseIntensity;
        
		// korzystanie z envelop do wygładzania zdarzeń rytmicznych
        beatEnv.Trigger(beatVal);
        onsetEnv.Trigger(onsetVal);
		beatEnv.Update(dt);
		onsetEnv.Update(dt);
		float beatLevel = beatEnv.GetValue();
		float onsetLevel = onsetEnv.GetValue();


		// Aktualizacja słońca

        {
            // skala słońca (PLANET_SCALE + BEAT)
            if (beatLevel > 0.0f) {
                sunScaleVal *= 1.0f + beatLevel * 0.25f * sunScaleSensitivity; // max +25%
            }
            glm::vec3 sunScaleVec(sunScaleVal * sunBaseScale);
            planets[0]->SetScale(sunScaleVec);

            // Inne parametry
            sunShader.Bind();

            glm::vec3 baseColor = glm::mix(
                glm::vec3(0.7f, 0.6f, 0.7f), // zimny przy niskim emisji
                glm::vec3(1.0f, 0.7f, 0.4f), // ciepły przy wysokiej emisji
                sunEmissionVal
            );

            sunShader.SetUniform3fv("uBaseColor", baseColor);
            sunShader.SetUniform1f("uNoiseAmount", noiseAmountVal); // NOISE_AMOUNT
			sunShader.SetUniform1f("uFlickerStrength", starFlickerVal); // STAR_FLICKER
            sunShader.SetUniform1f("uAtmosphereAlpha", atmosphereAlphaV); // ATMOSPHERE_ALPHA
            sunShader.SetUniform1f("uTextureBlend", textureBlend); // tekstura vs kolor
        }

		// --- Aktualizacja planet ---

        for (size_t i = 1; i < planets.size(); ++i) {

			// Skala planet na podstawie pasma audio (MapBandForPlanet)
            float bandMapped = audio.MapBandForPlanet(frameIndex, i - 1);

            float scale = 1.0f * planetBaseScale + bandMapped * planetScaleSensitivity;
            float baseOrbit = 80.0f + i * 50.0f;
            float orbitRadius = baseOrbit + bandMapped * (planetScaleSensitivity * 10.0f);

            planets[i]->SetScale(glm::vec3(scale));
            planets[i]->SetOrbitRadius(orbitRadius);

			// Drgania orbity (ORBIT_SHAKE)
            glm::vec3 baseTilt = planets[i]->GetBaseOrbitTilt(); // dodaj taką metodę/atrybut albo trzymaj wartość
            float shakeAmp = 15.0f;
            if (beatVal > 0.5f) {
                shakeAmp *= 1.3f;
            }
            glm::vec3 newTilt = baseTilt + glm::vec3(orbitShakeVal * shakeAmp, 0.0f, 0.0f);

            planets[i]->SetOrbitTilt(newTilt);

			// Kolor planet (PLANET_COLOR)
			Material matBase = planets[i]->GetBaseMaterial(); // niezmieniony materiał bazowy
			Material mat = planets[i]->GetMaterial(); // materiał do modyfikacji

            glm::vec3 baseDiffuse = planets[i]->GetBaseDiffuse();
			float baseShininess = planets[i]->GetBaseShininess();

            glm::vec3 ambientShift = glm::mix(
                glm::vec3(0.5f, 0.5f, 0.6f),
                glm::vec3(0.9f, 0.85f, 0.7f),
                planetColorLowMidVal
            );

            glm::vec3 diffuseShift = glm::mix(
                glm::vec3(0.8f, 0.9f, 1.0f),
                glm::vec3(1.1f, 0.95f, 0.8f),
                planetColorMidVal
            );


            // ustawienia materiału
            mat.diffuse = baseDiffuse * diffuseShift;
            mat.ambient = baseDiffuse * 0.2f * ambientShift;
			mat.shininess = baseShininess;

            if (onsetLevel > 0.0f) {
                float intensity = 1.0f + onsetLevel * 0.2f; // do +20% jasności
                mat.diffuse = baseDiffuse * intensity;

                float shininessBoost = glm::mix(1.0f, 2.0f, onsetLevel); // do 2x większy połysk
                mat.shininess = baseShininess * shininessBoost;
            }

            planets[i]->SetMaterial(mat);


			// Inne parametry
			planetShader.Bind();
			planetShader.SetUniform1f("uSpecularScale", specularVal); // SPECULAR_INTENSITY
			planetShader.SetUniform1f("uNoiseAmount", noiseAmountVal); // NOISE_AMOUNT
			planetShader.SetUniform1f("uAtmosphereAlpha", atmosphereAlphaV); // ATMOSPHERE_ALPHA
			planetShader.SetUniform1f("uTextureBlend", textureBlend); // tekstura vs kolor
        }

		// Aktualizacja gwiazd (STAR_FLICKER)
        {
            starShader.Bind();
			starShader.SetUniform1f("uFlickerScale", starFlickerVal); // STAR_FLICKER
			starShader.SetUniform1f("uAtmosphereAlpha", atmosphereAlphaV); // ATMOSPHERE_ALPHA
        }


        // --- Rysowanie ---
        {
            for (auto& planet : planets) {
                planet->Update(dt);

                if (planet.get() == planets[0].get()) { // słońce
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
        }

		//float rms = audio.MapValue(AudioVisualParam::SUN_EMISSION, frameIndex);
  //      // Aktualizacja RMS z filtrem wygładzającym
  //      float alpha = 1.0f - exp(-dt / 0.2f); // tau = 0.2s
  //      rmsSmoothed += alpha * (rms - rmsSmoothed);

  //      // nieliniowa reakcja (mocniejszy puls)
  //      float rmsResponse = pow(newBands[1].second, 0.8f);

  //      // pulsujący rozmiar słońca
  //      float sunScaleDynamic = sunBaseScale * (1.0f + rmsResponse * 0.4f); // 40% max rozszerzenie

  //      // globalne światło
  //      float lightIntensityDynamic = lightBaseIntensity * (1.0f + rmsResponse * 0.8f);
        
		// Debug print co sekundę
        static float debugTimer = 0.0f;
        debugTimer += dt;
        if (debugTimer > 1.0f) {
			/*std::cout << "Frame Index: " << frameIndex << "\n";
			std::cout << "Sun Emission Value: " << sunEmissionVal << "\n";
			std::cout << "Sun Scale Value: " << sunScaleVal << "\n";
			std::cout << "Orbit Shake Value: " << orbitShakeVal << "\n";
			std::cout << "Planet Color Value: " << planetColorVal << "\n";
			std::cout << "Specular Intensity Value: " << specularVal << "\n";
			std::cout << "Noise Amount Value: " << noiseAmountVal << "\n";
			std::cout << "Atmosphere Alpha Value: " << atmosphereAlphaV << "\n";

            std::cout << "-------------------------" << "\n";*/

            /*std::cout << "Band stats loaded:\n";
            std::cout << "SubBass min=" << audio.GetBandStats().sub_bass_min
                << " max=" << audio.GetBandStats().sub_bass_max << "\n";
			std::cout << "Bass min=" << audio.GetBandStats().bass_min
				<< " max=" << audio.GetBandStats().bass_max << "\n";
			std::cout << "LowMid min=" << audio.GetBandStats().low_mid_min
				<< " max=" << audio.GetBandStats().low_mid_max << "\n";
			std::cout << "Mid min=" << audio.GetBandStats().mid_min
				<< " max=" << audio.GetBandStats().mid_max << "\n";
			std::cout << "HighMid min=" << audio.GetBandStats().high_mid_min
				<< " max=" << audio.GetBandStats().high_mid_max << "\n";
			std::cout << "Presence min=" << audio.GetBandStats().presence_min
				<< " max=" << audio.GetBandStats().presence_max << "\n";
			std::cout << "Brilliance min=" << audio.GetBandStats().brilliance_min
				<< " max=" << audio.GetBandStats().brilliance_max << "\n";
			std::cout << "Air min=" << audio.GetBandStats().air_min
				<< " max=" << audio.GetBandStats().air_max << "\n";

            std::cout << "-------------------------" << "\n";*/

			std::cout << "Smoothed bands:\n";
			std::cout << " Sub Bass: " << audio.GetSmoothedBandByType(Band::SUB_BASS) << "\n";
			std::cout << " Bass: " << audio.GetSmoothedBandByType(Band::BASS) << "\n";
			std::cout << " Low Mid: " << audio.GetSmoothedBandByType(Band::LOW_MID) << "\n";
			std::cout << " Mid: " << audio.GetSmoothedBandByType(Band::MID) << "\n";
			std::cout << " High Mid: " << audio.GetSmoothedBandByType(Band::HIGH_MID) << "\n";
			std::cout << " Presence: " << audio.GetSmoothedBandByType(Band::PRESENCE) << "\n";
			std::cout << " Brilliance: " << audio.GetSmoothedBandByType(Band::BRILLIANCE) << "\n";
			std::cout << " Air: " << audio.GetSmoothedBandByType(Band::AIR) << "\n";
            


			/*glm::vec3 baseColor = planets[1]->GetBaseMaterial().diffuse;
			glm::vec3 currentColor = planets[1]->GetMaterial().diffuse;
			std::cout << "Base Material Color: " << baseColor.r << ", " << baseColor.g << ", " << baseColor.b << "\n";
			std::cout << "Current Material Color: " << currentColor.r << ", " << currentColor.g << ", " << currentColor.b << "\n";

            std::cout << "-------------------------" << "\n";*/
            /*for (auto& planet : planets) {
				  planet->DebugPrint();
            }*/
            debugTimer = 0.0f;
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
