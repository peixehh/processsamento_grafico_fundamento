"""
Leitura/gravação de imagens e utilitários de conversão.
"""
from __future__ import annotations

import os

import cv2
import numpy as np

EXTENSOES_IMAGEM = (".png", ".jpg", ".jpeg", ".bmp", ".tif", ".tiff", ".webp")


def ler_imagem(caminho: str, flags: int = cv2.IMREAD_COLOR) -> np.ndarray | None:
    """Lê uma imagem do disco (suporta caminhos com acentos)."""
    try:
        dados = np.fromfile(caminho, dtype=np.uint8)
    except OSError:
        return None
    if dados.size == 0:
        return None
    return cv2.imdecode(dados, flags)


def salvar_imagem(caminho: str, img: np.ndarray) -> bool:
    """Salva uma imagem no disco (suporta caminhos com acentos)."""
    ext = os.path.splitext(caminho)[1].lower() or ".png"
    if ext not in EXTENSOES_IMAGEM:
        ext = ".png"
        caminho += ext
    ok, buffer = cv2.imencode(ext, img)
    if not ok:
        return False
    buffer.tofile(caminho)
    return True


def para_bgr(img: np.ndarray) -> np.ndarray:
    """Garante imagem BGR de 3 canais (converte cinza/BGRA)."""
    if img.ndim == 2:
        return cv2.cvtColor(img, cv2.COLOR_GRAY2BGR)
    if img.shape[2] == 4:
        return cv2.cvtColor(img, cv2.COLOR_BGRA2BGR)
    return img


def para_cinza(img: np.ndarray) -> np.ndarray:
    """Garante imagem de 1 canal (tons de cinza)."""
    if img.ndim == 2:
        return img
    if img.shape[2] == 4:
        img = img[:, :, :3]
    return cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)


def limitar_tamanho(img: np.ndarray, lado_max: int = 1920) -> np.ndarray:
    """Reduz a imagem (mantendo proporção) se o maior lado passar de lado_max."""
    h, w = img.shape[:2]
    maior = max(h, w)
    if maior <= lado_max:
        return img
    escala = lado_max / maior
    return cv2.resize(img, (round(w * escala), round(h * escala)), interpolation=cv2.INTER_AREA)


def ajustar_cobrindo(img: np.ndarray, altura: int, largura: int) -> np.ndarray:
    """Redimensiona para cobrir (altura x largura) e recorta o centro (sem distorcer)."""
    h, w = img.shape[:2]
    if (h, w) == (altura, largura):
        return img
    escala = max(largura / w, altura / h)
    nw, nh = max(largura, round(w * escala)), max(altura, round(h * escala))
    interp = cv2.INTER_AREA if escala < 1 else cv2.INTER_LINEAR
    img = cv2.resize(img, (nw, nh), interpolation=interp)
    x0 = (nw - largura) // 2
    y0 = (nh - altura) // 2
    return np.ascontiguousarray(img[y0:y0 + altura, x0:x0 + largura])
