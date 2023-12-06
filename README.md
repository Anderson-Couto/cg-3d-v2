# Dice V2

# Membros

Anderson Couto Cardoso (RA 21017415)

## Introdução

Aplicação desenvolvida para o curso Computação Gráfica, com o intuito de mostrar gráficos 3D com Open GL. Essa nova versão tem a proposta de incluir mais funcionalidades relacionadas aos serviços em três dimensões e texturização.

Detalhes da primeira versão é possível encontrar aqui: https://github.com/Anderson-Couto/cg-3d-v1

## Descrição da Aplicação

Nesta nova versão de Dice, o usuário deve selecionar uma quantidade de dados a serem jogados. Após definido, a aplicação inicia ao pressionar a tecla "SPACE", e então aquela quantidade de dados irá rotacionar pela tela e sorteará números de 1 a 6 em cada dado. É possível rotacionar o espaço da tela por meio do "BOTÃO ESQUERDO" do mouse.

É possivel acessar a aplicação pelo link: https://anderson-couto.github.io/cg-3d-v2/public/index.html

## Estrutura

Neste projeto, foi utilizada a linguagem C++, assim como as bibliotecas `ABCg` e `ImGui`. Por fim, foi utilizada a API `OpenGL` para renderização gráfica.
A estrutura do codigo ficou da seguinte forma:

- `main.cpp`: Classe base do software;
- `window.hpp`: Define a interface da classe Window, que herda da classe `abcg::OpenGLWindow`. Essa classe é destinada para a criação de uma janela OpenGL e manipulação de eventos relacionados.
- `window.cpp`: Implementação dos métodos e atributos da classe Window. É aqui que são invocados os shaders, renderização dos objetos e atribuições de regras de negócio que atendem todo a aplicação;
- `gamedata.hpp`: Abstração responsável por guardar estados da aplicação;
- `imfilebrowser.h`: Biblioteca de funções genéricas;
- `dices.hpp`: Classe com métodos e atributos relacionados aos dados;
- `dices.cpp`: Implementação dos métodos e atributos da classe Dices;
- `trackball.hpp`: Classe com métodos e atributos relacionados ao giro de tela;
- `trackball.cpp`: Implementação dos métodos e atributos da classe Trackball;
- `assets\shaders\dice.frag`: _Fragment shader_, é o objeto que possui as propriedades do dado e programa como os pixels são exibidos na tela.
- `assets\shaders\dice.vert`: _Vertex shader_, é o objeto que possui as propriedades de vértices do dado.
- `assets\dice.mtl`: Contêm uma ou mais definições de material, cada uma das quais inclui a cor, a textura e o mapa de reflexão de materiais individuais dos dados.
- `assets\dice.obj`: Representação 3D de um dado.
- `assets\maps\dice.jpg`: Textura do dado que está sendo utilizada na aplicação.

## Desenvolvimento e Code Review

Abaixo podemos encontrar mais detalhes sobre o desenvolvimento da aplicação. Em cada etapa, será feito um comparativo entre este código e a versão 1. É possível encontrar mais detalhes sobre a versão anterior no link: https://github.com/Anderson-Couto/cg-3d-v1.
A base do projeto utiliza o código `viewer4`, do Professor Harlen Batagelo. É possivel baixar pelo link: https://hbatagelo.github.io/abcgapps/src/viewer4.zip

### main.cpp

Não há grandes novidades aqui. O código segue o mesmo padrão do que foi desenvolvido ao longo do curso.

### gamedata.hpp

Aqui ocorre apenas a gestão do input de rolar o dado. Não há mudanças em relação ao codigo anterior.

### window.hpp e window.cpp

A classe `Window` recebeu poucas alterações de estrutura. As grandes mudanças foram a criação da função onUpdate(), responsável por realizar atualizações em matrizes da aplicação e fazer chamadas à função update da classe dices, a inclusão de variáveis relacionadas ao Trackball, para manipulação de ambiente, e inclusão de variáveis relacionadas à iluminação e renderização dos objetos em tela. Abaixo segue um resumo das funcionalidades:

- Manipulação de Eventos (onEvent):

  - Rastreamento do movimento do mouse para controlar a rotação da câmera (usando uma trackball).
  - Ativação da rotação dos dados ao pressionar a barra de espaço.
  - Resposta a eventos de pressionar e soltar o botão esquerdo do mouse para interação com a trackball.
  - Zoom com a roda do mouse.

- Inicialização (onCreate):

  - Configuração inicial do OpenGL.
  - Compilação e criação de programas de shader a partir de arquivos no sistema de arquivos.
  - Carregamento de um modelo 3D padrão ("dice.obj").
  - Inicialização da trackball para rotação.
  - Criação de instâncias de dados (dices) e configuração de parâmetros iniciais.

- Atualização (onUpdate):

  - Atualização da matriz de visualização e projeção da câmera.
  - Atualização da rotação da trackball.
  - Atualização dos dados (dices).

- Renderização (onPaint):

  - Limpeza do buffer de cor e do buffer de profundidade.
  - Definição de uniformes compartilhados entre todos os modelos (iluminação, texturas, etc.).
  - Renderização de cada instância de dado (dice) no loop.

- Interface do Usuário (onPaintUI):

  - Utilização da biblioteca ImGui para criar uma interface gráfica simples.
  - Botão para rolar todos os dados.
  - Combo box para selecionar o número de dados.

- Redimensionamento da Janela (onResize):

  - Atualização do tamanho da viewport ao redimensionar a janela.

- Carregamento de Modelo (loadModel):

  - Limpeza de dados existentes.
  - Carregamento de um novo modelo a partir de um arquivo (por padrão, "dice.obj").
  - Configuração dos parâmetros de material com base no modelo carregado.

- Destruir Recursos (onDestroy):
  - Liberação de recursos OpenGL e limpeza dos dados dos dados (dices).

### trackball.hpp e trackball.cpp

É a mesma base do projeto `viewer4`.

### dices.hpp e dices.cpp

A classe `Dices` recebeu a base da classe `Model`, presente no projeto `viewer4`. Abaixo segue um resumo das funcionalidades próprias da classe:

- Criação de Dados (create):
  - Gera uma semente para o gerador de números aleatórios.
  - Inicializa um vetor de dados (m_dices) com a quantidade especificada.
  - Cada dado é inicializado chamando inicializarDado.

```
void Dices::create(int quantity) {
  auto seed{std::chrono::steady_clock::now().time_since_epoch().count()};
  m_randomEngine.seed(seed);

  m_dices.clear();
  m_dices.resize(quantity);

  for(auto &dice : m_dices) {
    dice = inicializarDado();
  }
}
```

- Inicialização de Dado (inicializarDado):
  - Inicializa um dado com uma posição aleatória e chama a função jogarDado para iniciar sua rotação.

```
Dices::Dice Dices::inicializarDado() {
  Dice dice;
  std::uniform_real_distribution<float> fdist(-1.0f, 1.0f);
  dice.position = glm::vec3{fdist(m_randomEngine), fdist(m_randomEngine), fdist(m_randomEngine)};

  jogarDado(dice);

  return dice;
}
```

- Jogar o Dado (jogarDado):
  - Configura a rotação inicial e a direção de translação do dado.
  - Faz a gestão do giro do dado.

```
void Dices::jogarDado(Dice &dice) {
  alterarSpin(dice);

  dice.DoTranslateAxis = {0, 0, 0};
  std::uniform_int_distribution<int> idist(-1, 1);
  dice.DoTranslateAxis = {idist(m_randomEngine), idist(m_randomEngine), idist(m_randomEngine)};
  dice.dadoGirando = true;
}
```

- Atualização dos Dados (update):
  - Atualiza a velocidade de rotação dos dados com um decaimento ao longo do tempo.
  - Atualiza a posição e a rotação dos dados em cada quadro, levando em consideração colisões.

```
void Dices::update(float deltaTime) {
  for(auto &dice : m_dices) {
    // Atualização da velocidade com decaimento
    dice.spinSpeed -= dice.decayRate * deltaTime;
    dice.spinSpeed = std::max(dice.spinSpeed, 0.0f);

    if(dice.dadoGirando) {
      // ...
      // Lógica para atualização da posição e rotação dos dados
    }

    if(dice.dadoGirando && dice.timeLeft <= 0) {
      dice.dadoGirando = false;
    }
  }
}
```

- Alterar Spin do Dado (alterarSpin):
  - Gera um tempo aleatório e uma direção aleatória de rotação para o dado.

```
void Dices::alterarSpin(Dice &dice) {
  std::uniform_real_distribution<float> fdist(1.0f, 5.0f);
  dice.timeLeft = fdist(m_randomEngine);
  dice.DoRotateAxis = {0, 0, 0};
  std::uniform_int_distribution<int> idist(0, 2);
  dice.DoRotateAxis[idist(m_randomEngine)] = 1;
}
```

- Verificar Colisões (checkCollisions):
  - Verifica colisões entre os dados e atualiza as posições e rotações em caso de colisão.

### dice.frag e dice.vert

É a mesma base do shader `textura`, presente no projeto `viewer4`.

## Referências

https://free3d.com/3d-model/dice-34662.html

# About ABCg

![build workflow](https://github.com/hbatagelo/abcg/actions/workflows/build.yml/badge.svg)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/hbatagelo/abcg)](https://github.com/hbatagelo/abcg/releases/latest)

Development framework accompanying the course [MCTA008-17 Computer Graphics](http://professor.ufabc.edu.br/~harlen.batagelo/cg/) at [UFABC](https://www.ufabc.edu.br/).

[Documentation](https://hbatagelo.github.io/abcg/abcg/doc/html/) | [Release notes](CHANGELOG.md)

ABCg is a lightweight C++ framework that simplifies the development of 3D graphics applications based on [OpenGL](https://www.opengl.org), [OpenGL ES](https://www.khronos.org), [WebGL](https://www.khronos.org/webgl/), and [Vulkan](https://www.vulkan.org). It is designed for the tutorials and assignments of the course "MCTA008-17 Computer Graphics" taught at Federal University of ABC (UFABC).

---

## Main features

- Supported platforms: Linux, macOS, Windows, WebAssembly.
- Supported backends: OpenGL 3.3+, OpenGL ES 3.0+, WebGL 2.0 (via Emscripten), Vulkan 1.3.
- Applications that use the common subset of functions between OpenGL 3.3 and OpenGL ES 3.0 can be built for WebGL 2.0 using the same source code.
- OpenGL functions can be qualified with the `abcg::` namespace to enable throwing exceptions with descriptive GL error messages that include the source code location.
- Includes helper classes and functions for loading textures (using [SDL_image](https://www.libsdl.org/projects/SDL_image/)), loading OBJ 3D models (using [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)), and compiling GLSL shaders to SPIR-V with [glslang](https://github.com/KhronosGroup/glslang).

---

## Requirements

The following minimum requirements are shared among all platforms:

- [CMake](https://cmake.org/) 3.21.
- A C++ compiler with at least partial support for C++20 (tested with GCC 12, Clang 16, MSVC 17, and emcc 3.1.42).
- A system with support for OpenGL 3.3 (OpenGL backend) or Vulkan 1.3 (Vulkan backend). Conformant software rasterizers such as Mesa's [Gallium llvmpipe](https://docs.mesa3d.org/drivers/llvmpipe.html) and lavapipe (post Jun 2022) are supported. Mesa's [D3D12](https://devblogs.microsoft.com/directx/directx-heart-linux/) backend on [WSL 2.0](https://docs.microsoft.com/en-us/windows/wsl/install) is supported as well.

For WebAssembly:

- [Emscripten](https://emscripten.org/).
- A browser with support for WebGL 2.0.

For building desktop applications:

- [SDL](https://www.libsdl.org/) 2.0.
- [SDL_image](https://www.libsdl.org/projects/SDL_image/) 2.0.
- [GLEW](http://glew.sourceforge.net/) 2.2.0 (required for OpenGL-based applications).
- [Vulkan](https://www.lunarg.com/vulkan-sdk/) 1.3 (required for Vulkan-based applications).

Desktop dependencies can be resolved automatically with [Conan](https://conan.io/), but it is disabled by default. To use Conan, install Conan 1.47 or a later 1.\* version (ABCg is not compatible with Conan 2.0!) and then configure CMake with `-DENABLE_CONAN=ON`.

The default renderer backend is OpenGL (CMake option `GRAPHICS_API=OpenGL`). To use the Vulkan backend, configure CMake with `-DGRAPHICS_API=Vulkan`.

---

## Installation and usage

Start by cloning the repository:

    # Get abcg repo
    git clone https://github.com/hbatagelo/abcg.git

    # Enter the directory
    cd abcg

Follow the instructions below to build the "Hello, World!" sample located in `abcg/examples/helloworld`.

### Windows

- Run `build-vs.bat` for building with the Visual Studio 2022 toolchain.
- Run `build.bat` for building with GCC (MinGW-w64).

`build-vs.bat` and `build.bat` accept two optional arguments: (1) the build type, which is `Release` by default, and (2) an extra CMake option. For example, for a `Debug` build with `-DENABLE_CONAN=ON` using VS 2022, run

    build-vs.bat Debug -DENABLE_CONAN=ON

### Linux and macOS

Run `./build.sh`.

The script accepts two optional arguments: (1) the build type, which is `Release` by default, and (2) an extra CMake option. For example, for a `Debug` build with `-DENABLE_CONAN=ON`, run

    ./build.sh Debug -DENABLE_CONAN=ON

### WebAssembly

1.  Run `build-wasm.bat` (Windows) or `./build-wasm.sh` (Linux/macOS).
2.  Run `runweb.bat` (Windows) or `./runweb.sh` (Linux/macOS) for setting up a local web server.
3.  Open <http://localhost:8080/helloworld.html>.

---

## Docker setup

ABCg can be built in a [Docker](https://www.docker.com/) container. The Dockerfile provided is based on Ubuntu 22.04 and includes Emscripten.

1.  Create the Docker image (`abcg`):

        sudo docker build -t abcg .

2.  Create the container (`abcg_container`):

        sudo docker create -it \
          -p 8080:8080 \
          -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
          -e DISPLAY \
          --name abcg_container abcg

3.  Start the container:

        sudo docker start -ai abcg_container

    On NVIDIA GPUs, install the [NVIDIA Container Toolkit](https://github.com/NVIDIA/nvidia-docker) to allow the container to use the host's NVIDIA driver and X server. Expose the X server with `sudo xhost +local:root` before starting the container.

---

## License

ABCg is licensed under the MIT License. See [LICENSE](https://github.com/hbatagelo/abcg/blob/main/LICENSE) for more information.
