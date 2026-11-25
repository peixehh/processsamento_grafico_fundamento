from __future__ import annotations

import os
import sys

AZUL = (37, 128, 195)          # azul #2580c3
AZUL_CLARO = (92, 179, 255)    # #5cb3ff
AZUL_NOITE = (20, 27, 45)      # #141b2d
CINZA_AZULADO = (139, 155, 189)  # #8b9bbd
BRANCO = (230, 236, 247)       # #e6ecf7

LARGURA = 72


def _ativar_cores() -> bool:
    if os.environ.get("NO_COLOR"):
        return False
    if not os.environ.get("FORCE_COLOR"):
        if not hasattr(sys.stdout, "isatty") or not sys.stdout.isatty():
            return False
    if os.name == "nt":
        try:
            import ctypes
            kernel32 = ctypes.windll.kernel32
            handle = kernel32.GetStdHandle(-11)  # STD_OUTPUT_HANDLE
            modo = ctypes.c_uint32()
            if kernel32.GetConsoleMode(handle, ctypes.byref(modo)):
                kernel32.SetConsoleMode(handle, modo.value | 0x0004)  # ENABLE_VIRTUAL_TERMINAL_PROCESSING
        except Exception:
            pass
    return True


CORES_ATIVAS = _ativar_cores()


def cor(texto: str, frente=BRANCO, fundo=None, negrito: bool = False) -> str:
    if not CORES_ATIVAS:
        return texto
    codigo = f"\033[38;2;{frente[0]};{frente[1]};{frente[2]}m"
    if fundo is not None:
        codigo += f"\033[48;2;{fundo[0]};{fundo[1]};{fundo[2]}m"
    if negrito:
        codigo = "\033[1m" + codigo
    return f"{codigo}{texto}\033[0m"


def banner(titulo: str, linhas: list[str]) -> None:
    """Cabeçalho: faixa azul com o título e linhas de informação."""
    print(cor(f"  {titulo}".ljust(LARGURA), frente=(255, 255, 255), fundo=AZUL, negrito=True))
    for linha in linhas:
        print(cor("  │ ", AZUL) + cor(linha, BRANCO))
    print(cor("─" * LARGURA, AZUL))


def log(etiqueta: str, mensagem: str = "") -> None:
    """Mensagem curta: [etiqueta] em azul claro + texto."""
    print(cor(f"[{etiqueta}]", AZUL_CLARO, negrito=True), cor(mensagem, BRANCO))


def bloco(etiqueta: str, texto: str) -> None:
    """Bloco de texto (ex.: descrição de filtro) com separador azul."""
    print()
    print(cor("─" * LARGURA, AZUL))
    linhas = texto.splitlines() or [""]
    print(cor(f"[{etiqueta}]", AZUL_CLARO, negrito=True), cor(linhas[0], (255, 255, 255), negrito=True))
    for linha in linhas[1:]:
        destaque = linha.strip().startswith("•") or linha.startswith("Parâmetros")
        print(cor(linha, AZUL_CLARO if destaque else CINZA_AZULADO))
