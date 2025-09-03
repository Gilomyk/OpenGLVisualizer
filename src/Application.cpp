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

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"

#define WIDTH 800
#define HEIGHT 600

// Callback do zmiany rozmiaru okna
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
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

    // 🎨 Wierzchołki + kolory
    float vertices[] = {
    -50.0f, -50.0f, -90.0f, 0.0f, 0.0f,
    -50.0f,  50.0f, -90.0f, 0.0f, 4.0f,
     50.0f, -50.0f, -90.0f, 4.0f, 0.0f,
     50.0f, 50.0f, -90.0f, 4.0f, 4.0f
    };

    unsigned int indices[] = {
    0, 2, 1,   // zamiast 0,1,2
    1, 2, 3
    };

	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Ustawienie funkcji mieszania dla przezroczystości
	GLCall(glEnable(GL_BLEND)); // Włączenie mieszania kolorów

    VertexArray va;
    VertexBuffer vb(vertices, sizeof(vertices));
    IndexBuffer ib(indices, 6);
    
    VertexBufferLayout layout;
    layout.Push<float>(3);  // 3 floaty — pozycja (x,y, z)
	layout.Push<float>(2); // 2 floaty — tekstura (u,v)

    va.AddBuffer(vb, layout);

	//glm::mat4 proj = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f); // Prosta macierz ortograficzna
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f), // kąt FOV
        (float)WIDTH / (float)HEIGHT, // proporcje okna
        0.1f,  // near plane
        100.0f // far plane
    );

    //glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-100.0f, 0.0f, 0.0f));
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f), // pozycja kamery
        glm::vec3(0.0f, 0.0f, 0.0f), // punkt, na który patrzy
        glm::vec3(0.0f, 1.0f, 0.0f)  // wektor "up"
    );

    Shader shader("shaders/vertex.vert", "shaders/fragment.frag");
    shader.Bind();
	shader.SetUniform4f("uColor", 1.0f, 0.0f, 0.0f, 1.0f); // Ustaw kolor na czerwony

	Texture texture("res/textures/bed.png");
	texture.Bind(); // Zwiąż teksturę, jeśli jest używana w shaderze

	// Ustaw uniform dla tekstury, jeśli jest używana
	shader.SetUniform1i("uTexture", 0); // Zakładając, że tekstura jest w samym shaderze
	// Ustaw uniform dla macierzy projekcji

	va.Unbind();
	ib.Unbind();  // 🟢 Odłącz VAO, aby uniknąć przypadkowych zmian
	shader.Unbind(); // Odłącz shader, aby uniknąć przypadkowych zmian

	Renderer renderer;

 //   // Inicjalizacja GUI
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


        // Setting the first uniform
        {
            glm::mat4 model = glm::mat4(1.0f); // macierz jednostkowa
            model = glm::translate(model, translationA);
            model = glm::scale(model, glm::vec3(0.3f, 0.3f, 1.0f)); // zmniejsza 2x

            glm::mat4 mvp = proj * view * model;

            shader.Bind();
            shader.SetUniformMat4f("u_MVP", mvp);
            renderer.Draw(va, ib, shader);
        }
        
        {
            glm::mat4 model = glm::mat4(1.0f); // macierz jednostkowa
            model = glm::translate(model, translationB);
            model = glm::scale(model, glm::vec3(0.3f, 0.3f, 1.0f)); // zmniejsza 2x

            glm::mat4 mvp = proj * view * model;

            shader.Bind();
            shader.SetUniformMat4f("u_MVP", mvp);
            renderer.Draw(va, ib, shader);
        }

        if (r >= 1.0) r = 0.0;
        r += 0.05f;

        /*if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);*/


        {
            static float f = 0.0f;

            ImGui::SliderFloat3("Translation A", &translationA.r, 0.0f, (float)WIDTH);
            ImGui::SliderFloat3("Translation B", &translationB.r, 0.0f, (float)WIDTH);
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
