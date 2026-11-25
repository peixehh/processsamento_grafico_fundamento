"""Pipeline de etapas: permite combinar filtros e operações em sequência.

Cada etapa é aplicada sobre o resultado da anterior:
    resultado = etapa_n( ... etapa_2( etapa_1(original) ) )

No modo foto o resultado acumulado fica em cache (só a nova etapa é
calculada); no modo vídeo a sequência inteira é aplicada a cada frame.
"""
from __future__ import annotations

from dataclasses import dataclass, field

import numpy as np

from .filtros import Filtro
from .io_utils import ajustar_cobrindo, para_bgr
from .operacoes import Operacao


@dataclass(eq=False)
class Etapa:
    item: Filtro | Operacao
    params: dict
    canal: str = "RGB"
    img_b: np.ndarray | None = None    # segunda imagem (somente operações)
    nome_b: str = ""
    _cache_b: dict = field(default_factory=dict, repr=False)

    @property
    def eh_operacao(self) -> bool:
        return isinstance(self.item, Operacao)

    def _b_para(self, h: int, w: int) -> np.ndarray:
        chave = (h, w)
        if chave not in self._cache_b:
            if len(self._cache_b) > 4:
                self._cache_b.clear()
            self._cache_b[chave] = ajustar_cobrindo(para_bgr(self.img_b), h, w)
        return self._cache_b[chave]

    def aplicar(self, img: np.ndarray) -> np.ndarray:
        if self.eh_operacao:
            if self.img_b is None:
                return img
            b = self._b_para(*img.shape[:2])
            return self.item.aplicar(img, b, self.params, self.canal)
        return self.item.aplicar(img, self.params, self.canal)

    def detalhes(self) -> str:
        """Parâmetros, imagem B e canal, ex.: '(k=7, sigma=1.5) [canal R]'."""
        p = self.item.normalizar(self.params)
        partes = [f"{par.nome}={p[par.nome]:g}" if isinstance(p[par.nome], (int, float)) else str(p[par.nome])
                  for par in self.item.params]
        extras = []
        if partes:
            extras.append("(" + ", ".join(partes) + ")")
        if self.eh_operacao and self.nome_b:
            extras.append(f"com B={self.nome_b}")
        if self.canal != "RGB":
            extras.append(f"[canal {self.canal}]")
        return " ".join(extras)

    def resumo(self) -> str:
        detalhes = self.detalhes()
        return self.item.nome + (" " + detalhes if detalhes else "")


class Pipeline:
    def __init__(self) -> None:
        self.etapas: list[Etapa] = []

    def __len__(self) -> int:
        return len(self.etapas)

    def adicionar(self, etapa: Etapa) -> None:
        self.etapas.append(etapa)

    def remover_ultima(self) -> Etapa | None:
        return self.etapas.pop() if self.etapas else None

    def limpar(self) -> None:
        self.etapas.clear()

    def aplicar(self, img: np.ndarray) -> np.ndarray:
        resultado = para_bgr(img)
        for etapa in self.etapas:
            resultado = etapa.aplicar(resultado)
        return resultado
