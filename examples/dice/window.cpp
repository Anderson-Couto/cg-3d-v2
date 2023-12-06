#include "window.hpp"

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
    m_zoom += (event.wheel.y > 0 ? -1.0f : 1.0f) / 5.0f;
    m_zoom = glm::clamp(m_zoom, -1.5f, 10.0f);
  }

}

void Window::onCreate() {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  abcg::glClearColor(0, 0, 0, 1);
  abcg::glEnable(GL_DEPTH_TEST);

  // Create programs
  for (auto const &name : m_shaderNames) {
    auto const path{assetsPath + "shaders/" + name};
    auto const program{abcg::createOpenGLProgram(
        {{.source = path + ".vert", .stage = abcg::ShaderStage::Vertex},
         {.source = path + ".frag", .stage = abcg::ShaderStage::Fragment}})};
    m_programs.push_back(program);
  }

  // Load default model
  loadModel(assetsPath + "dice.obj");
  m_mappingMode = 0; // "From mesh" option

  // Initial trackball spin
  m_trackBallModel.setAxis(glm::normalize(glm::vec3(1, 1, 1)));
  m_trackBallModel.setVelocity(0.1f);

  m_dices.create(quantity);
}

void Window::loadModel(std::string_view path) {
  auto const assetsPath{abcg::Application::getAssetsPath()};

  m_dices.destroy();

  m_dices.loadDiffuseTexture(assetsPath + "maps/dice.jpg");
  m_dices.loadObj(path);
  m_dices.setupVAO(m_programs.at(m_currentProgramIndex));
  m_trianglesToDraw = m_dices.getNumTriangles();

  // Use material properties from the loaded model
  m_Ka = m_dices.getKa();
  m_Kd = m_dices.getKd();
  m_Ks = m_dices.getKs();
  m_shininess = m_dices.getShininess();
}

void Window::onPaint() {
  update();

  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  // Use currently selected program
  auto const program{m_programs.at(m_currentProgramIndex)};
  abcg::glUseProgram(program);

  // Get location of uniform variables
  auto const viewMatrixLoc{abcg::glGetUniformLocation(program, "viewMatrix")};
  auto const projMatrixLoc{abcg::glGetUniformLocation(program, "projMatrix")};
  auto const modelMatrixLoc{abcg::glGetUniformLocation(program, "modelMatrix")};
  auto const normalMatrixLoc{
      abcg::glGetUniformLocation(program, "normalMatrix")};
  auto const lightDirLoc{
      abcg::glGetUniformLocation(program, "lightDirWorldSpace")};
  auto const shininessLoc{abcg::glGetUniformLocation(program, "shininess")};
  auto const IaLoc{abcg::glGetUniformLocation(program, "Ia")};
  auto const IdLoc{abcg::glGetUniformLocation(program, "Id")};
  auto const IsLoc{abcg::glGetUniformLocation(program, "Is")};
  auto const KaLoc{abcg::glGetUniformLocation(program, "Ka")};
  auto const KdLoc{abcg::glGetUniformLocation(program, "Kd")};
  auto const KsLoc{abcg::glGetUniformLocation(program, "Ks")};
  auto const diffuseTexLoc{abcg::glGetUniformLocation(program, "diffuseTex")};
  auto const mappingModeLoc{abcg::glGetUniformLocation(program, "mappingMode")};

  // Set uniform variables that have the same value for every model
  abcg::glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, &m_viewMatrix[0][0]);
  abcg::glUniformMatrix4fv(projMatrixLoc, 1, GL_FALSE, &m_projMatrix[0][0]);
  abcg::glUniform1i(diffuseTexLoc, 0);
  abcg::glUniform1i(mappingModeLoc, m_mappingMode);

  auto const lightDirRotated{m_trackBallModel.getRotation() * m_lightDir};
  abcg::glUniform4fv(lightDirLoc, 1, &lightDirRotated.x);
  abcg::glUniform4fv(IaLoc, 1, &m_Ia.x);
  abcg::glUniform4fv(IdLoc, 1, &m_Id.x);
  abcg::glUniform4fv(IsLoc, 1, &m_Is.x);
  
  for(auto &dice : m_dices.m_dices){
    dice.modelMatrix = glm::translate(m_modelMatrix, dice.position);
    dice.modelMatrix = glm::scale(dice.modelMatrix, glm::vec3(0.5f));
    dice.modelMatrix = glm::rotate(dice.modelMatrix, dice.rotationAngle.x, glm::vec3(1.0f, 0.0f, 0.0f));
    dice.modelMatrix = glm::rotate(dice.modelMatrix, dice.rotationAngle.y, glm::vec3(0.0f, 1.0f, 0.0f));
    dice.modelMatrix = glm::rotate(dice.modelMatrix, dice.rotationAngle.z, glm::vec3(0.0f, 0.0f, 1.0f));

    abcg::glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, &dice.modelMatrix[0][0]);
    auto const modelViewMatrix{glm::mat3(m_viewMatrix * dice.modelMatrix)};
    auto const normalMatrix{glm::inverseTranspose(modelViewMatrix)};
    abcg::glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, &normalMatrix[0][0]);

    abcg::glUniform4fv(KaLoc, 1, &m_Ka.x);
    abcg::glUniform4fv(KdLoc, 1, &m_Kd.x);
    abcg::glUniform4fv(KsLoc, 1, &m_Ks.x);
    abcg::glUniform1f(shininessLoc, m_shininess);

    m_dices.render(m_trianglesToDraw);
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
      for(auto &dice : m_dices.m_dices){
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

  auto const aspect{gsl::narrow<float>(m_viewportSize.x) /
                        gsl::narrow<float>(m_viewportSize.y)};
  m_projMatrix =
            glm::perspective(glm::radians(45.0f), aspect, 0.1f, 25.0f);

  abcg::glFrontFace(GL_CCW);
}