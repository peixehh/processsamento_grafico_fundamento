"""
Stickers: imagens PNG com canal alfa (estáticas ou animadas por spritesheet).

Composição com transparência (fórmula dos slides):
    C = c_sticker · α + c_fundo · (1 − α)

Stickers animados usam uma spritesheet (quadros lado a lado). O quadro atual
é escolhido pelo tempo: iFrame = int(t · fps) % nFrames (slides de sprites).
"""
from __future__ import annotations

import json
import math
import os
import time
from dataclasses import dataclass, field

import cv2
import numpy as np

from .console import log
from .io_utils import ler_imagem


@dataclass(eq=False)
class Sticker:
    id: str
    nome: str
    quadros: list[np.ndarray]   # imagens BGRA
    fps: float = 0.0
    pixel_art: bool = False     # redimensiona com vizinho mais próximo (mantém os "pixels")

    @property
    def animado(self) -> bool:
        return len(self.quadros) > 1

    @property
    def proporcao(self) -> float:
        h, w = self.quadros[0].shape[:2]
        return h / w

    def indice_quadro(self, t: float) -> int:
        if not self.animado:
            return 0
        return int(t * self.fps) % len(self.quadros)


def _fatiar_spritesheet(img: np.ndarray, colunas: int, linhas: int) -> list[np.ndarray]:
    h, w = img.shape[:2]
    qw, qh = w // colunas, h // linhas
    quadros = []
    for lin in range(linhas):
        for col in range(colunas):
            quadros.append(np.ascontiguousarray(img[lin * qh:(lin + 1) * qh, col * qw:(col + 1) * qw]))
    return quadros


def carregar_stickers(pasta: str) -> list[Sticker]:
    """Carrega os stickers listados em <pasta>/stickers.json."""
    manifesto = os.path.join(pasta, "stickers.json")
    if not os.path.exists(manifesto):
        return []
    with open(manifesto, encoding="utf-8") as f:
        itens = json.load(f)
    stickers = []
    for item in itens:
        img = ler_imagem(os.path.join(pasta, item["arquivo"]), cv2.IMREAD_UNCHANGED)
        if img is None:
            log("stickers", f"não foi possível ler {item['arquivo']}")
            continue
        if img.ndim == 2:
            img = cv2.cvtColor(img, cv2.COLOR_GRAY2BGRA)
        elif img.shape[2] == 3: 
            img = cv2.cvtColor(img, cv2.COLOR_BGR2BGRA)
        colunas = int(item.get("quadros", 1))
        linhas = int(item.get("linhas", 1))
        quadros = _fatiar_spritesheet(img, colunas, linhas) if colunas * linhas > 1 else [img]
        quadros = _recortar_margens(quadros)
        stickers.append(Sticker(item["id"], item["nome"], quadros, float(item.get("fps", 0)),
                                bool(item.get("pixel_art", False))))
    return stickers


def _recortar_margens(quadros: list[np.ndarray], folga: int = 2) -> list[np.ndarray]:
    """Remove as bordas totalmente transparentes (mesmo recorte para todos os quadros).

    Emojis baixados costumam ter muita margem vazia; sem o recorte o tamanho e a
    área de clique do sticker ficariam maiores que o desenho.
    """
    alfa = np.max(np.stack([q[:, :, 3] for q in quadros]), axis=0)
    ys, xs = np.nonzero(alfa > 8)
    if ys.size == 0:
        return quadros
    h, w = alfa.shape
    y0, y1 = max(0, ys.min() - folga), min(h, ys.max() + 1 + folga)
    x0, x1 = max(0, xs.min() - folga), min(w, xs.max() + 1 + folga)
    return [np.ascontiguousarray(q[y0:y1, x0:x1]) for q in quadros]


# Transformação e composição
_cache_transformados: dict = {}


def transformar(sticker: Sticker, quadro: int, largura: int, angulo: float) -> np.ndarray:
    """Redimensiona o quadro para `largura` px e gira `angulo` graus (anti-horário)."""
    largura = max(4, int(largura))
    angulo = round(angulo, 1)
    chave = (id(sticker), quadro, largura, angulo)
    if chave in _cache_transformados:
        return _cache_transformados[chave]
    src = sticker.quadros[quadro]
    altura = max(4, int(round(largura * sticker.proporcao)))
    if sticker.pixel_art:
        interp = cv2.INTER_NEAREST  # mantém os blocos nítidos da pixel art
    else:
        interp = cv2.INTER_AREA if largura < src.shape[1] else cv2.INTER_LINEAR
    img = cv2.resize(src, (largura, altura), interpolation=interp)
    if abs(angulo) > 0.05:
        # canvas maior para não cortar as pontas ao girar
        rad = math.radians(angulo)
        nw = int(abs(largura * math.cos(rad)) + abs(altura * math.sin(rad))) + 2
        nh = int(abs(largura * math.sin(rad)) + abs(altura * math.cos(rad))) + 2
        m = cv2.getRotationMatrix2D((largura / 2, altura / 2), angulo, 1.0)
        m[0, 2] += nw / 2 - largura / 2
        m[1, 2] += nh / 2 - altura / 2
        img = cv2.warpAffine(img, m, (nw, nh), flags=cv2.INTER_NEAREST if sticker.pixel_art else cv2.INTER_LINEAR,
                             borderMode=cv2.BORDER_CONSTANT, borderValue=(0, 0, 0, 0))
    if len(_cache_transformados) > 256:
        _cache_transformados.clear()
    _cache_transformados[chave] = img
    return img


def sobrepor(fundo: np.ndarray, sprite: np.ndarray, cx: float, cy: float) -> None:
    """Desenha `sprite` (BGRA) centralizado em (cx, cy) sobre `fundo` (BGR), in-place.

    Recorta automaticamente as partes que saem da imagem.
    """
    H, W = fundo.shape[:2]
    h, w = sprite.shape[:2]
    x0 = int(round(cx - w / 2))
    y0 = int(round(cy - h / 2))
    fx0, fy0 = max(x0, 0), max(y0, 0)
    fx1, fy1 = min(x0 + w, W), min(y0 + h, H)
    if fx0 >= fx1 or fy0 >= fy1:
        return
    sp = sprite[fy0 - y0:fy1 - y0, fx0 - x0:fx1 - x0]
    alfa = sp[:, :, 3:4].astype(np.float32) / 255.0
    roi = fundo[fy0:fy1, fx0:fx1]
    mistura = sp[:, :, :3].astype(np.float32) * alfa + roi.astype(np.float32) * (1.0 - alfa)
    roi[:] = mistura.astype(np.uint8)


@dataclass(eq=False)
class StickerColocado:
    """Um sticker posicionado na imagem.

    A posição (u, v) é relativa (0..1) à largura/altura da imagem, assim o
    sticker continua no mesmo lugar independentemente da resolução exibida,
    salva ou do frame da webcam.
    """

    sticker: Sticker
    u: float
    v: float
    escala: float = 0.25     # largura do sticker como fração da largura da imagem
    angulo: float = 0.0      # graus, anti-horário
    t0: float = field(default_factory=time.monotonic)

    def largura_px(self, largura_img: int) -> int:
        return max(4, int(self.escala * largura_img))

    def desenhar(self, fundo: np.ndarray, agora: float | None = None) -> None:
        agora = time.monotonic() if agora is None else agora
        H, W = fundo.shape[:2]
        quadro = self.sticker.indice_quadro(agora - self.t0)
        sprite = transformar(self.sticker, quadro, self.largura_px(W), self.angulo)
        sobrepor(fundo, sprite, self.u * W, self.v * H)

    def contem(self, u: float, v: float, W: int, H: int) -> bool:
        w = self.largura_px(W)
        h = w * self.sticker.proporcao
        # teste no sistema de coordenadas do sticker (considera a rotação)
        rad = math.radians(self.angulo)
        dx, dy = (u - self.u) * W, (v - self.v) * H
        lx = dx * math.cos(rad) - dy * math.sin(rad)
        ly = dx * math.sin(rad) + dy * math.cos(rad)
        return abs(lx) <= w / 2 and abs(ly) <= h / 2
