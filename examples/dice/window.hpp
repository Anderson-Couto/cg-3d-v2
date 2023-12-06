#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <vector>
#include <random>
#include "abcgOpenGL.hpp"
#include "dices.hpp"
#include "trackball.hpp"
#include "gamedata.hpp"

class Window : public abcg::OpenGLWindow {
 protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onPaint() override;
  void onUpdate() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

 private:
  GameData m_gameData;
  glm::ivec2 m_viewportSize{};
  Dices m_dices;
  int m_trianglesToDraw{40704};
  int quantity{1};
  TrackBall m_trackBallModel;
  float m_zoom{};

  glm::mat4 m_modelMatrix{1.0f};
  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_projMatrix{1.0f};

  std::vector<const char*> m_shaderNames{"dice"};

  std::vector<GLuint> m_programs;
  int m_currentProgramIndex{};

  // Mapping mode
  // 0: triplanar; 1: cylindrical; 2: spherical; 3: from mesh
  int m_mappingMode{};

  // Light and material properties
  glm::vec4 m_lightDir{-1.0f, -1.0f, -1.0f, 0.0f};
  glm::vec4 m_Ia{1.0f};
  glm::vec4 m_Id{1.0f};
  glm::vec4 m_Is{1.0f};
  glm::vec4 m_Ka{};
  glm::vec4 m_Kd{};
  glm::vec4 m_Ks{};
  float m_shininess{};

  void loadModel(std::string_view path);
};

#endif