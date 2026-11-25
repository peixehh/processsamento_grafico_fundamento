"""
Captura de vídeo em uma thread separada (webcam ou arquivo de vídeo).

A thread lê os frames continuamente e guarda apenas o mais recente; a
interface pega esse frame no seu próprio ritmo. Assim a GUI não trava
esperando a câmera.
"""
from __future__ import annotations

import os
import threading
import time

import cv2
import numpy as np


class FonteVideo:
    def __init__(self, fonte: int | str = 0, largura: int = 1280, altura: int = 720) -> None:
        self.fonte = fonte
        self.largura = largura
        self.altura = altura
        self.estado = "parado"      # parado | abrindo | ok | erro
        self.erro = ""
        self._cap: cv2.VideoCapture | None = None
        self._thread: threading.Thread | None = None
        self._lock = threading.Lock()
        self._frame: np.ndarray | None = None
        self._contador = 0
        self._rodando = False

    @property
    def eh_arquivo(self) -> bool:
        return isinstance(self.fonte, str)

    def iniciar(self) -> None:
        if self._rodando:
            return
        self._rodando = True
        self.estado = "abrindo"
        self._thread = threading.Thread(target=self._laco, daemon=True)
        self._thread.start()

    def _abrir(self) -> cv2.VideoCapture | None:
        if self.eh_arquivo:
            cap = cv2.VideoCapture(self.fonte)
            return cap if cap.isOpened() else None
        backends = [cv2.CAP_DSHOW, cv2.CAP_MSMF, cv2.CAP_ANY] if os.name == "nt" else [cv2.CAP_ANY]
        for backend in backends:
            cap = cv2.VideoCapture(self.fonte, backend)
            if cap.isOpened():
                cap.set(cv2.CAP_PROP_FRAME_WIDTH, self.largura)
                cap.set(cv2.CAP_PROP_FRAME_HEIGHT, self.altura)
                ok, _ = cap.read()
                if ok:
                    return cap
            cap.release()
        return None

    def _laco(self) -> None:
        cap = self._abrir()
        if cap is None:
            self.estado = "erro"
            self.erro = (f"Não foi possível abrir o vídeo '{self.fonte}'." if self.eh_arquivo
                         else f"Não foi possível abrir a webcam {self.fonte}.")
            self._rodando = False
            return
        self._cap = cap
        self.estado = "ok"
        fps = cap.get(cv2.CAP_PROP_FPS) if self.eh_arquivo else 0
        intervalo = 1.0 / fps if fps and fps > 1 else 1.0 / 30
        falhas = 0
        while self._rodando:
            inicio = time.perf_counter()
            ok, frame = cap.read()
            if not ok:
                if self.eh_arquivo:  # reinicia o vídeo (loop)
                    cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
                    continue
                falhas += 1
                if falhas > 60:
                    self.estado = "erro"
                    self.erro = "A webcam parou de enviar imagens."
                    break
                time.sleep(0.02)
                continue
            falhas = 0
            with self._lock:
                self._frame = frame
                self._contador += 1
            if self.eh_arquivo:  # respeita o FPS do arquivo
                espera = intervalo - (time.perf_counter() - inicio)
                if espera > 0:
                    time.sleep(espera)
        cap.release()
        self._cap = None
        self._rodando = False

    def ler(self) -> tuple[int, np.ndarray | None]:
        """Devolve (número do frame, frame mais recente)."""
        with self._lock:
            return self._contador, self._frame

    def parar(self) -> None:
        self._rodando = False
        if self._thread is not None and self._thread is not threading.current_thread():
            self._thread.join(timeout=2.0)
        self._thread = None
        if self.estado != "erro":
            self.estado = "parado"
