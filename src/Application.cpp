#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Renderer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"

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
    -0.9f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
    -0.1f, -0.5f, 0.0f,

     0.1f, -0.5f, 0.0f,
     0.5f,  0.5f, 0.0f,
     0.9f, -0.5f, 0.0f
    };

    unsigned int indices[] = {
    0, 1, 2,   // lewy trójkąt
    3, 4, 5    // prawy trójkąt
    };

    VertexArray va;
    VertexBuffer vb(vertices, sizeof(vertices));
    IndexBuffer ib(indices, 6);
    va.AddBuffer(vb, 3);

    Shader shader("shaders/vertex.vert", "shaders/fragment.frag");
    shader.Bind();
	shader.SetUniform4f("uColor", 1.0f, 0.0f, 0.0f, 1.0f); // Ustaw kolor na czerwony

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
