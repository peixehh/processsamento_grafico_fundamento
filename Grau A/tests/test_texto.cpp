#include "check.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "TextRenderer.h"

int main()
{
    if (!glfwInit())
    {
        std::puts("GLFW nao iniciou");
        return 2;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* janela = glfwCreateWindow(200, 100, "teste", nullptr, nullptr);
    if (!janela)
    {
        std::puts("sem contexto OpenGL 3.3");
        glfwTerminate();
        return 2;
    }
    glfwMakeContextCurrent(janela);
    if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))
    {
        std::puts("GLAD nao carregou");
        return 2;
    }

    const std::string base = std::string(GAME_ROOT) + "/assets/";
    const std::string vert = base + "shaders/text.vert", frag = base + "shaders/text.frag";

    // Review Focus 2 fonte inexistente => init devolve false (e nao trava)
    {
        TextRenderer ruim;
        CHECK(!ruim.init(vert, frag, base + "fonts/nao-existe.ttf", { 16 }));
    }

    TextRenderer t;
    CHECK(t.init(vert, frag, base + "fonts/DejaVuSansMono-Bold.ttf", { 16, 32 }));

    // fonte monoespacada a 16 px 10 px por caractere (o mesmo da celula do atlas antigo)
    CHECK_NEAR(t.width("", 16), 0.0, 1e-6);
    CHECK_NEAR(t.width("ABCDE", 16), 50.0, 1e-6);
    CHECK_NEAR(t.width("      ", 16), 60.0, 1e-6);                 // espacos tambem andam
    CHECK_NEAR(t.width("a\xC3\xA7\xC3\xA3o", 16), 40.0, 1e-6);     // "acao" com cedilha e til 2 bytes UTF-8 = 1 caractere
    double w16 = t.width("A", 16), w32 = t.width("A", 32);
    CHECK(w32 > w16 * 1.8 && w32 < w16 * 2.2);                      // 32 px e cerca de 2x o de 16 px
    CHECK(t.lineHeight(16) >= 16 && t.lineHeight(16) <= 24);

    // Review Focus 1 fora do Latin-1, byte solto, UTF-8 truncado e controles viram '?' (sem travar nem ler alem do fim)
    const double q = t.width("?", 16);
    CHECK_NEAR(t.width("\xE2\x82\xAC", 16), q, 1e-6);              // euro (3 bytes)
    CHECK_NEAR(t.width("\xF0\x9F\x98\x80", 16), q, 1e-6);          // emoji (4 bytes)
    CHECK_NEAR(t.width("\x80", 16), q, 1e-6);                       // byte de continuacao solto
    CHECK_NEAR(t.width("abc\xC3", 16), t.width("abc?", 16), 1e-6);  // sequencia truncada no fim
    CHECK_NEAR(t.width("a\nb", 16), t.width("a?b", 16), 1e-6);      // caractere de controle

    // tamanho que nao foi carregado nao faz nada e nao quebra
    CHECK_NEAR(t.width("A", 24), 0.0, 1e-6);
    t.draw("A", 0.0f, 0.0f, 24, { 1.0f, 1.0f, 1.0f, 1.0f });

    // desenha um "A" de 32 px num framebuffer fora da tela e olha os pixels
    GLuint fbo = 0, tex = 0;
    glGenFramebuffers(1, &fbo);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 200, 100, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    CHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glViewport(0, 0, 200, 100);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGetError();   // limpa erros antigos
    t.setProjection(0.0f, 200.0f, 100.0f, 0.0f);
    t.draw("A", 10.0f, 10.0f, 32, { 1.0f, 1.0f, 1.0f, 1.0f });
    CHECK(glGetError() == GL_NO_ERROR);

    std::vector<unsigned char> px(200 * 100 * 4);
    glReadPixels(0, 0, 200, 100, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    int claros = 0;
    for (size_t i = 0; i < px.size(); i += 4)
        if (px[i] > 128) ++claros;
    CHECK(claros > 50);            // o "A" de 32 px tem centenas de pixels claros
    CHECK(claros < 2000);          // e nao pintou a tela inteira
    CHECK(px[(0 * 200 + 199) * 4] == 0);   // canto inferior direito continua preto

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
    glfwDestroyWindow(janela);
    glfwTerminate();
    return resultado();
}
