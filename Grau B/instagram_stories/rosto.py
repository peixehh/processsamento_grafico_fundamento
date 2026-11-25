"""
Detecção de face e "filtros de rosto" (estilo filtros dos stories)
"""
from __future__ import annotations

import math
import os
import shutil
import tempfile
from dataclasses import dataclass

import cv2
import numpy as np

from .io_utils import para_cinza
from .stickers import Sticker, sobrepor, transformar

MODELO_YUNET = "face_detection_yunet_2023mar.onnx"
CASCADE_HAAR = "haarcascade_frontalface_default.xml"


@dataclass
class Rosto:
    x: float
    y: float
    w: float
    h: float
    pontos: np.ndarray | None = None  # 5x2: olho (esq. na imagem), olho (dir.), nariz, boca esq., boca dir.

    def vetor(self) -> np.ndarray:
        base = np.array([self.x, self.y, self.w, self.h], np.float32)
        return base if self.pontos is None else np.concatenate([base, self.pontos.ravel()])

    @staticmethod
    def de_vetor(v: np.ndarray, com_pontos: bool) -> "Rosto":
        return Rosto(*[float(a) for a in v[:4]], v[4:].reshape(5, 2).copy() if com_pontos else None)

    @property
    def centro(self) -> tuple[float, float]:
        return self.x + self.w / 2, self.y + self.h / 2

    @property
    def angulo_rad(self) -> float:
        """Inclinação da cabeça (linha entre os olhos); 0 se não há pontos."""
        if self.pontos is None:
            return 0.0
        (x1, y1), (x2, y2) = self.pontos[0], self.pontos[1]
        return math.atan2(y2 - y1, x2 - x1)

    @property
    def para_cima(self) -> np.ndarray:
        """Vetor unitário 'para cima' da cabeça, em coordenadas de imagem (y para baixo)."""
        a = self.angulo_rad
        return np.array([math.sin(a), -math.cos(a)], np.float32)

    def ancora(self, nome: str) -> np.ndarray:
        """Pontos de referência do rosto usados para posicionar stickers."""
        x, y, w, h = self.x, self.y, self.w, self.h
        if self.pontos is not None:
            olho_e, olho_d, nariz, boca_e, boca_d = (p.astype(np.float32) for p in self.pontos)
        else:  
            olho_e = np.array([x + 0.31 * w, y + 0.38 * h], np.float32)
            olho_d = np.array([x + 0.69 * w, y + 0.38 * h], np.float32)
            nariz = np.array([x + 0.50 * w, y + 0.60 * h], np.float32)
            boca_e = np.array([x + 0.36 * w, y + 0.78 * h], np.float32)
            boca_d = np.array([x + 0.64 * w, y + 0.78 * h], np.float32)
        olhos = (olho_e + olho_d) / 2
        boca = (boca_e + boca_d) / 2
        if nome == "olhos":
            return olhos
        if nome == "olho_e":
            return olho_e
        if nome == "olho_d":
            return olho_d
        if nome == "nariz":
            return nariz
        if nome == "boca":
            return boca
        if nome == "buco":  # entre o nariz e a boca (bigode)
            return (nariz + boca) / 2
        if nome == "topo":  # alto da cabeça
            return olhos + self.para_cima * (0.55 * h)
        return np.array(self.centro, np.float32)


class DetectorRosto:
    """Detector de faces com fallback YuNet -> Haar Cascade."""

    def __init__(self, pasta_modelos: str, lado_deteccao: int = 480) -> None:
        self.pasta_modelos = pasta_modelos
        self.lado_deteccao = lado_deteccao
        self.backend = "nenhum"
        self.mensagem = ""
        self._yunet = None
        self._haar = None
        self._tam_yunet = None
        self._carregar()

    @property
    def disponivel(self) -> bool:
        return self.backend != "nenhum"

    def _caminho_ascii(self, caminho: str) -> str:
        """Algumas funções do OpenCV não abrem caminhos com acento no Windows."""
        try:
            caminho.encode("ascii")
            return caminho
        except UnicodeEncodeError:
            destino = os.path.join(tempfile.gettempdir(), "instagram_stories_" + os.path.basename(caminho))
            if not os.path.exists(destino) or os.path.getsize(destino) != os.path.getsize(caminho):
                shutil.copyfile(caminho, destino)
            return destino

    def _carregar(self) -> None:
        # 1) YuNet
        modelo = os.path.join(self.pasta_modelos, MODELO_YUNET)
        if os.path.exists(modelo) and hasattr(cv2, "FaceDetectorYN"):
            try:
                try:  # o DNN do OpenCV 5 emite um aviso inofensivo ao criar a rede
                    cv2.utils.logging.setLogLevel(cv2.utils.logging.LOG_LEVEL_ERROR)
                except AttributeError:
                    pass
                self._yunet = cv2.FaceDetectorYN.create(self._caminho_ascii(modelo), "", (320, 320), 0.75, 0.3, 20)
                self.backend = "YuNet (DNN)"
                self.mensagem = "Detecção de face: YuNet (cv2.FaceDetectorYN)."
                return
            except cv2.error as e:
                self.mensagem = f"Falha ao carregar YuNet: {e}"
        # 2) Haar Cascade
        if hasattr(cv2, "CascadeClassifier"):
            candidatos = [os.path.join(self.pasta_modelos, CASCADE_HAAR)]
            dados = getattr(getattr(cv2, "data", None), "haarcascades", None)
            if dados:
                candidatos.append(os.path.join(dados, CASCADE_HAAR))
            for c in candidatos:
                if os.path.exists(c):
                    haar = cv2.CascadeClassifier(self._caminho_ascii(c))
                    if not haar.empty():
                        self._haar = haar
                        self.backend = "Haar Cascade"
                        self.mensagem = "Detecção de face: Haar Cascade (Viola-Jones)."
                        return
        self.mensagem = ("Detecção de face indisponível: coloque o modelo "
                         f"{MODELO_YUNET} em assets/modelos/ (veja README) "
                         "ou use opencv-python 4.x (Haar Cascade).")

    def detectar(self, img: np.ndarray) -> list[Rosto]:
        if not self.disponivel:
            return []
        h, w = img.shape[:2]
        escala = min(1.0, self.lado_deteccao / max(h, w))
        pequena = cv2.resize(img, (round(w * escala), round(h * escala)), interpolation=cv2.INTER_AREA) if escala < 1 else img
        rostos: list[Rosto] = []
        if self._yunet is not None:
            ph, pw = pequena.shape[:2]
            if self._tam_yunet != (pw, ph):
                self._yunet.setInputSize((pw, ph))
                self._tam_yunet = (pw, ph)
            _, faces = self._yunet.detect(pequena if pequena.ndim == 3 else cv2.cvtColor(pequena, cv2.COLOR_GRAY2BGR))
            if faces is not None:
                for f in faces:
                    pontos = f[4:14].reshape(5, 2) / escala
                    # YuNet: [olho dir., olho esq., nariz, boca dir., boca esq.] da pessoa.
                    # Reordena para "esquerda/direita na imagem" (vale também com espelhamento).
                    if pontos[0, 0] > pontos[1, 0]:
                        pontos = pontos[[1, 0, 2, 4, 3]]
                    rostos.append(Rosto(*(float(v) / escala for v in f[:4]), pontos.astype(np.float32)))
        elif self._haar is not None:
            cinza = cv2.equalizeHist(para_cinza(pequena))
            minimo = max(24, int(min(cinza.shape) * 0.08))
            for (x, y, fw, fh) in self._haar.detectMultiScale(cinza, 1.1, 6, minSize=(minimo, minimo)):
                rostos.append(Rosto(x / escala, y / escala, fw / escala, fh / escala))
        rostos.sort(key=lambda r: -r.w)
        return rostos[:6]


class SuavizadorRostos:
    """Suaviza as posições entre frames (média móvel exponencial) para reduzir o tremor."""

    def __init__(self, alfa: float = 0.55) -> None:
        self.alfa = alfa
        self.anteriores: list[Rosto] = []
        self.sem_deteccao = 0

    def atualizar(self, novos: list[Rosto]) -> list[Rosto]:
        if not novos:
            self.sem_deteccao += 1
            if self.sem_deteccao > 4:  # mantém por alguns frames para não piscar
                self.anteriores = []
            return self.anteriores
        self.sem_deteccao = 0
        resultado = []
        for r in novos:
            cx, cy = r.centro
            melhor, dist = None, 1e9
            for a in self.anteriores:
                ax, ay = a.centro
                d = math.hypot(cx - ax, cy - ay)
                if d < dist and (a.pontos is None) == (r.pontos is None):
                    melhor, dist = a, d
            if melhor is not None and dist < 0.6 * r.w:
                v = self.alfa * r.vetor() + (1 - self.alfa) * melhor.vetor()
                r = Rosto.de_vetor(v, r.pontos is not None)
            resultado.append(r)
        self.anteriores = resultado
        return resultado


# Filtros de rosto
@dataclass
class ItemRosto:
    sticker: str
    ancora: str
    largura: float          # largura do sticker em relação à largura do rosto
    deslocamento: float = 0  # deslocamento "para cima" em alturas do próprio sticker


PRESETS_ROSTO: dict[str, list[ItemRosto]] = {
    "Thug Life (óculos pixel)": [ItemRosto("oculos", "olhos", 1.10, -0.15)],
    "Coroa": [ItemRosto("coroa", "topo", 0.75, 0.30)],
    "Bigodudo": [ItemRosto("bigode", "buco", 0.80, 0.0), ItemRosto("oculos", "olhos", 1.10, -0.15)],
    "Apaixonado (animado)": [ItemRosto("coracao_pulsante", "olho_e", 0.40, 0.0),
                             ItemRosto("coracao_pulsante", "olho_d", 0.40, 0.0)],
    "Estrela (animado)": [ItemRosto("estrela_saltitante", "topo", 0.50, 0.25)],
}

EFEITOS_ROSTO = ["Pele suave (beauty)", "Anônimo", "Modo retrato (desfocar fundo)"]

OPCOES_ROSTO = ["Nenhum"] + list(PRESETS_ROSTO) + EFEITOS_ROSTO

DESCRICOES_ROSTO = {
    "Nenhum": "Sem filtro de rosto.",
    "Thug Life (óculos pixel)": "Óculos pixel art alinhados aos olhos (giram junto com a cabeça).",
    "Coroa": "Coroa no topo da cabeça.",
    "Bigodudo": "Bigode entre o nariz e a boca + óculos pixel.",
    "Apaixonado (animado)": "Corações pixel pulsando (spritesheet animada) sobre os olhos.",
    "Estrela (animado)": "Estrela pixel pulando (spritesheet animada) sobre a cabeça.",
    "Pele suave (beauty)": "Filtro bilateral aplicado só na região do rosto (suaviza a pele, preserva contornos).",
    "Anônimo": "Pixeliza a região de cada rosto detectado (anonimização).",
    "Modo retrato (desfocar fundo)": "Desfoca tudo, exceto uma elipse suave em torno de cada rosto (efeito retrato).",
}


def _mascara_rosto(shape, rostos: list[Rosto], escala_x=0.75, escala_y=1.05, desfoque=0.25) -> np.ndarray:
    h, w = shape[:2]
    m = np.zeros((h, w), np.uint8)
    for r in rostos:
        cx, cy = r.centro
        eixos = (int(r.w * escala_x), int(r.h * escala_y))
        cv2.ellipse(m, (int(cx), int(cy + 0.05 * r.h)), eixos, math.degrees(r.angulo_rad), 0, 360, 255, -1)
    if rostos:
        s = max(3.0, desfoque * max(r.w for r in rostos))
        m = cv2.GaussianBlur(m, (0, 0), s)
    return m.astype(np.float32) / 255.0


def _misturar(a: np.ndarray, b: np.ndarray, m: np.ndarray) -> np.ndarray:
    m = m[:, :, None]
    return (a.astype(np.float32) * m + b.astype(np.float32) * (1 - m)).astype(np.uint8)


def aplicar_efeito_rosto(img: np.ndarray, rostos: list[Rosto], efeito: str) -> np.ndarray:
    """Filtros aplicados em regiões do rosto (antes dos stickers)."""
    if not rostos or efeito not in EFEITOS_ROSTO:
        return img
    if efeito == "Pele suave (beauty)":
        suave = cv2.bilateralFilter(img, 9, 60, 9)
        suave = cv2.bilateralFilter(suave, 9, 60, 9)
        return _misturar(suave, img, _mascara_rosto(img.shape, rostos, 0.55, 0.75))
    if efeito == "Anônimo":
        saida = img.copy()
        H, W = img.shape[:2]
        for r in rostos:
            x0, y0 = max(0, int(r.x - 0.1 * r.w)), max(0, int(r.y - 0.2 * r.h))
            x1, y1 = min(W, int(r.x + 1.1 * r.w)), min(H, int(r.y + 1.15 * r.h))
            if x1 - x0 < 2 or y1 - y0 < 2:
                continue
            roi = saida[y0:y1, x0:x1]
            bloco = max(4, int(r.w / 10))
            peq = cv2.resize(roi, (max(1, (x1 - x0) // bloco), max(1, (y1 - y0) // bloco)), interpolation=cv2.INTER_LINEAR)
            saida[y0:y1, x0:x1] = cv2.resize(peq, (x1 - x0, y1 - y0), interpolation=cv2.INTER_NEAREST)
        return saida
    if efeito == "Modo retrato (desfocar fundo)":
        h, w = img.shape[:2]
        peq = cv2.resize(img, (max(1, w // 4), max(1, h // 4)), interpolation=cv2.INTER_AREA)
        fundo = cv2.resize(cv2.GaussianBlur(peq, (0, 0), 4), (w, h), interpolation=cv2.INTER_LINEAR)
        # elipse maior: cabeça + pescoço/ombros
        m = _mascara_rosto(img.shape, rostos, 0.95, 1.55, 0.3)
        return _misturar(img, fundo, m)
    return img


def desenhar_stickers_rosto(img: np.ndarray, rostos: list[Rosto], preset: str,
                            stickers: dict[str, Sticker], t: float) -> None:
    """Posiciona automaticamente os stickers do preset em cada rosto (in-place)."""
    itens = PRESETS_ROSTO.get(preset)
    if not itens:
        return
    for r in rostos:
        angulo = -math.degrees(r.angulo_rad)  # transformar() usa ângulo anti-horário
        for item in itens:
            st = stickers.get(item.sticker)
            if st is None:
                continue
            largura = item.largura * r.w
            altura = largura * st.proporcao
            centro = r.ancora(item.ancora) + r.para_cima * (item.deslocamento * altura)
            sprite = transformar(st, st.indice_quadro(t), int(largura), angulo)
            sobrepor(img, sprite, float(centro[0]), float(centro[1]))
