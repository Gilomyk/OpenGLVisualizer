#include "Planet.h"
#include <GLFW/glfw3.h>
#include <glm/gtx/euler_angles.hpp>

Planet::Planet(float radius, unsigned int sectorCount, unsigned int stackCount, 
               float orbitRadius, glm::vec3 orbitTilt, float orbitSpeed, float spinSpeed,
               Planet* parent, bool enableOrbit)
    : m_Sphere(radius, sectorCount, stackCount),
    m_Layout([] {
        VertexBufferLayout l;
        l.Push<float>(3); // position
        l.Push<float>(3); // normal
        l.Push<float>(2); // texcoord
        return l;
    }()),
    m_Mesh(m_Sphere.GetVertices().data(),
        static_cast<unsigned int>(m_Sphere.GetVertices().size()),
        m_Sphere.GetIndices().data(),
        static_cast<unsigned int>(m_Sphere.GetIndices().size()),
        m_Layout),
    m_Position(0.0f),
    m_Scale(1.0f),
    m_Rotation(0.0f)
{
	m_OrbitRadius = orbitRadius;
    m_OrbitTilt = m_BaseOrbitTilt = orbitTilt;
	m_OrbitSpeed = orbitSpeed;
	m_SpinSpeed = spinSpeed;
	m_OrbitAngle = 0.0f;
	m_SpinAngle = 0.0f;
	m_Parent = parent;

    if (enableOrbit && orbitRadius > 0.0f) {
        m_Trail = std::make_unique<OrbitTrail>();

        m_Orbit = std::make_unique<Orbit>(orbitRadius, 100, glm::vec3(1.0f));
		m_Orbit->SetPosition(glm::vec3(0.0f));
		m_Orbit->SetRotation(m_OrbitTilt);
    }
}

void Planet::Update(float dt) {
	m_OrbitAngle += m_OrbitSpeed * dt;
	if (m_OrbitAngle > 360.0f) m_OrbitAngle -= 360.0f;

	m_SpinAngle += m_SpinSpeed * dt;
	if (m_SpinAngle > 360.0f) m_SpinAngle -= 360.0f;

	float rad = glm::radians(m_OrbitAngle);
	glm::vec3 orbitPos(m_OrbitRadius * cos(rad), 0.0f, m_OrbitRadius * sin(rad));

    glm::mat4 tilt = glm::yawPitchRoll(
        glm::radians(m_OrbitTilt.y),
        glm::radians(m_OrbitTilt.x),
        glm::radians(m_OrbitTilt.z));

    glm::vec3 rotatedPos = glm::vec3(tilt * glm::vec4(orbitPos, 1.0));

    m_Position = m_Parent ? m_Parent->GetPosition() + rotatedPos : rotatedPos;
	m_Rotation.y = m_SpinAngle; // Rotacja wokó³ osi Y

    if (m_Trail) {
        m_Trail->AddPoint(m_Position);
    }

    if (m_Orbit) {
        m_Orbit->SetPosition(glm::vec3(0.0f));
    }

}

void Planet::DrawPlanet(Shader& shader, Renderer& renderer, const Camera& camera) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_Position);
    model = glm::scale(model, m_Scale);
    model *= glm::toMat4(glm::quat(glm::radians(m_Rotation)));

    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    shader.Bind();
    
    // MVP and normal matrix
    shader.SetUniformMat4f("uModel", model);
    shader.SetUniformMat4f("uView", camera.GetViewMatrix());
    shader.SetUniformMat4f("uProjection", camera.GetProjectionMatrix());
    shader.SetUniformMat3f("uNormalMatrix", normalMatrix);

    // light uniforms
    shader.SetUniform3fv("uLightPos", glm::vec3(0.0f, 0.0f, 0.0f));
    shader.SetUniform3fv("uLightAmbient", glm::vec3(0.2f, 0.2f, 0.2f));
    shader.SetUniform3fv("uLightDiffuse", glm::vec3(0.7f, 0.7f, 0.7f));
    shader.SetUniform3fv("uLightSpecular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.SetUniform3fv("uViewPos", camera.GetPosition());

    // === MATERIAL ===
    if (m_Texture) {
        // tryb z tekstur¹ (np. S³oñce)
        m_Texture->Bind(0);
        shader.SetUniform1i("uUseTexture", 1);
        shader.SetUniform1i("uDiffuseMap", 0);
    }
    else {
        // tryb materia³owy (planety kolorowe)
        shader.SetUniform1i("uUseTexture", 0);
        shader.SetUniform3fv("uMaterial.diffuse", m_Material.diffuse);
        shader.SetUniform3fv("uMaterial.specular", m_Material.specular);
        shader.SetUniform1f("uMaterial.shininess", m_Material.shininess);
    }

    shader.SetUniform1f("uTime", (float)glfwGetTime());

    // Rendering
    renderer.Draw(m_Mesh, shader, GL_TRIANGLES);
}

void Planet::DrawSun(Shader& shader, Renderer& renderer, const Camera& camera) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, m_Position);
    model = glm::scale(model, m_Scale);
    model *= glm::toMat4(glm::quat(glm::radians(m_Rotation)));

    shader.Bind();
    shader.SetUniformMat4f("uModel", model);
    shader.SetUniformMat4f("uView", camera.GetViewMatrix());
    shader.SetUniformMat4f("uProjection", camera.GetProjectionMatrix());

    shader.SetUniform1f("uTime", glfwGetTime());

    shader.SetUniform1f("uFlickerStrength", 0.2f);
    shader.SetUniform1f("uGradientFalloff", 1.5f);

    if (m_Texture) {
        // tryb z tekstur¹ (np. S³oñce)
        m_Texture->Bind(0);
        shader.SetUniform1i("uDiffuseMap", 0);
    }

    renderer.Draw(m_Mesh, shader, GL_TRIANGLES);
}

void Planet::DebugPrint() const {
    std::cout << "Planet:\n";
    std::cout << "  Position: ("
        << m_Position.x << ", "
        << m_Position.y << ", "
        << m_Position.z << ")\n";
    std::cout << "  Orbit radius: " << m_OrbitRadius << "\n";
    std::cout << "  Orbit tilt (planet): ("
        << m_OrbitTilt.x << ", "
        << m_OrbitTilt.y << ", "
        << m_OrbitTilt.z << ")\n";
    std::cout << "  Orbit angle: " << m_OrbitAngle << " deg\n";
    std::cout << "  Spin angle: " << m_SpinAngle << " deg\n";

    if (m_Parent) {
        std::cout << "  Parent pos: ("
            << m_Parent->GetPosition().x << ", "
            << m_Parent->GetPosition().y << ", "
            << m_Parent->GetPosition().z << ")\n";
    }
    else {
        std::cout << "  Parent: none\n";
    }

    if (m_Orbit) {
        std::cout << "  Orbit Center Pos: ("
            << m_Orbit->GetPosition().x << ", "
            << m_Orbit->GetPosition().y << ", "
            << m_Orbit->GetPosition().z << ")\n";
        std::cout << " Orbit tilt (orbit): ("
            << m_Orbit->GetRotation().x << ", "
            << m_Orbit->GetRotation().y << ", "
            << m_Orbit->GetRotation().z << ")\n";
    }
    std::cout << "-----------------------------\n";
}
