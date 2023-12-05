#include "dices.hpp"
#include <fmt/core.h>
#include <tiny_obj_loader.h>
#include <glm/gtx/fast_trigonometry.hpp>
#include <cppitertools/itertools.hpp>
#include <filesystem>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

namespace std {
  template <> struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const noexcept {
      const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
      const std::size_t h2{std::hash<glm::vec3>()(vertex.normal)};
      const std::size_t h3{std::hash<glm::vec2>()(vertex.texCoord)};
      return h1 ^ h2 ^ h3;
    }
  };
}

void Dices::create(int quantity){
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  dices.clear();
  dices.resize(quantity);

  for(auto &dice : dices) {
    dice = inicializarDado();
  }
}

Dice Dices::inicializarDado() {
  Dice dice;
  std::uniform_real_distribution<float> fdist(-1.0f,1.0f);
  dice.position = glm::vec3{fdist(m_randomEngine),fdist(m_randomEngine),fdist(m_randomEngine)};

  jogarDado(dice);

  return dice;
}

void Dices::jogarDado(Dice &dice) {
  tempoGirandoAleatorio(dice);
  eixoAlvoAleatorio(dice);
  direcaoAleatoria(dice);
  dice.dadoGirando = true;
}

void Dices::update(float deltaTime) {
  for(auto &dice : dices) {
    if(dice.dadoGirando)
    {
      checkCollisions(dice);
      dice.timeLeft -= deltaTime;
      if(dice.DoRotateAxis.x)
        dice.rotationAngle.x = glm::wrapAngle(dice.rotationAngle.x + glm::radians(dice.spinSpeed) * dice.timeLeft); 
      if(dice.DoRotateAxis.y)
        dice.rotationAngle.y = glm::wrapAngle(dice.rotationAngle.y + glm::radians(dice.spinSpeed) * dice.timeLeft); 
      if(dice.DoRotateAxis.z)
        dice.rotationAngle.z = glm::wrapAngle(dice.rotationAngle.z + glm::radians(dice.spinSpeed) * dice.timeLeft); 
      if(dice.DoTranslateAxis.x != 0)
        dice.position.x = dice.position.x + dice.spinSpeed * dice.timeLeft * dice.DoTranslateAxis.x * 0.001f; 
      if(dice.DoTranslateAxis.y != 0)
        dice.position.y = dice.position.y + dice.spinSpeed * dice.timeLeft * dice.DoTranslateAxis.y * 0.001f; 
      if(dice.DoTranslateAxis.z != 0)
        dice.position.z = dice.position.z + dice.spinSpeed * dice.timeLeft * dice.DoTranslateAxis.z * 0.001f;
    }
    if(dice.dadoGirando && dice.timeLeft <= 0){
      dice.dadoGirando = false;
    }
  }
}

void Dices::tempoGirandoAleatorio(Dice &dice){
  std::uniform_real_distribution<float> fdist(2.0f,7.0f);
  dice.timeLeft = fdist(m_randomEngine);
}

void Dices::eixoAlvoAleatorio(Dice &dice){
  dice.DoRotateAxis = {0, 0, 0};
  std::uniform_int_distribution<int> idist(0,2);
  dice.DoRotateAxis[idist(m_randomEngine)] = 1;
}

void Dices::direcaoAleatoria(Dice &dice){
  dice.DoTranslateAxis = {0, 0, 0};
  std::uniform_int_distribution<int> idist(-1,1);
  dice.DoTranslateAxis = {idist(m_randomEngine),idist(m_randomEngine), idist(m_randomEngine)};
}

void Dices::checkCollisions(Dice &dice){
  bool colidiu{false};

  for(auto &other_dice : dices) {
    if(&other_dice == &dice) continue;

    const auto distance{glm::distance(other_dice.position, dice.position)};

    if (distance > 0.5f) continue;

    if(!dice.dadoColidindo) {
      dice.dadoColidindo = true;
      dice.DoTranslateAxis *= -1;
      colidiu = true;
    }
    if(!other_dice.dadoColidindo) {
      other_dice.dadoColidindo = true;
      other_dice.DoTranslateAxis = dice.DoTranslateAxis * (-1);
      tempoGirandoAleatorio(other_dice);
      eixoAlvoAleatorio(other_dice);
      other_dice.dadoGirando = true;
    }
  }
  if(!colidiu)
  {
    dice.dadoColidindo = false;
  }
  
  if(dice.position.x > 2.5f){
    dice.DoTranslateAxis.x = -1;
    colidiu = true;}
  else if(dice.position.x < -2.5f){
    dice.DoTranslateAxis.x = 1;
    colidiu = true;}
  if(dice.position.y > 2.5f){
    dice.DoTranslateAxis.y = -1;
    colidiu = true;}
  else if(dice.position.y < -2.5f){
    dice.DoTranslateAxis.y = 1;
    colidiu = true;}
  if(dice.position.z > 2.5f){
    dice.DoTranslateAxis.z = -1;
    colidiu = true;}
  else if(dice.position.z < -2.5f){
    dice.DoTranslateAxis.z = 1;
    colidiu = true;}

  if(colidiu){
    tempoGirandoAleatorio(dice);
    eixoAlvoAleatorio(dice);
  }
}

void Dices::computeNormals() {
  for (auto& vertex : m_vertices) {
    vertex.normal = glm::zero<glm::vec3>();
  }

  for (const auto offset : iter::range<int>(0, m_indices.size(), 3)) {
    Vertex& a{m_vertices.at(m_indices.at(offset + 0))};
    Vertex& b{m_vertices.at(m_indices.at(offset + 1))};
    Vertex& c{m_vertices.at(m_indices.at(offset + 2))};

    const auto edge1{b.position - a.position};
    const auto edge2{c.position - b.position};
    const glm::vec3 normal{glm::cross(edge1, edge2)};

    a.normal += normal;
    b.normal += normal;
    c.normal += normal;
  }

  for (auto& vertex : m_vertices) {
    vertex.normal = glm::normalize(vertex.normal);
  }

  m_hasNormals = true;
}

void Dices::createBuffers() {
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices[0]) * m_indices.size(), m_indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Dices::loadDiffuseTexture(std::string_view path) {
  if (!std::filesystem::exists(path)) return;
  
  abcg::glDeleteTextures(1, &m_diffuseTexture);
  m_diffuseTexture = abcg::loadOpenGLTexture( {.path = path} );
}

void Dices::loadObj(std::string_view path, bool standardize) {
  auto const basePath{std::filesystem::path{path}.parent_path().string() + "/"};

  tinyobj::ObjReaderConfig readerConfig;
  readerConfig.mtl_search_path = basePath;

  tinyobj::ObjReader reader;

  if (!reader.ParseFromFile(path.data(), readerConfig)) {
    if (!reader.Error().empty()) {
      throw abcg::RuntimeError(
          fmt::format("Failed to load model {} ({})", path, reader.Error()));
    }
    throw abcg::RuntimeError(fmt::format("Failed to load model {}", path));
  }

  if (!reader.Warning().empty()) {
    fmt::print("Warning: {}\n", reader.Warning());
  }

  auto const &attrib{reader.GetAttrib()};
  auto const &shapes{reader.GetShapes()};
  auto const &materials{reader.GetMaterials()};

  m_vertices.clear();
  m_indices.clear();

  m_hasNormals = false;
  m_hasTexCoords = false;

  std::unordered_map<Vertex, GLuint> hash{};

  for (auto const &shape : shapes) {
    for (auto const offset : iter::range(shape.mesh.indices.size())) {
      auto const index{shape.mesh.indices.at(offset)};

      auto const startIndex{3 * index.vertex_index};
      glm::vec3 position{attrib.vertices.at(startIndex + 0),
                         attrib.vertices.at(startIndex + 1),
                         attrib.vertices.at(startIndex + 2)};

      glm::vec3 normal{};
      if (index.normal_index >= 0) {
        m_hasNormals = true;
        auto const normalStartIndex{3 * index.normal_index};
        normal = {attrib.normals.at(normalStartIndex + 0),
                  attrib.normals.at(normalStartIndex + 1),
                  attrib.normals.at(normalStartIndex + 2)};
      }

      glm::vec2 texCoord{};
      if (index.texcoord_index >= 0) {
        m_hasTexCoords = true;
        auto const texCoordsStartIndex{2 * index.texcoord_index};
        texCoord = {attrib.texcoords.at(texCoordsStartIndex + 0),
                    attrib.texcoords.at(texCoordsStartIndex + 1)};
      }

      Vertex const vertex{
          .position = position, .normal = normal, .texCoord = texCoord};

      if (!hash.contains(vertex)) {
        hash[vertex] = m_vertices.size();
        m_vertices.push_back(vertex);
      }

      m_indices.push_back(hash[vertex]);
    }
  }

  if (!materials.empty()) {
    auto const &mat{materials.at(0)};
    m_Ka = {mat.ambient[0], mat.ambient[1], mat.ambient[2], 1};
    m_Kd = {mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1};
    m_Ks = {mat.specular[0], mat.specular[1], mat.specular[2], 1};
    m_shininess = mat.shininess;

    if (!mat.diffuse_texname.empty())
      loadDiffuseTexture(basePath + mat.diffuse_texname);
  } else {
    m_Ka = {0.1f, 0.1f, 0.1f, 1.0f};
    m_Kd = {0.7f, 0.7f, 0.7f, 1.0f};
    m_Ks = {1.0f, 1.0f, 1.0f, 1.0f};
    m_shininess = 25.0f;
  }

  if (standardize) {
    Dices::standardize();
  }

  if (!m_hasNormals) {
    computeNormals();
  }

  createBuffers();
}

void Dices::render(int numTriangles) const {
  abcg::glBindVertexArray(m_VAO);
  abcg::glActiveTexture(GL_TEXTURE0);
  abcg::glBindTexture(GL_TEXTURE_2D, m_diffuseTexture);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  abcg::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  auto const numIndices{(numTriangles < 0) ? m_indices.size()
                                           : numTriangles * 3};

  abcg::glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr);

  abcg::glBindVertexArray(0);
}

void Dices::setupVAO(GLuint program) {
  abcg::glDeleteVertexArrays(1, &m_VAO);
  abcg::glGenVertexArrays(1, &m_VAO);
  abcg::glBindVertexArray(m_VAO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  auto const positionAttribute{
      abcg::glGetAttribLocation(program, "inPosition")};
  if (positionAttribute >= 0) {
    abcg::glEnableVertexAttribArray(positionAttribute);
    abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex), nullptr);
  }

  auto const normalAttribute{abcg::glGetAttribLocation(program, "inNormal")};
  if (normalAttribute >= 0) {
    abcg::glEnableVertexAttribArray(normalAttribute);
    auto const offset{offsetof(Vertex, normal)};
    abcg::glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void *>(offset));
  }

  auto const texCoordAttribute{
      abcg::glGetAttribLocation(program, "inTexCoord")};
  if (texCoordAttribute >= 0) {
    abcg::glEnableVertexAttribArray(texCoordAttribute);
    auto const offset{offsetof(Vertex, texCoord)};
    abcg::glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE,
                                sizeof(Vertex),
                                reinterpret_cast<void *>(offset));
  }

  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);
  abcg::glBindVertexArray(0);
}

void Dices::standardize() {
  glm::vec3 max(std::numeric_limits<float>::lowest());
  glm::vec3 min(std::numeric_limits<float>::max());
  for (auto const &vertex : m_vertices) {
    max = glm::max(max, vertex.position);
    min = glm::min(min, vertex.position);
  }

  auto const center{(min + max) / 2.0f};
  auto const scaling{2.0f / glm::length(max - min)};
  for (auto &vertex : m_vertices) {
    vertex.position = (vertex.position - center) * scaling;
  }
}

void Dices::destroy(){
  abcg::glDeleteTextures(1, &m_diffuseTexture);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}