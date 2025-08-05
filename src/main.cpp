#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

std::string loadShaderSource(const char* filepath) {
    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int compileShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type);  
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Błąd?
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, NULL, info);
        std::cerr << "Shader error: " << info << std::endl;
    }
    return shader;
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

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);  // 🔧 Konfiguracja VAO

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // Połącz VBO
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 

    // aPos – location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 🔄 Odłącz VBO i VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 🧠 Shadery
    std::string vShaderCode = loadShaderSource("shaders/vertex.vert");
    std::string fShaderCode = loadShaderSource("shaders/fragment.frag");
    unsigned int vertexShader = compileShader(vShaderCode.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fShaderCode.c_str(), GL_FRAGMENT_SHADER);

    // 🛠 Program shaderowy
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    float r = 0.0f;

    // Zwolnij shadery po linkowaniu
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // 🔁 Pętla renderująca
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        int colorLoc = glGetUniformLocation(shaderProgram, "uColor");
        glUniform3f(colorLoc, r, 1.0f, 0.0f);

        glBindVertexArray(VAO);  // 🟢 Aktywuj VAO z VBO + konfiguracją
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (r >= 1.0) r = 0.0;
        r += 0.05f;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 🧹 Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
