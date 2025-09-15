#include "MaterialGenerator.h"

Material MaterialGenerator::RandomMaterial() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    // losowy kolor diffuse (jasnoœæ ograniczona, ¿eby nie by³o za ciemno)
    glm::vec3 diffuse = glm::vec3(
        0.2f + 0.8f * dist01(gen),   // R
        0.2f + 0.8f * dist01(gen),   // G
        0.2f + 0.8f * dist01(gen)    // B
    );

    glm::vec3 ambient = diffuse * 0.2f; // 20% intensywnoœci diffuse

    // specular (bardziej subtelne, ¿eby nie by³y jak lustro)
    glm::vec3 specular = glm::vec3(
        0.3f + 0.5f * dist01(gen),
        0.3f + 0.5f * dist01(gen),
        0.3f + 0.5f * dist01(gen)
    );

    // shininess – zakres np. 8–128
    float shininess = 8.0f + dist01(gen) * 120.0f;

    return { ambient, diffuse, specular, shininess };
}