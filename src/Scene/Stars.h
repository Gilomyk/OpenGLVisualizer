#pragma once
#include <glm/glm.hpp>
#include <vector>
#include "../VertexArray.h"
#include "../VertexBuffer.h"
#include "../Shader.h"
#include "../Core/Camera.h"

class Stars {
public:
    Stars(unsigned int count, float radius);
    void Draw(Shader& shader, const Camera& camera);

private:
    unsigned int m_Count;
    VertexArray m_VAO;
    VertexBuffer* m_VBO; // wskaŸnik, bo chcemy kontrolowaæ ¿ywotnoœæ
};