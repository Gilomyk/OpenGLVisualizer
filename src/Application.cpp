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

// Callback do zmiany rozmiaru okna
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "VAO Triangle", NULL, NULL);
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

    // 🎨 Wierzchołki + kolory
    float vertices[] = {
    -0.5f, -0.5f, 0.0f, 0.0f,
    -0.5f,  0.5f, 0.0f, 1.0f,
     0.5f, -0.5f, 1.0f, 0.0f,
     0.5f, 0.5f, 1.0f, 1.0f
    };

    unsigned int indices[] = {
    0, 1, 2,   // lewy trójkąt
    1, 2, 3    // prawy trójkąt
    };

	GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); // Ustawienie funkcji mieszania dla przezroczystości
	GLCall(glEnable(GL_BLEND)); // Włączenie mieszania kolorów

    VertexArray va;
    VertexBuffer vb(vertices, sizeof(vertices));
    IndexBuffer ib(indices, 6);
    
    VertexBufferLayout layout;
    layout.Push<float>(2);  // 2 floaty — pozycja (x,y)
	layout.Push<float>(2); // 2 floaty — tekstura (u,v)

    va.AddBuffer(vb, layout);

    Shader shader("shaders/vertex.vert", "shaders/fragment.frag");
    shader.Bind();
	shader.SetUniform4f("uColor", 1.0f, 0.0f, 0.0f, 1.0f); // Ustaw kolor na czerwony

	Texture texture("res/textures/bed.png");
	texture.Bind(); // Zwiąż teksturę, jeśli jest używana w shaderze

	// Ustaw uniform dla tekstury, jeśli jest używana
	shader.SetUniform1i("uTexture", 0); // Zakładając, że tekstura jest w samym shaderze

	va.Unbind();
	ib.Unbind();  // 🟢 Odłącz VAO, aby uniknąć przypadkowych zmian
	shader.Unbind(); // Odłącz shader, aby uniknąć przypadkowych zmian

	Renderer renderer;

    float r = 0.0f;

    // 🔁 Pętla renderująca
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

		shader.Bind();
		shader.SetUniform4f("uColor", r, 0.0f, 1.0f - r, 1.0f); // Zmieniaj kolor w czasie

		renderer.Clear();
		renderer.Draw(va, ib, shader);

        if (r >= 1.0) r = 0.0;
        r += 0.05f;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
