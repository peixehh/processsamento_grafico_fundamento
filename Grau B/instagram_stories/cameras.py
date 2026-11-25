"""
Lista as câmeras conectadas, para a interface mostrar em um dropbox.
"""
from __future__ import annotations

import ctypes
import functools
import os
from ctypes import POINTER, Structure, byref, c_ubyte, c_ulong, c_ushort, c_void_p, c_wchar_p

import cv2


def listar_cameras() -> list[tuple[int, str]]:
    """Devolve [(número para o OpenCV, nome), ...] das câmeras encontradas."""
    if os.name == "nt":
        try:
            return list(enumerate(_nomes_directshow()))
        except Exception:
            pass
    return list(_testar_numeros())


@functools.lru_cache(maxsize=1)
def _testar_numeros(maximo: int = 4) -> tuple[tuple[int, str], ...]:
    encontradas = []
    for i in range(maximo):
        cap = cv2.VideoCapture(i)
        if cap.isOpened():
            encontradas.append((i, f"Câmera {i}"))
        cap.release()
    return tuple(encontradas)


# Windows nomes das câmeras pelo DirectShow
class _GUID(Structure):
    _fields_ = [("d1", c_ulong), ("d2", c_ushort), ("d3", c_ushort), ("d4", c_ubyte * 8)]


class _VARIANT(Structure):
    """Só a parte do VARIANT necessária para ler um texto (BSTR)."""
    _fields_ = [("vt", c_ushort), ("r1", c_ushort), ("r2", c_ushort), ("r3", c_ushort),
                ("valor", c_void_p), ("extra", c_void_p)]


_VT_BSTR = 8
_CLSCTX_INPROC_SERVER = 1


def _guid(texto: str) -> _GUID:
    g = _GUID()
    ctypes.oledll.ole32.CLSIDFromString(c_wchar_p(texto), byref(g))
    return g


def _chamar(obj: c_void_p, indice: int, *args, tipos=()) -> int:
    vtable = ctypes.cast(obj, POINTER(POINTER(c_void_p))).contents
    prototipo = ctypes.WINFUNCTYPE(ctypes.HRESULT, c_void_p, *tipos)
    return prototipo(vtable[indice])(obj, *args)


def _liberar(obj: c_void_p) -> None:
    """IUnknown::Release (método 2): devolve o objeto COM."""
    vtable = ctypes.cast(obj, POINTER(POINTER(c_void_p))).contents
    ctypes.WINFUNCTYPE(c_ulong, c_void_p)(vtable[2])(obj)


def _nomes_directshow() -> list[str]:
    ole32 = ctypes.oledll.ole32
    try:
        ole32.CoInitialize(None)
        inicializou = True
    except OSError:
        inicializou = False
    nomes: list[str] = []
    try:
        # Enumerador de dispositivos do sistema -> só a categoria "entrada de vídeo"
        criador = c_void_p()
        ole32.CoCreateInstance(byref(_guid("{62BE5D10-60EB-11D0-BD3B-00A0C911CE86}")), None,
                               _CLSCTX_INPROC_SERVER,
                               byref(_guid("{29840822-5B84-11D0-BD3B-00A0C911CE86}")), byref(criador))
        lista = c_void_p()
        try:
            hr = _chamar(criador, 3, byref(_guid("{860BB310-5D01-11D0-BD3B-00A0C911CE86}")),
                         byref(lista), 0, tipos=(POINTER(_GUID), POINTER(c_void_p), c_ulong))
        finally:
            _liberar(criador)
        if hr != 0 or not lista:
            return nomes
        try:
            while True:
                item = c_void_p()
                pegou = c_ulong()
                hr = _chamar(lista, 3, 1, byref(item), byref(pegou),
                             tipos=(c_ulong, POINTER(c_void_p), POINTER(c_ulong)))
                if hr != 0 or pegou.value == 0:
                    break
                try:
                    nomes.append(_nome_amigavel(item) or f"Câmera {len(nomes)}")
                finally:
                    _liberar(item)
        finally:
            _liberar(lista)
    finally:
        if inicializou:
            ole32.CoUninitialize()
    return nomes


def _nome_amigavel(item: c_void_p) -> str:
    """Lê a propriedade "FriendlyName" (o nome que aparece no Windows) de uma câmera."""
    bolsa = c_void_p()
    _chamar(item, 9, None, None, byref(_guid("{55272A00-42CB-11CE-8135-00AA004BB851}")), byref(bolsa),
            tipos=(c_void_p, c_void_p, POINTER(_GUID), POINTER(c_void_p)))
    try:
        valor = _VARIANT()
        _chamar(bolsa, 3, "FriendlyName", byref(valor), None,
                tipos=(c_wchar_p, POINTER(_VARIANT), c_void_p))
        try:
            return ctypes.wstring_at(valor.valor) if valor.vt == _VT_BSTR and valor.valor else ""
        finally:
            ctypes.oledll.oleaut32.VariantClear(byref(valor))
    finally:
        _liberar(bolsa)
