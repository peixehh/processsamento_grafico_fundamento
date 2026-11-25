"""
Operações aritméticas entre duas imagens (A = imagem atual, B = segunda imagem).

A imagem B é ajustada ao tamanho de A (redimensiona cobrindo e recorta o centro).
Todas as operações trabalham com uint8 e saturam o resultado em [0, 255].

Também gera texturas procedurais que podem ser usadas como imagem B
(gradientes, luz vazada, no estilo dos apps de stories
"""
from __future__ import annotations

from dataclasses import dataclass, field
from functools import lru_cache
from typing import Callable

import cv2
import numpy as np

from .filtros import Param, aplicar_por_canal


@dataclass
class Operacao:
    id: str
    nome: str
    descricao: str
    func: Callable[[np.ndarray, np.ndarray, dict], np.ndarray]
    params: list[Param] = field(default_factory=list)
    categoria: str = "Operações com duas imagens"

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
            linhas.append("Parâmetros: nenhum.")
        return "\n".join(linhas)

    def aplicar(self, a: np.ndarray, b: np.ndarray, params: dict | None = None, canal: str = "RGB") -> np.ndarray:
        p = self.normalizar(params or {})
        return aplicar_por_canal(a, canal, lambda x, y: self.func(x, y, p), b)


def _saturar(x):
    return np.clip(x, 0, 255).astype(np.uint8)


# Operações
def op_adicao(a, b, p):
    return cv2.addWeighted(a, 1.0, b, p["k"], 0)          # A + k·B (saturado)


def op_subtracao_ponderada(a, b, p):
    return cv2.addWeighted(a, p["alfa"], b, -p["beta"], p["gama"])  # α·A − β·B + γ


def op_blending(a, b, p):
    alfa = p["alfa"]
    return cv2.addWeighted(a, alfa, b, 1 - alfa, 0)       # C = α·A + (1 − α)·B


def op_diferenca(a, b, p):
    return cv2.convertScaleAbs(cv2.absdiff(a, b), alpha=p["ganho"])  # |A − B|


def op_tela(a, b, p):
    inv = cv2.multiply(255 - a, 255 - b, scale=1 / 255.0)
    tela = 255 - inv                                      
    return cv2.addWeighted(a, 1 - p["intensidade"], tela, p["intensidade"], 0)


OPERACOES: list[Operacao] = [
    Operacao("adicao", "Adição (A + k·B)",
             "Soma pixel a pixel as duas imagens: C = A + k·B, saturando em 255. Clareia a imagem "
             "e sobrepõe as duas (cv2.addWeighted).",
             op_adicao, [Param("k", "Peso de B (k)", 0, 2, 1.0, 0.05)]),
    Operacao("subtracao_ponderada", "Subtração ponderada (α·A − β·B + γ)",
             "Combinação linear com pesos: C = α·A − β·B + γ. O γ (deslocamento) permite trazer "
             "para a faixa visível os valores que ficariam negativos.",
             op_subtracao_ponderada, [Param("alfa", "Peso de A (α)", 0, 2, 1.0, 0.05),
                                      Param("beta", "Peso de B (β)", 0, 2, 0.5, 0.05),
                                      Param("gama", "Deslocamento (γ)", -128, 128, 0, 1)]),
    Operacao("blending", "Blending (mistura alfa)",
             "Mistura com transparência (fórmula dos slides): C = α·A + (1 − α)·B. Com α = 0.5 "
             "as duas imagens aparecem com 50% de opacidade.",
             op_blending, [Param("alfa", "Opacidade de A (α)", 0, 1, 0.5, 0.01)]),
    Operacao("diferenca", "Diferença absoluta |A − B|",
             "Valor absoluto da diferença. Regiões iguais ficam pretas; regiões diferentes ficam "
             "claras. No modo vídeo, use 'Usar imagem atual como B' para detectar movimento.",
             op_diferenca, [Param("ganho", "Ganho", 0.5, 8, 1.0, 0.1)]),
    Operacao("tela", "Tela / Screen",
             "Inverso da multiplicação: C = 255 − (255 − A)(255 − B)/255. Clareia; ótimo para "
             "aplicar texturas de luz (light leak).",
             op_tela, [Param("intensidade", "Intensidade", 0, 1, 1.0, 0.05)]),
]

OPERACOES_POR_ID: dict[str, Operacao] = {o.id: o for o in OPERACOES}


# Texturas geradas (podem ser usadas como imagem B)
def _grade(h, w):
    y, x = np.mgrid[0:h, 0:w].astype(np.float32)
    return x / max(w - 1, 1), y / max(h - 1, 1)


def _gradiente(h, w, cor_topo, cor_base):
    _, y = _grade(h, w)
    c1 = np.array(cor_topo, np.float32)
    c2 = np.array(cor_base, np.float32)
    img = c1[None, None, :] * (1 - y[:, :, None]) + c2[None, None, :] * y[:, :, None]
    return _saturar(img)


def tex_arco_iris(h, w):
    x, y = _grade(h, w)
    hsv = np.zeros((h, w, 3), np.uint8)
    hsv[:, :, 0] = ((x * 0.6 + y * 0.4) * 179).astype(np.uint8)
    hsv[:, :, 1] = 200
    hsv[:, :, 2] = 255
    return cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)


def tex_por_do_sol(h, w):
    return _gradiente(h, w, (85, 16, 43), (95, 126, 255))  # RGB: roxo -> laranja


def tex_oceano(h, w):
    return _gradiente(h, w, (230, 200, 60), (90, 40, 10))


def tex_luz_vazada(h, w):
    img = np.zeros((h, w, 3), np.float32)
    rng = np.random.default_rng(7)
    cores = [(40, 120, 255), (120, 40, 255), (60, 200, 255), (200, 80, 255)]  # laranja/rosa/amarelo
    for i, cor in enumerate(cores):
        cx = rng.uniform(-0.1, 0.4) * w if i % 2 == 0 else rng.uniform(0.6, 1.1) * w
        cy = rng.uniform(0, 1) * h
        raio = rng.uniform(0.25, 0.45) * max(h, w)
        x, y = np.meshgrid(np.arange(w, dtype=np.float32), np.arange(h, dtype=np.float32))
        g = np.exp(-((x - cx) ** 2 + (y - cy) ** 2) / (2 * (raio / 2) ** 2))
        img += g[:, :, None] * np.array(cor, np.float32)[None, None, :]
    return _saturar(img)


def tex_vinheta_radial(h, w):
    x, y = _grade(h, w)
    d = np.sqrt((x - 0.5) ** 2 + (y - 0.5) ** 2) / 0.7071
    v = _saturar((1 - d ** 1.5) * 255)
    return cv2.cvtColor(v, cv2.COLOR_GRAY2BGR)


def tex_grao(h, w):
    rng = np.random.default_rng(3)
    ruido = rng.normal(128, 45, (h, w)).astype(np.float32)
    ruido = cv2.GaussianBlur(ruido, (0, 0), 0.8)
    return cv2.cvtColor(_saturar(ruido), cv2.COLOR_GRAY2BGR)


def tex_bokeh(h, w):
    rng = np.random.default_rng(11)
    img = np.zeros((h, w, 3), np.uint8)
    img[:] = (40, 15, 25)
    for _ in range(45):
        c = (int(rng.uniform(0, w)), int(rng.uniform(0, h)))
        r = int(rng.uniform(0.02, 0.08) * max(h, w))
        hue = int(rng.uniform(0, 179))
        cor_hsv = np.uint8([[[hue, 160, 255]]])
        cor = tuple(int(v) for v in cv2.cvtColor(cor_hsv, cv2.COLOR_HSV2BGR)[0, 0])
        camada = img.copy()
        cv2.circle(camada, c, r, cor, -1, cv2.LINE_AA)
        img = cv2.addWeighted(img, 0.6, camada, 0.4, 0)
    return cv2.GaussianBlur(img, (0, 0), max(h, w) * 0.006)


def tex_xadrez(h, w):
    n = 8
    x, y = _grade(h, w)
    m = ((np.floor(x * n) + np.floor(y * n * h / max(w, 1))) % 2).astype(np.uint8) * 255
    return cv2.cvtColor(m, cv2.COLOR_GRAY2BGR)


TEXTURAS: dict[str, Callable[[int, int], np.ndarray]] = {
    "Gradiente arco-íris": tex_arco_iris,
    "Pôr do sol (gradiente)": tex_por_do_sol,
    "Oceano (gradiente)": tex_oceano,
    "Luz vazada (light leak)": tex_luz_vazada,
    "Bokeh (luzes desfocadas)": tex_bokeh,
    "Vinheta radial": tex_vinheta_radial,
    "Grão de filme (ruído)": tex_grao,
    "Xadrez": tex_xadrez,
}


@lru_cache(maxsize=16)
def gerar_textura(nome: str, h: int = 1280, w: int = 720) -> np.ndarray:
    img = TEXTURAS[nome](h, w)
    img.flags.writeable = False
    return img
