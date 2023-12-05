#include "window.hpp"

#include <imgui.h>
#include <cppitertools/itertools.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <fmt/core.h>
#include "imfilebrowser.h"

void Window::onEvent(SDL_Event const &event) {
  glm::ivec2 mousePosition;
  SDL_GetMouseState(&mousePosition.x, &mousePosition.y);

  if (event.type == SDL_MOUSEMOTION) {
    m_trackBallModel.mouseMove(mousePosition);
  }

  if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE) {
    m_gameData.m_input.set(static_cast<size_t>(Input::Roll));
  }

  if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
      m_trackBallModel.mousePress(mousePosition);
  }

  if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_SPACE) {
    m_gameData.m_input.reset(static_cast<size_t>(Input::Roll));
  }
  
  if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
      m_trackBallModel.mouseRelease(mousePosition);
  }

  if (event.type == SDL_MOUSEWHEEL) {
    m_zoom += (event.wheel.y > 0 ? -1.0f : +1.0f) / 5.0f;
    m_zoom = glm::clamp(m_zoom, -1.5f, 10.0f);
  }

}

void Window::onCreate() {
  abcg::glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  abcg::glEnable(GL_DEPTH_TEST);

  auto const assetsPath{abcg::Application::getAssetsPath()};

  for (const auto& name : m_shaderNames) {
    const auto path{assetsPath + "shaders/" + name};
    
    const auto program = 
      abcg::createOpenGLProgram({{.source = path + ".vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = path + ".frag",
                                  .stage = abcg::ShaderStage::Fragment}});

    m_programs.push_back(program);
  }

  loadModel(assetsPath + "dice.obj");
  m_mappingMode = 0;
  m_dices.create(quantity);
}

void Window::loadModel(std::string_view path) {
  m_dices.destroy();

  auto const assetsPath{abcg::Application::getAssetsPath()};
  m_dices.loadDiffuseTexture(assetsPath + "maps/dice.jpg");
  m_dices.loadObj(path);
  m_dices.setupVAO(m_programs.at(m_currentProgramIndex));
}

void Window::onPaint() {
  update();

  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  const auto program{m_programs.at(m_currentProgramIndex)};
  abcg::glUseProgram(program);

  const GLint viewMatrixLoc{abcg::glGetUniformLocation(program, "viewMatrix")};
  const GLint projMatrixLoc{abcg::glGetUniformLocation(program, "projMatrix")};
  const GLint modelMatrixLoc{abcg::glGetUniformLocation(program, "modelMatrix")};
  const GLint normalMatrixLoc{abcg::glGetUniformLocation(program, "normalMatrix")};
  const GLint lightDirLoc{abcg::glGetUniformLocation(program, "lightDirWorldSpace")};
  const GLint IaLoc{abcg::glGetUniformLocation(program, "Ia")};
  const GLint IdLoc{abcg::glGetUniformLocation(program, "Id")};
  const GLint IsLoc{abcg::glGetUniformLocation(program, "Is")};
  const GLint diffuseTexLoc{abcg::glGetUniformLocation(program, "diffuseTex")};
  const GLint mappingModeLoc{abcg::glGetUniformLocation(program, "mappingMode")}; 

  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);

  const auto lightDirRotated{m_trackBallModel.getRotation() * m_lightDir};
  abcg::glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  abcg::glUniform4fv(IaLoc, 1, &m_Ia.x);
  abcg::glUniform4fv(IdLoc, 1, &m_Id.x);
  abcg::glUniform4fv(IsLoc, 1, &m_Is.x);
  abcg::glUniform1i(diffuseTexLoc, 0);
  abcg::glUniform1i(mappingModeLoc, m_mappingMode);
  
  for(auto &dice : m_dices.dices){
    dice.modelMatrix = glm::translate(m_modelMatrix, dice.position);
    dice.modelMatrix = glm::scale(dice.modelMatrix, glm::vec3(0.5f));
    dice.modelMatrix = glm::rotate(dice.modelMatrix, dice.rotationAngle.x, glm::vec3(1.0f, 0.0f, 0.0f));
    dice.modelMatrix = glm::rotate(dice.modelMatrix, dice.rotationAngle.y, glm::vec3(0.0f, 1.0f, 0.0f));
    dice.modelMatrix = glm::rotate(dice.modelMatrix, dice.rotationAngle.z, glm::vec3(0.0f, 0.0f, 1.0f));

    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &dice.modelMatrix[0][0]);
    const auto modelViewMatrix{glm::mat3(m_viewMatrix * dice.modelMatrix)};
    glm::mat3 normalMatrix{glm::inverseTranspose(modelViewMatrix)};
    abcg::glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

    m_dices.render();
  }

  abcg::glUseProgram(0);
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();

  {
    ImGui::SetNextWindowPos(ImVec2(m_viewportSize.x - 138, m_viewportSize.y - 40));
    ImGui::SetNextWindowSize(ImVec2(128, 35));
    ImGui::Begin("Button window", nullptr, ImGuiWindowFlags_NoDecoration);

    ImGui::PushItemWidth(200);
    if(m_gameData.m_input[static_cast<size_t>(Input::Roll)]){
      for(auto &dice : m_dices.dices){
        m_dices.jogarDado(dice);
      } 
    }
    {
      static std::size_t currentIndex{};
      const std::vector<std::string> comboItems{"1", "2", "3", "4", "5"};

      ImGui::PushItemWidth(70);
      if (ImGui::BeginCombo("Dados",
                            comboItems.at(currentIndex).c_str())) {
        for (const auto index : iter::range(comboItems.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();
      if(quantity != (int)currentIndex + 1){
        quantity = currentIndex + 1;
        m_dices.create(quantity);
      }
    }

    ImGui::End();
  }
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  m_trackBallModel.resizeViewport(size);
}

void Window::onDestroy() {
  m_dices.destroy();
  for (const auto& program : m_programs) {
    abcg::glDeleteProgram(program);
  }
}

void Window::update() {
  const float deltaTime{static_cast<float>(getDeltaTime())};

  m_dices.update(deltaTime);

  m_modelMatrix = m_trackBallModel.getRotation();

  m_viewMatrix =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f + m_zoom),
                  glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  const auto aspect{static_cast<float>(m_viewportSize.x) /
                        static_cast<float>(m_viewportSize.y)};
  m_projMatrix =
      glm::perspective(glm::radians(45.0f), aspect, 0.1f, 25.0f);

  abcg::glDisable(GL_CULL_FACE);
  abcg::glFrontFace(GL_CCW);
}