"""

Base de filtros do InstagramStories

Cada filtro é descrito por um objeto Filtro com:
    - nome, categoria e descrição textual;
    - lista de parâmetros configuráveis (Param);
    - função func(img, params) -> img

As funções de filtro recebem uma imagem uint8 com 1 canal (cinza ou um único
canal R/G/B) ou 3 canais (BGR) e devolvem uma imagem uint8. A escolha do canal
(RGB, Cinza, R, G ou B) é feita por `aplicar_por_canal`, que separa o canal,
chama o filtro e recompõe a imagem.

Referências: slides "Filtros - OpenCV" e "Processamento de Imagens"
(R. B. Queiroz), Gonzalez & Woods (2018) e docs.opencv.org/4.x
"""
from __future__ import annotations

from dataclasses import dataclass, field
from functools import lru_cache
from typing import Any, Callable

import cv2
import numpy as np

from .io_utils import para_bgr, para_cinza

# Canais
CANAIS = ("RGB", "Cinza", "R", "G", "B")
INDICE_CANAL = {"B": 0, "G": 1, "R": 2}  # OpenCV guarda os canais em ordem BGR

DESCRICAO_CANAIS = {
    "RGB": "imagem inteira (colorida)",
    "Cinza": "imagem inteira em tons de cinza",
    "R": "somente o canal Vermelho",
    "G": "somente o canal Verde",
    "B": "somente o canal Azul",
}


def aplicar_por_canal(img: np.ndarray, canal: str, fn: Callable, img_b: np.ndarray | None = None) -> np.ndarray:
    img = para_bgr(img)

    def chamar(a, b):
        return fn(a) if b is None else fn(a, b)

    if canal == "RGB":
        return para_bgr(chamar(img, None if img_b is None else para_bgr(img_b)))
    if canal == "Cinza":
        saida = chamar(para_cinza(img), None if img_b is None else para_cinza(img_b))
        return cv2.cvtColor(para_cinza(saida), cv2.COLOR_GRAY2BGR)
    i = INDICE_CANAL[canal]
    a = np.ascontiguousarray(img[:, :, i])
    b = None if img_b is None else np.ascontiguousarray(para_bgr(img_b)[:, :, i])
    saida = para_cinza(chamar(a, b))
    resultado = img.copy()
    resultado[:, :, i] = saida
    return resultado


def visualizar_canal(img: np.ndarray, canal: str, isolado_em_cinza: bool = False) -> np.ndarray:
    """
    Imagem exibida na tela de acordo com o canal selecionado

    Ao selecionar um único canal, a janela principal mostra o resultado
    daquele canal: colorido (só R, só G ou só B) ou em tons de cinza
    """
    img = para_bgr(img)
    if canal == "RGB":
        return img
    if canal == "Cinza":
        return cv2.cvtColor(para_cinza(img), cv2.COLOR_GRAY2BGR)
    i = INDICE_CANAL[canal]
    c = img[:, :, i]
    if isolado_em_cinza:
        return cv2.merge([c, c, c])
    saida = np.zeros_like(img)
    saida[:, :, i] = c
    return saida


# Estruturas de descrição
@dataclass
class Param:
    """Parâmetro configurável de um filtro/operação"""

    nome: str
    rotulo: str
    minimo: float = 0.0
    maximo: float = 1.0
    padrao: Any = 0
    passo: float = 1.0          # passo >= 1 -> inteiro
    impar: bool = False         # tamanhos de kernel precisam ser ímpares
    opcoes: list[str] | None = None
    ajuda: str = ""

    def normalizar(self, valor: Any) -> Any:
        if self.opcoes:
            return valor if valor in self.opcoes else self.padrao
        v = min(max(float(valor), self.minimo), self.maximo)
        if self.passo >= 1:
            v = int(round(v))
            if self.impar and v % 2 == 0:
                v = v + 1 if v + 1 <= self.maximo else v - 1
            return v
        return round(round(v / self.passo) * self.passo, 4)

    def texto(self) -> str:
        if self.opcoes:
            base = f"{self.rotulo}: {' | '.join(self.opcoes)} (padrão: {self.padrao})"
        else:
            extra = ", ímpar" if self.impar else ""
            base = f"{self.rotulo}: {self.minimo:g} a {self.maximo:g}{extra} (padrão: {self.padrao:g})"
        return base + (f" - {self.ajuda}" if self.ajuda else "")


@dataclass
class Filtro:
    id: str
    nome: str
    categoria: str
    descricao: str
    func: Callable[[np.ndarray, dict], np.ndarray]
    params: list[Param] = field(default_factory=list)
    pesado: bool = False  # custo alto em vídeo

    def params_padrao(self) -> dict:
        return {p.nome: p.padrao for p in self.params}

    def normalizar(self, valores: dict) -> dict:
        return {p.nome: p.normalizar(valores.get(p.nome, p.padrao)) for p in self.params}

    def texto_descricao(self) -> str:
        linhas = [f"{self.nome}  [{self.categoria}]", "", self.descricao, ""]
        if self.params:
            linhas.append("Parâmetros configuráveis:")
            linhas += [f"  • {p.texto()}" for p in self.params]
        else:
            linhas.append("Parâmetros: nenhum")
        if self.pesado:
            linhas += ["", "Filtro pesado; no modo vídeo pode reduzir o FPS"]
        return "\n".join(linhas)

    def aplicar(self, img: np.ndarray, params: dict | None = None, canal: str = "RGB") -> np.ndarray:
        p = self.normalizar(params or {})
        return aplicar_por_canal(img, canal, lambda x: self.func(x, p))


def _saturar(img_float: np.ndarray) -> np.ndarray:
    return np.clip(img_float, 0, 255).astype(np.uint8)


# Suavização (passa-baixa)
def f_media(img, p):
    k = p["k"]
    return cv2.blur(img, (k, k))


def f_gaussiano(img, p):
    k = p["k"]
    return cv2.GaussianBlur(img, (k, k), p["sigma"])


def f_mediana(img, p):
    return cv2.medianBlur(img, p["k"])


def f_bilateral(img, p):
    return cv2.bilateralFilter(img, p["d"], p["sigma_cor"], p["sigma_espaco"])


def f_movimento(img, p):
    """Desfoque de movimento: kernel com uma linha de 1s girada no ângulo dado"""
    k = p["k"]
    kernel = np.zeros((k, k), np.float32)
    kernel[k // 2, :] = 1.0
    rot = cv2.getRotationMatrix2D(((k - 1) / 2, (k - 1) / 2), p["angulo"], 1.0)
    kernel = cv2.warpAffine(kernel, rot, (k, k))
    kernel /= max(kernel.sum(), 1e-6)
    return cv2.filter2D(img, -1, kernel)


# Realce/bordas (passa-alta)
def f_nitidez(img, p):
    a = p["intensidade"]
    # Com a = 1 é o kernel clássico [[0,-1,0],[-1,5,-1],[0,-1,0]]
    kernel = np.array([[0, -a, 0], [-a, 1 + 4 * a, -a], [0, -a, 0]], np.float32)
    return cv2.filter2D(img, -1, kernel)


def f_unsharp(img, p):
    borrada = cv2.GaussianBlur(img, (0, 0), p["sigma"])
    q = p["quantidade"]
    return cv2.addWeighted(img, 1 + q, borrada, -q, 0)  # f + q·(f - borrada)


def f_laplaciano(img, p):
    lap = cv2.Laplacian(img, cv2.CV_32F, ksize=p["k"])
    if p["modo"] == "Bordas":
        return cv2.convertScaleAbs(lap, alpha=p["ganho"])
    # Realce (Gonzalez & Woods)
    return _saturar(img.astype(np.float32) - p["ganho"] * lap)


def f_sobel(img, p):
    k = p["k"]
    if p["direcao"] == "Horizontal (dx)":
        g = np.abs(cv2.Sobel(img, cv2.CV_32F, 1, 0, ksize=k))
    elif p["direcao"] == "Vertical (dy)":
        g = np.abs(cv2.Sobel(img, cv2.CV_32F, 0, 1, ksize=k))
    else:
        gx = cv2.Sobel(img, cv2.CV_32F, 1, 0, ksize=k)
        gy = cv2.Sobel(img, cv2.CV_32F, 0, 1, ksize=k)
        g = np.sqrt(gx * gx + gy * gy)
    return cv2.convertScaleAbs(g, alpha=p["ganho"])


def f_canny(img, p):
    bordas = cv2.Canny(para_cinza(img), p["limiar1"], p["limiar2"])
    if p["modo"] == "Somente bordas":
        return bordas
    # "Neon": bordas engrossadas e coloridas sobre a imagem escurecida
    base = (para_bgr(img).astype(np.float32) * 0.35).astype(np.uint8)
    grossas = cv2.dilate(bordas, np.ones((2, 2), np.uint8))
    brilho = cv2.GaussianBlur(grossas, (0, 0), 3)
    neon = np.zeros_like(base)
    neon[:, :, 0] = 255  # B
    neon[:, :, 1] = 60   # G
    neon[:, :, 2] = 255  # R  -> magenta
    alfa = (np.maximum(grossas, brilho).astype(np.float32) / 255.0)[:, :, None]
    return _saturar(base * (1 - alfa) + neon * alfa)


def f_relevo(img, p):
    a = p["intensidade"]
    kernel = np.array([[-1, -1, 0], [-1, 0, 1], [0, 1, 1]], np.float32) * a
    if p["modo"] == "Cinza (relevo)":
        r = cv2.filter2D(para_cinza(img).astype(np.float32), -1, kernel) + 128
        return _saturar(r)
    r = img.astype(np.float32) + cv2.filter2D(img.astype(np.float32), -1, kernel)
    return _saturar(r)


# Estatísticos/morfológicos
FORMAS = {"Retângulo": cv2.MORPH_RECT, "Elipse": cv2.MORPH_ELLIPSE, "Cruz": cv2.MORPH_CROSS}


def _elemento(p):
    return cv2.getStructuringElement(FORMAS[p["forma"]], (p["k"], p["k"]))


def f_dilatacao(img, p):
    return cv2.dilate(img, _elemento(p), iterations=p["iteracoes"])


def f_erosao(img, p):
    return cv2.erode(img, _elemento(p), iterations=p["iteracoes"])


def f_gradiente_morf(img, p):
    return cv2.morphologyEx(img, cv2.MORPH_GRADIENT, _elemento(p))


# Cor e intensidade (pixel a pixel)
def f_cinza(img, p):
    if img.ndim == 2:
        return img
    if p["pesos"] == "Média simples":
        pesos = np.array([[1 / 3, 1 / 3, 1 / 3]], np.float32)
    else:  
        pesos = np.array([[0.0721, 0.7154, 0.2125]], np.float32)  # ordem B, G, R
    return cv2.transform(img, pesos)


def f_negativo(img, p):
    return np.bitwise_xor(img, np.uint8(255)) 


_MATRIZ_SEPIA = np.array([[0.131, 0.534, 0.272],   # B'
                          [0.168, 0.686, 0.349],   # G'
                          [0.189, 0.769, 0.393]],  # R'
                         np.float32)


def f_sepia(img, p):
    bgr = para_bgr(img)
    sepia = cv2.transform(bgr, _MATRIZ_SEPIA)
    t = p["intensidade"]
    return cv2.addWeighted(bgr, 1 - t, sepia, t, 0)


def f_brilho_contraste(img, p):
    return _saturar((img.astype(np.float32) - 128.0) * p["contraste"] + 128.0 + p["brilho"])


@lru_cache(maxsize=32)
def _lut_gamma(gamma: float) -> np.ndarray:
    return np.array([((i / 255.0) ** gamma) * 255 for i in range(256)], np.float32).clip(0, 255).astype(np.uint8)


def f_gamma(img, p):
    return cv2.LUT(img, _lut_gamma(float(p["gamma"]))) 


def f_binarizacao(img, p):
    cinza = para_cinza(img)
    if p["modo"] == "Otsu (automático)":
        _, b = cv2.threshold(cinza, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
    elif p["modo"] == "Adaptativo (gaussiano)":
        bloco = p["bloco"]
        b = cv2.adaptiveThreshold(cinza, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C, cv2.THRESH_BINARY, bloco, 5)
    else:
        _, b = cv2.threshold(cinza, p["limiar"], 255, cv2.THRESH_BINARY)
    return b


def f_saturacao(img, p):
    if img.ndim == 2:
        return img  # um único canal não tem matiz/saturação
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    h, s, v = cv2.split(hsv)
    # Matiz do OpenCV vai de 0 a 179 (graus / 2)
    h = ((h.astype(np.int32) + int(round(p["matiz"] / 2))) % 180).astype(np.uint8)
    s = _saturar(s.astype(np.float32) * p["saturacao"])
    return cv2.cvtColor(cv2.merge([h, s, v]), cv2.COLOR_HSV2BGR)


def f_equalizacao(img, p):
    if p["modo"] == "Global (equalizeHist)":
        eq = cv2.equalizeHist
    else:
        clahe = cv2.createCLAHE(clipLimit=p["clip"], tileGridSize=(8, 8))
        eq = clahe.apply
    if img.ndim == 2:
        return eq(img)
    ycc = cv2.cvtColor(img, cv2.COLOR_BGR2YCrCb)  # equaliza só a luminância (Y)
    ycc[:, :, 0] = eq(np.ascontiguousarray(ycc[:, :, 0]))
    return cv2.cvtColor(ycc, cv2.COLOR_YCrCb2BGR)


def f_temperatura(img, p):
    if img.ndim == 2:
        return img
    f = img.astype(np.float32)
    t, tint = p["temperatura"], p["tonalidade"]
    f[:, :, 2] += t          # R
    f[:, :, 0] -= t          # B
    f[:, :, 1] -= tint       # G (tonalidade verde <-> magenta)
    return _saturar(f)


def f_posterizacao(img, p):
    n = p["niveis"]
    passo = 256.0 / n
    q = np.floor(img.astype(np.float32) / passo)
    return _saturar(q * (255.0 / max(n - 1, 1)))


# Efeitos/estilos
@lru_cache(maxsize=8)
def _mascara_vinheta(h: int, w: int, raio: float) -> np.ndarray:
    kx = cv2.getGaussianKernel(w, w * raio)
    ky = cv2.getGaussianKernel(h, h * raio)
    m = ky @ kx.T
    return (m / m.max()).astype(np.float32)


def f_vinheta(img, p):
    h, w = img.shape[:2]
    m = 1.0 - p["forca"] * (1.0 - _mascara_vinheta(h, w, float(p["raio"])))
    if img.ndim == 3:
        m = m[:, :, None]
    return _saturar(img.astype(np.float32) * m)


def f_pixelizar(img, p):
    h, w = img.shape[:2]
    b = p["bloco"]
    pequena = cv2.resize(img, (max(1, w // b), max(1, h // b)), interpolation=cv2.INTER_LINEAR)
    return cv2.resize(pequena, (w, h), interpolation=cv2.INTER_NEAREST)


def f_cartoon(img, p):
    cor = para_bgr(img)
    h, w = cor.shape[:2]
    # bilateral em meia resolução (mais rápido) aplicado várias vezes
    pequena = cv2.resize(cor, (max(1, w // 2), max(1, h // 2)), interpolation=cv2.INTER_AREA)
    for _ in range(p["suavizacao"]):
        pequena = cv2.bilateralFilter(pequena, 9, 40, 7)
    suave = cv2.resize(pequena, (w, h), interpolation=cv2.INTER_LINEAR)
    suave = f_posterizacao(suave, {"niveis": p["niveis"]})
    cinza = cv2.medianBlur(para_cinza(cor), 7)
    bordas = cv2.adaptiveThreshold(cinza, 255, cv2.ADAPTIVE_THRESH_MEAN_C, cv2.THRESH_BINARY, 9, p["bordas"])
    return cv2.bitwise_and(suave, suave, mask=bordas)


_rng = np.random.default_rng()


def f_ruido(img, p):
    q = p["quantidade"]
    if p["tipo"] == "Sal e pimenta":
        saida = img.copy()
        sorteio = _rng.random(img.shape[:2])
        saida[sorteio < q / 2] = 0
        saida[sorteio > 1 - q / 2] = 255
        return saida
    ruido = _rng.normal(0, q * 255, img.shape).astype(np.float32)
    return _saturar(img.astype(np.float32) + ruido)


def f_vintage(img, p):
    t = p["intensidade"]
    saida = f_sepia(img, {"intensidade": 0.55 * t})
    saida = f_brilho_contraste(saida, {"contraste": 1 - 0.15 * t, "brilho": 8 * t})
    if saida.ndim == 3:
        saida = f_temperatura(saida, {"temperatura": 18 * t, "tonalidade": -4 * t})
    saida = f_vinheta(saida, {"forca": 0.55 * t, "raio": 0.55})
    if p["grao"] > 0:
        saida = f_ruido(saida, {"tipo": "Gaussiano", "quantidade": p["grao"]})
    return saida


# Registro de filtros
def _k(padrao=5, maximo=31, minimo=3, rotulo="Tamanho do kernel (k)"):
    return Param("k", rotulo, minimo, maximo, padrao, 1, impar=True)


SUAVIZACAO = "Suavização (passa-baixa)"
REALCE = "Realce e bordas (passa-alta)"
MORFOLOGIA = "Estatísticos / morfológicos"
COR = "Cor e intensidade"
EFEITOS = "Efeitos e estilos"

CATEGORIAS = [SUAVIZACAO, REALCE, MORFOLOGIA, COR, EFEITOS]

FILTROS: list[Filtro] = [
    # Suavização
    Filtro("media", "Média (Box Blur)", SUAVIZACAO,
           "Substitui cada pixel pela média aritmética dos vizinhos dentro de uma janela k×k "
           "(kernel np.ones((k,k))/k²). Remove ruído e detalhes finos, mas também borra as bordas. "
           "Implementado com cv2.blur.",
           f_media, [_k(5)]),
    Filtro("gaussiano", "Gaussiano (Gaussian Blur)", SUAVIZACAO,
           "Média ponderada por uma gaussiana 2D: vizinhos próximos pesam mais que os distantes. "
           "Suaviza de forma mais natural que a média e é muito usado antes de detectar bordas. "
           "Implementado com cv2.GaussianBlur.",
           f_gaussiano, [_k(7), Param("sigma", "Sigma (σ)", 0, 10, 1.5, 0.1,
                                      ajuda="desvio padrão; 0 = calculado a partir de k")]),
    Filtro("mediana", "Mediana", SUAVIZACAO,
           "Filtro estatístico: cada pixel recebe a mediana dos vizinhos. Excelente para remover ruído "
           "'sal e pimenta' preservando as bordas (experimente aplicar o filtro Ruído antes). "
           "Implementado com cv2.medianBlur.",
           f_mediana, [_k(5, 15)]),
    Filtro("bilateral", "Bilateral (preserva bordas)", SUAVIZACAO,
           "Suavização que considera distância espacial E diferença de cor: suaviza regiões "
           "homogêneas (efeito 'pele lisa') mantendo as bordas nítidas. cv2.bilateralFilter.",
           f_bilateral, [Param("d", "Diâmetro da vizinhança (d)", 3, 15, 9, 1),
                         Param("sigma_cor", "Sigma de cor", 10, 200, 75, 1),
                         Param("sigma_espaco", "Sigma espacial", 10, 200, 75, 1)], pesado=True),
    Filtro("movimento", "Desfoque de movimento", SUAVIZACAO,
           "Convolução com um kernel em forma de linha (média ao longo de uma direção), simulando "
           "o rastro de um objeto em movimento. Kernel criado com NumPy e aplicado com cv2.filter2D.",
           f_movimento, [_k(15, 51, 3, "Comprimento do rastro (k)"),
                         Param("angulo", "Ângulo (graus)", -90, 90, 0, 1)]),
    # Realce/bordas
    Filtro("nitidez", "Nitidez (Sharpen)", REALCE,
           "Realça detalhes somando à imagem a diferença entre o pixel e seus vizinhos. Com "
           "intensidade 1 usa o kernel clássico [[0,-1,0],[-1,5,-1],[0,-1,0]] via cv2.filter2D.",
           f_nitidez, [Param("intensidade", "Intensidade", 0.1, 3, 1.0, 0.1)]),
    Filtro("unsharp", "Máscara de nitidez (Unsharp Mask)", REALCE,
           "Técnica clássica de fotografia: subtrai uma versão borrada da imagem para obter os "
           "detalhes (f − borrada) e soma-os de volta amplificados: g = f + q·(f − borrada).",
           f_unsharp, [Param("sigma", "Raio do desfoque (σ)", 0.5, 10, 3, 0.1),
                       Param("quantidade", "Quantidade (q)", 0, 4, 1.0, 0.1)]),
    Filtro("laplaciano", "Laplaciano", REALCE,
           "Derivada de 2ª ordem (∇²f): responde a variações bruscas de intensidade em todas as "
           "direções. Modo 'Bordas' mostra |∇²f|; modo 'Realce' aplica g = f − c·∇²f (nitidez, "
           "Gonzalez & Woods). cv2.Laplacian.",
           f_laplaciano, [Param("modo", "Modo", padrao="Bordas", opcoes=["Bordas", "Realce (f − c·∇²f)"]),
                          Param("k", "Abertura (ksize)", 1, 7, 3, 1, impar=True),
                          Param("ganho", "Ganho (c)", 0.1, 4, 1.0, 0.1)]),
    Filtro("sobel", "Sobel (gradiente)", REALCE,
           "Derivada de 1ª ordem: detecta bordas verticais (dx), horizontais (dy) ou a magnitude "
           "do gradiente √(gx² + gy²). cv2.Sobel.",
           f_sobel, [Param("direcao", "Direção", padrao="Magnitude",
                           opcoes=["Horizontal (dx)", "Vertical (dy)", "Magnitude"]),
                     Param("k", "Abertura (ksize)", 1, 7, 3, 1, impar=True),
                     Param("ganho", "Ganho", 0.1, 4, 1.0, 0.1)]),
    Filtro("canny", "Bordas de Canny / Neon", REALCE,
           "Detector de bordas de Canny (gradiente + supressão de não-máximos + histerese com dois "
           "limiares). Modo 'Neon' desenha as bordas brilhando sobre a imagem escurecida.",
           f_canny, [Param("modo", "Modo", padrao="Somente bordas", opcoes=["Somente bordas", "Neon sobre a imagem"]),
                     Param("limiar1", "Limiar inferior", 0, 255, 80, 1),
                     Param("limiar2", "Limiar superior", 0, 255, 160, 1)]),
    Filtro("relevo", "Relevo (Emboss)", REALCE,
           "Kernel direcional [[-1,-1,0],[-1,0,1],[0,1,1]] que destaca bordas numa diagonal, dando "
           "aparência de baixo-relevo. Modo 'Cinza' soma 128 ao resultado (efeito de gravura).",
           f_relevo, [Param("modo", "Modo", padrao="Colorido", opcoes=["Colorido", "Cinza (relevo)"]),
                      Param("intensidade", "Intensidade", 0.2, 4, 1.0, 0.1)]),
    # Morfologia
    Filtro("dilatacao", "Máximo (Dilatação)", MORFOLOGIA,
           "Filtro de máximo: cada pixel recebe o MAIOR valor da vizinhança definida pelo elemento "
           "estruturante. Expande regiões claras e preenche pequenos buracos escuros. cv2.dilate.",
           f_dilatacao, [_k(3, 21), Param("forma", "Forma do elemento", padrao="Retângulo", opcoes=list(FORMAS)),
                         Param("iteracoes", "Iterações", 1, 5, 1, 1)]),
    Filtro("erosao", "Mínimo (Erosão)", MORFOLOGIA,
           "Filtro de mínimo: cada pixel recebe o MENOR valor da vizinhança. Expande regiões escuras "
           "e encolhe as claras (remove pontos brancos isolados). cv2.erode.",
           f_erosao, [_k(3, 21), Param("forma", "Forma do elemento", padrao="Retângulo", opcoes=list(FORMAS)),
                      Param("iteracoes", "Iterações", 1, 5, 1, 1)]),
    Filtro("gradiente_morf", "Gradiente morfológico", MORFOLOGIA,
           "Diferença entre dilatação e erosão (máximo − mínimo): resulta no contorno dos objetos. "
           "cv2.morphologyEx(MORPH_GRADIENT).",
           f_gradiente_morf, [_k(3, 21), Param("forma", "Forma do elemento", padrao="Elipse", opcoes=list(FORMAS))]),
    # Cor/intensidade
    Filtro("cinza", "Escala de cinza", COR,
           "Converte para tons de cinza. 'Luminância' usa a sensibilidade do olho humano "
           "(0.2125R + 0.7154G + 0.0721B); 'Média simples' usa (R+G+B)/3.",
           f_cinza, [Param("pesos", "Pesos", padrao="Luminância (olho humano)",
                           opcoes=["Luminância (olho humano)", "Média simples"])]),
    Filtro("negativo", "Negativo", COR,
           "Inverte as intensidades: cada canal recebe canal XOR 255 (equivale a 255 − valor). "
           "Escolha um único canal (R, G ou B) para inverter só aquele canal.",
           f_negativo),
    Filtro("sepia", "Sépia", COR,
           "Tom envelhecido de fotografia antiga, obtido por uma matriz 3×3 que recombina R, G e B "
           "(cv2.transform), misturada à original conforme a intensidade.",
           f_sepia, [Param("intensidade", "Intensidade", 0, 1, 1.0, 0.05)]),
    Filtro("brilho_contraste", "Brilho e contraste", COR,
           "Transformação linear de intensidade g = α·(f − 128) + 128 + β, com saturação em [0, 255].",
           f_brilho_contraste, [Param("contraste", "Contraste (α)", 0, 3, 1.3, 0.05),
                                Param("brilho", "Brilho (β)", -100, 100, 10, 1)]),
    Filtro("gamma", "Correção gama", COR,
           "Transformação de potência s = 255·(r/255)^γ aplicada por tabela (cv2.LUT). γ < 1 clareia "
           "as sombras; γ > 1 escurece.",
           f_gamma, [Param("gamma", "Gama (γ)", 0.1, 4, 0.6, 0.05)]),
    Filtro("binarizacao", "Binarização (limiar)", COR,
           "Transforma em preto e branco: pixels acima do limiar viram 255, abaixo viram 0. "
           "Também há limiar automático (Otsu) e adaptativo (local).",
           f_binarizacao, [Param("modo", "Modo", padrao="Limiar fixo",
                                 opcoes=["Limiar fixo", "Otsu (automático)", "Adaptativo (gaussiano)"]),
                           Param("limiar", "Limiar", 0, 255, 127, 1),
                           Param("bloco", "Bloco (adaptativo)", 3, 99, 31, 1, impar=True)]),
    Filtro("saturacao", "Saturação e matiz (HSV)", COR,
           "Converte para HSV, multiplica o canal S (saturação) por um fator e gira o canal H (matiz), "
           "voltando para BGR. Fator > 1 deixa as cores mais vibrantes; 0 deixa cinza.",
           f_saturacao, [Param("saturacao", "Fator de saturação", 0, 3, 1.5, 0.05),
                         Param("matiz", "Deslocamento de matiz (graus)", -180, 180, 0, 1)]),
    Filtro("equalizacao", "Equalização de histograma", COR,
           "Redistribui as intensidades para usar toda a faixa [0, 255], aumentando o contraste. "
           "CLAHE equaliza por blocos com limite de contraste (bom para imagens médicas).",
           f_equalizacao, [Param("modo", "Modo", padrao="CLAHE (adaptativa)",
                                 opcoes=["CLAHE (adaptativa)", "Global (equalizeHist)"]),
                           Param("clip", "Limite de contraste (CLAHE)", 1, 10, 2.5, 0.1)]),
    Filtro("temperatura", "Temperatura de cor", COR,
           "Esquenta (mais vermelho, menos azul) ou esfria a imagem, e ajusta a tonalidade "
           "verde/magenta. Somas por canal com saturação.",
           f_temperatura, [Param("temperatura", "Temperatura (frio ↔ quente)", -100, 100, 30, 1),
                           Param("tonalidade", "Tonalidade (magenta ↔ verde)", -60, 60, 0, 1)]),
    Filtro("posterizacao", "Posterização", COR,
           "Quantiza cada canal em poucos níveis de intensidade, criando áreas chapadas de cor "
           "(efeito pôster/pop-art).",
           f_posterizacao, [Param("niveis", "Níveis por canal", 2, 16, 4, 1)]),
    # Efeitos
    Filtro("vinheta", "Vinheta", EFEITOS,
           "Escurece as bordas da imagem multiplicando-a por uma máscara gaussiana centrada, "
           "direcionando o olhar para o centro.",
           f_vinheta, [Param("forca", "Força", 0, 1, 0.7, 0.05),
                       Param("raio", "Raio", 0.2, 1.5, 0.5, 0.05)]),
    Filtro("pixelizar", "Pixelizar (mosaico)", EFEITOS,
           "Reduz a resolução e amplia de novo com interpolação por vizinho mais próximo, formando "
           "blocos (efeito 8-bit/censura).",
           f_pixelizar, [Param("bloco", "Tamanho do bloco (px)", 2, 64, 12, 1)]),
    Filtro("cartoon", "Cartoon", EFEITOS,
           "Combina suavização bilateral repetida + posterização das cores + bordas por limiar "
           "adaptativo, imitando um desenho animado.",
           f_cartoon, [Param("suavizacao", "Passadas do bilateral", 1, 6, 3, 1),
                       Param("niveis", "Níveis de cor", 2, 16, 8, 1),
                       Param("bordas", "Sensibilidade das bordas (C)", 1, 20, 6, 1)], pesado=True),
    Filtro("ruido", "Ruído (sal e pimenta / gaussiano)", EFEITOS,
           "Adiciona ruído artificial: 'sal e pimenta' (pixels pretos e brancos aleatórios) ou "
           "gaussiano (grão). Útil para testar os filtros de suavização (ex.: mediana).",
           f_ruido, [Param("tipo", "Tipo", padrao="Sal e pimenta", opcoes=["Sal e pimenta", "Gaussiano"]),
                     Param("quantidade", "Quantidade", 0, 0.5, 0.05, 0.01)]),
    Filtro("vintage", "Vintage (estilo stories)", EFEITOS,
           "Filtro composto inspirado em apps de fotos: sépia parcial + menos contraste + "
           "temperatura quente + vinheta + grão de filme.",
           f_vintage, [Param("intensidade", "Intensidade", 0, 1, 0.8, 0.05),
                       Param("grao", "Grão de filme", 0, 0.3, 0.06, 0.01)]),
]

FILTROS_POR_ID: dict[str, Filtro] = {f.id: f for f in FILTROS}
