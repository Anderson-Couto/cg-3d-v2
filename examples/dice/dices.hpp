#ifndef DICES_HPP_
#define DICES_HPP_

#include "abcgOpenGL.hpp"
#include <random>
#include <list>

class Window;

struct Vertex {
  glm::vec3 position{};
  glm::vec3 normal{};
  glm::vec2 texCoord{};
  glm::vec4 Ka;
  glm::vec4 Kd;
  glm::vec4 Ks;
  float shininess;

  bool operator==(const Vertex& other) const noexcept {
    static const auto epsilon{std::numeric_limits<float>::epsilon()};
    return glm::all(glm::epsilonEqual(position, other.position, epsilon)) &&
           glm::all(glm::epsilonEqual(normal, other.normal, epsilon)) &&
           glm::all(glm::epsilonEqual(texCoord, other.texCoord, epsilon)) &&
           glm::all(glm::epsilonEqual(Ka, other.Ka, epsilon)) &&
           glm::all(glm::epsilonEqual(Kd, other.Kd, epsilon)) &&
           glm::all(glm::epsilonEqual(Ks, other.Ks, epsilon));
  }
};

struct Dice {
  glm::mat4 modelMatrix{1.0f};
  glm::vec3 position{0.0f};
  glm::vec3 rotationAngle{};
  float timeLeft{0.0f};
  float spinSpeed{2.5f};
  bool dadoGirando{false};
  bool dadoColidindo{false};
  glm::ivec3 DoRotateAxis{};
  glm::ivec3 DoTranslateAxis{};
};

class Dices {
  public:
    void create(int quantity);
    void destroy();
    void loadDiffuseTexture(std::string_view path); //here
    void loadObj(std::string_view path, bool standardize = true); //here
    void render(int numTriangles = -1) const; //here
    void setupVAO(GLuint program); //here
    void update(float deltaTime);
    void jogarDado(Dice &);
    void standardize(); //here

  std::vector<Dice> dices;

  [[nodiscard]] int getNumTriangles() const {
    return gsl::narrow<int>(m_indices.size()) / 3;
  }

  [[nodiscard]] glm::vec4 getKa() const { return m_Ka; }
  [[nodiscard]] glm::vec4 getKd() const { return m_Kd; }
  [[nodiscard]] glm::vec4 getKs() const { return m_Ks; }
  [[nodiscard]] float getShininess() const { return m_shininess; }

  [[nodiscard]] bool isUVMapped() const { return m_hasTexCoords; }

  private:
    GLuint m_VAO{};
    GLuint m_VBO{};
    GLuint m_EBO{};

    glm::vec4 m_Ka{};
    glm::vec4 m_Kd{};
    glm::vec4 m_Ks{};
    float m_shininess{};
    GLuint m_diffuseTexture{};

    std::default_random_engine m_randomEngine;

    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;

    bool m_hasNormals{false};
    bool m_hasTexCoords{false};

    Dice inicializarDado();
    void tempoGirandoAleatorio(Dice&);
    void eixoAlvoAleatorio(Dice&);
    void direcaoAleatoria(Dice&);
    void checkCollisions(Dice&);
    void computeNormals(); //here
    void createBuffers(); //here
};

#endif