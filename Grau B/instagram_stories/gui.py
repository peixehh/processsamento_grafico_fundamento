"""
Interface gráfica do InstagramStories (Tkinter + OpenCV).
"""
from __future__ import annotations

import argparse
import os
import time
import tkinter as tk
import traceback
from tkinter import filedialog, messagebox, ttk

import cv2
import numpy as np
from PIL import Image, ImageTk

from . import NOME_APP, console
from .cameras import listar_cameras
from .filtros import CATEGORIAS, DESCRICAO_CANAIS, FILTROS, FILTROS_POR_ID, Filtro, Param, visualizar_canal
from .io_utils import ler_imagem, limitar_tamanho, salvar_imagem
from .operacoes import OPERACOES, OPERACOES_POR_ID, TEXTURAS, Operacao, gerar_textura
from .pipeline import Etapa, Pipeline
from .rosto import (DESCRICOES_ROSTO, OPCOES_ROSTO, PRESETS_ROSTO, DetectorRosto, SuavizadorRostos,
                    aplicar_efeito_rosto, desenhar_stickers_rosto)
from .stickers import StickerColocado, carregar_stickers
from .webcam import FonteVideo

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PASTA_STICKERS = os.path.join(RAIZ, "assets", "stickers")
PASTA_MODELOS = os.path.join(RAIZ, "assets", "modelos")
PASTA_IMAGENS = os.path.join(RAIZ, "assets", "imagens")
PASTA_SAIDAS = os.path.join(RAIZ, "saidas")

LADO_MAX_FOTO = 1920   # fotos maiores são reduzidas ao abrir
LADO_MAX_VIDEO = 960   # frames da webcam são processados nesta resolução

CORES = {
    # Tema azul escuro
    "fundo": "#0d1220",
    "painel": "#141b2d",
    "painel2": "#1c2540",
    "borda": "#2a3654",
    "texto": "#e6ecf7",
    "texto2": "#8b9bbd",
    "destaque": "#2580c3",       # azul (botões, sliders, marcações)
    "destaque_hover": "#3a96dc",
    "destaque_press": "#1b6aa6",
    "destaque2": "#1f5f99",      # seleção em listas, selo do modo
    "valor": "#5cb3ff",          # números dos parâmetros
    "hover": "#24304d",
    "canvas": "#080c16",
}

ABA_FILTROS, ABA_OPERACOES, ABA_STICKERS = 0, 1, 2

TEXTO_INICIAL_FILTRO = (
    "Selecione um filtro na lista acima para ver a descrição e os parâmetros.\n\n"
    "• A prévia aparece imediatamente na imagem.\n"
    "• Aplicar filtro fixa o efeito; o próximo filtro é aplicado sobre o resultado "
    "(combinação de filtros).\n"
    "• O Canal (painel esquerdo) define se o filtro atua na imagem inteira (RGB/Cinza) "
    "ou só em R, G ou B."
)
TEXTO_INICIAL_OP = (
    "Operações aritméticas entre a imagem atual (A) e uma segunda imagem (B).\n\n"
    "1) Escolha a imagem B acima (arquivo, textura gerada ou a própria imagem atual).\n"
    "2) Selecione a operação para ver a descrição e a prévia.\n"
    "3) Clique em 'Aplicar operação'."
)


def _fmt(v) -> str:
    if isinstance(v, str):
        return v
    if isinstance(v, int) or float(v).is_integer():
        return str(int(v))
    return f"{v:.2f}".rstrip("0").rstrip(".")


def configurar_estilo(root: tk.Tk) -> None:
    c = CORES
    st = ttk.Style(root)
    st.theme_use("clam")
    st.configure(".", background=c["painel"], foreground=c["texto"], fieldbackground=c["painel2"],
                 bordercolor=c["borda"], lightcolor=c["painel"], darkcolor=c["painel"],
                 troughcolor=c["painel2"], focuscolor=c["destaque"], font=("Segoe UI", 10),
                 selectbackground=c["destaque2"], selectforeground="white")
    st.configure("TFrame", background=c["painel"])
    st.configure("Fundo.TFrame", background=c["fundo"])
    st.configure("TLabel", background=c["painel"], foreground=c["texto"])
    st.configure("Fundo.TLabel", background=c["fundo"], foreground=c["texto"])
    st.configure("Sec.TLabel", foreground=c["texto2"])
    st.configure("Valor.TLabel", foreground=c["valor"], font=("Segoe UI Semibold", 10))
    st.configure("Logo.TLabel", background=c["fundo"], foreground="white", font=("Segoe UI Black", 15))
    st.configure("Badge.TLabel", background=c["destaque2"], foreground="white", padding=(12, 4),
                 font=("Segoe UI Semibold", 9))
    st.configure("Status.TLabel", background=c["fundo"], foreground=c["texto2"], padding=(10, 3))
    st.configure("TLabelframe", background=c["painel"], bordercolor=c["borda"], relief="solid")
    st.configure("TLabelframe.Label", background=c["painel"], foreground=c["texto2"],
                 font=("Segoe UI Semibold", 9))
    st.configure("TButton", background=c["painel2"], foreground=c["texto"], bordercolor=c["borda"],
                 padding=(10, 5), focusthickness=0)
    st.map("TButton", background=[("pressed", c["borda"]), ("active", c["hover"])],
           foreground=[("disabled", "#56647f")])
    st.configure("Topo.TButton", background=c["painel"], padding=(12, 6))
    st.map("Topo.TButton", background=[("pressed", c["borda"]), ("active", c["hover"])])
    st.configure("Destaque.TButton", background=c["destaque"], foreground="white", bordercolor=c["destaque"],
                 font=("Segoe UI Semibold", 10))
    st.map("Destaque.TButton", background=[("pressed", c["destaque_press"]), ("active", c["destaque_hover"])])
    st.configure("Sticker.TButton", padding=3, background=c["painel2"])
    st.configure("StickerSel.TButton", padding=3, background=c["destaque"], bordercolor="white")
    st.map("StickerSel.TButton", background=[("active", c["destaque_hover"])])
    for w in ("TRadiobutton", "TCheckbutton"):
        st.configure(w, background=c["painel"], foreground=c["texto"], indicatorbackground=c["painel2"],
                     indicatorforeground="white", focusthickness=0)
        st.map(w, background=[("active", c["painel"])],
               indicatorbackground=[("selected", c["destaque"]), ("active", c["borda"])])
    st.configure("TNotebook", background=c["painel"], bordercolor=c["borda"], tabmargins=(0, 0, 0, 0))
    st.configure("TNotebook.Tab", background=c["painel2"], foreground=c["texto2"], padding=(12, 6),
                 bordercolor=c["borda"])
    st.map("TNotebook.Tab", background=[("selected", c["painel"])], foreground=[("selected", "white")],
           expand=[("selected", (0, 0, 0, 0))])
    st.configure("Treeview", background=c["painel2"], fieldbackground=c["painel2"], foreground=c["texto"],
                 rowheight=24, bordercolor=c["borda"], borderwidth=0)
    st.map("Treeview", background=[("selected", c["destaque2"])], foreground=[("selected", "white")])
    st.configure("TCombobox", fieldbackground=c["painel2"], background=c["painel2"], foreground=c["texto"],
                 arrowcolor=c["texto"], bordercolor=c["borda"])
    st.map("TCombobox", fieldbackground=[("readonly", c["painel2"])], foreground=[("readonly", c["texto"])],
           selectbackground=[("readonly", c["painel2"])], selectforeground=[("readonly", c["texto"])])
    root.option_add("*TCombobox*Listbox.background", c["painel2"])
    root.option_add("*TCombobox*Listbox.foreground", c["texto"])
    root.option_add("*TCombobox*Listbox.selectBackground", c["destaque2"])
    st.configure("Horizontal.TScale", background=c["destaque"], troughcolor=c["painel2"],
                 bordercolor=c["borda"], lightcolor=c["destaque"], darkcolor=c["destaque"])
    st.configure("Vertical.TScrollbar", background="#34456b", troughcolor=c["painel2"],
                 bordercolor=c["painel2"], lightcolor="#34456b", darkcolor="#34456b",
                 arrowcolor=c["texto2"], gripcount=0)
    st.map("Vertical.TScrollbar", background=[("active", c["destaque"])])
    st.configure("TSpinbox", fieldbackground=c["painel2"], foreground=c["texto"], arrowcolor=c["texto"],
                 background=c["painel2"])


class App(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title(f"{NOME_APP} - Efeitos em Imagens e Vídeo")
        self.configure(bg=CORES["fundo"])
        self.geometry("1420x880")
        self.minsize(1180, 720)
        configurar_estilo(self)

        # estado
        self.modo: str | None = None              # None | "foto" | "video"
        self.nome_atual = ""
        self.original: np.ndarray | None = None   # foto original / último frame bruto
        self.resultado_etapas: np.ndarray | None = None  # cache do pipeline (modo foto)
        self.pipeline = Pipeline()
        self.colocados: list[StickerColocado] = []
        self.colocado_ativo: StickerColocado | None = None
        self.historico: list[tuple] = []
        self.fonte_video: FonteVideo | None = None
        self.ultimo_frame_id = -1
        self.ultimo_exibido: np.ndarray | None = None  # imagem final (O que é salvado)
        self._cache_base: np.ndarray | None = None
        self._rostos_atuais = []
        self.rostos_foto = None
        self.suavizador = SuavizadorRostos()
        self._render_agendado = None
        self._exib = (0, 0, 1.0)
        self._item_img = None
        self._foto_tk = None
        self._arrasto = None
        self._fps = 0.0
        self._t_frame = time.perf_counter()
        self._t_anim = 0.0
        self._vars_params: dict[str, list[tk.Variable]] = {}

        self.filtro_sel: Filtro | None = None
        self.params_filtro: dict = {}
        self.op_sel: Operacao | None = None
        self.params_op: dict = {}
        self.img_b: np.ndarray | None = None
        self.nome_b = ""
        self._cache_b_prev: dict = {}

        self.var_canal = tk.StringVar(value="RGB")
        self.var_canal_cinza = tk.BooleanVar(value=False)
        self.var_preview = tk.BooleanVar(value=True)
        self.var_rosto = tk.StringVar(value="Nenhum")
        self.var_espelhar = tk.BooleanVar(value=True)
        self.var_camera = tk.StringVar(value="")
        self.cameras: list[tuple[int, str]] = []   # [(número para o OpenCV, nome)]
        self.var_escala = tk.DoubleVar(value=25)
        self.var_angulo = tk.DoubleVar(value=0)
        self.var_textura = tk.StringVar(value=list(TEXTURAS)[0])

        self.stickers = carregar_stickers(PASTA_STICKERS)
        self.stickers_por_id = {s.id: s for s in self.stickers}
        self.sticker_sel = None
        self.detector = DetectorRosto(PASTA_MODELOS)

        self._montar_interface()
        self._atalhos()
        self.protocol("WM_DELETE_WINDOW", self._fechar)
        self._imprimir_boas_vindas()
        self.after(50, self._laco)

    # Construção da interface

    def _montar_interface(self) -> None:
        # barra superior
        topo = ttk.Frame(self, style="Fundo.TFrame", padding=(10, 8))
        topo.pack(side="top", fill="x")
        ttk.Label(topo, text=NOME_APP, style="Logo.TLabel").pack(side="left", padx=(4, 18))
        ttk.Button(topo, text="Abrir foto", style="Topo.TButton", command=self.abrir_foto).pack(side="left", padx=2)
        self.btn_webcam = ttk.Button(topo, text="Iniciar webcam", style="Topo.TButton", command=self.alternar_webcam)
        self.btn_webcam.pack(side="left", padx=2)
        ttk.Button(topo, text="Salvar", style="Topo.TButton", command=self.salvar).pack(side="left", padx=2)
        ttk.Button(topo, text="Desfazer", style="Topo.TButton", command=self.desfazer).pack(side="left", padx=2)
        ttk.Button(topo, text="Resetar", style="Topo.TButton", command=self.resetar).pack(side="left", padx=2)
        self.lbl_modo = ttk.Label(topo, text="SEM IMAGEM", style="Badge.TLabel")
        self.lbl_modo.pack(side="right", padx=4)

        # barra de status
        rodape = ttk.Frame(self, style="Fundo.TFrame")
        rodape.pack(side="bottom", fill="x")
        self.lbl_status = ttk.Label(rodape, text="Abra uma foto (Ctrl+O) ou inicie a webcam (Ctrl+W).",
                                    style="Status.TLabel")
        self.lbl_status.pack(side="left")
        self.lbl_info = ttk.Label(rodape, text="", style="Status.TLabel")
        self.lbl_info.pack(side="right")

        corpo = ttk.Frame(self, style="Fundo.TFrame")
        corpo.pack(side="top", fill="both", expand=True, padx=8, pady=(0, 4))

        esquerda = ttk.Frame(corpo, padding=8, width=280)
        esquerda.pack(side="left", fill="y")
        esquerda.pack_propagate(False)
        direita = ttk.Frame(corpo, padding=(6, 6, 6, 6), width=400)
        direita.pack(side="right", fill="y")
        direita.pack_propagate(False)

        centro = ttk.Frame(corpo, style="Fundo.TFrame", padding=(8, 0))
        centro.pack(side="left", fill="both", expand=True)
        self.canvas = tk.Canvas(centro, bg=CORES["canvas"], highlightthickness=0, cursor="crosshair")
        self.canvas.pack(fill="both", expand=True)
        self._txt_vazio = self.canvas.create_text(
            10, 10, fill=CORES["texto2"], font=("Segoe UI", 13), justify="center",
            text="Abra uma foto  (Ctrl+O)\nou inicie a webcam  (Ctrl+W)")
        self.canvas.bind("<Configure>", self._ao_redimensionar_canvas)
        self.canvas.bind("<ButtonPress-1>", self._ao_clicar)
        self.canvas.bind("<B1-Motion>", self._ao_arrastar)
        self.canvas.bind("<ButtonRelease-1>", self._ao_soltar)
        self.canvas.bind("<Button-3>", self._ao_botao_direito)
        self.canvas.bind("<MouseWheel>", self._ao_rolar)
        self.canvas.bind("<Motion>", self._ao_mover_mouse)

        self._montar_painel_esquerdo(esquerda)
        self._montar_painel_direito(direita)

    def _montar_painel_esquerdo(self, pai: ttk.Frame) -> None:
        # Canal
        fc = ttk.LabelFrame(pai, text=" CANAL (aplicação e visualização) ", padding=8)
        fc.pack(fill="x")
        l1 = ttk.Frame(fc)
        l1.pack(fill="x")
        l2 = ttk.Frame(fc)
        l2.pack(fill="x", pady=(2, 0))
        rotulos = {"RGB": "RGB", "Cinza": "Cinza", "R": "R (verm.)", "G": "G (verde)", "B": "B (azul)"}
        for i, (canal, rotulo) in enumerate(rotulos.items()):
            ttk.Radiobutton(l1 if i < 2 else l2, text=rotulo, value=canal, variable=self.var_canal,
                            command=self._ao_mudar_canal).pack(side="left", padx=(0, 6))
        ttk.Checkbutton(fc, text="Canal isolado em tons de cinza", variable=self.var_canal_cinza,
                        command=self._ao_mudar_canal).pack(anchor="w", pady=(6, 0))

        # Etapas
        fe = ttk.LabelFrame(pai, text=" ETAPAS APLICADAS (em sequência) ", padding=8)
        fe.pack(fill="both", expand=True, pady=(8, 0))
        self.lista_etapas = tk.Listbox(fe, bg=CORES["painel2"], fg=CORES["texto"], relief="flat",
                                       highlightthickness=0, selectbackground=CORES["destaque2"],
                                       activestyle="none", font=("Segoe UI", 9))
        self.lista_etapas.pack(fill="both", expand=True)
        ttk.Label(fe, text="Cada etapa é aplicada sobre o resultado da anterior.", style="Sec.TLabel",
                  wraplength=220, font=("Segoe UI", 8)).pack(anchor="w", pady=(4, 4))
        botoes = ttk.Frame(fe)
        botoes.pack(fill="x")
        ttk.Button(botoes, text="Desfazer", command=self.desfazer).pack(side="left", expand=True, fill="x")
        ttk.Button(botoes, text="Resetar tudo", command=self.resetar).pack(side="left", expand=True, fill="x",
                                                                            padx=(4, 0))

        # Vídeo
        fv = ttk.LabelFrame(pai, text=" VÍDEO ", padding=8)
        fv.pack(fill="x", pady=(8, 0))
        linha = ttk.Frame(fv)
        linha.pack(fill="x")
        ttk.Label(linha, text="Câmera").pack(side="left")
        # a lista é refeita sempre que abre, então aparece uma câmera conectada depois
        self.cmb_camera = ttk.Combobox(linha, textvariable=self.var_camera, state="readonly", width=20,
                                       postcommand=self._atualizar_cameras)
        self.cmb_camera.pack(side="left", fill="x", expand=True, padx=(6, 0))
        self.cmb_camera.bind("<<ComboboxSelected>>", self._ao_escolher_camera)
        self._atualizar_cameras()
        ttk.Checkbutton(fv, text="Espelhar", variable=self.var_espelhar).pack(anchor="w", pady=(6, 0))
        ttk.Button(fv, text="Abrir arquivo de vídeo...", command=self.abrir_video).pack(fill="x", pady=(6, 0))
        ttk.Label(fv, text="Ao parar o vídeo, a tela volta ao início.", style="Sec.TLabel",
                  wraplength=220, font=("Segoe UI", 8)).pack(anchor="w", pady=(4, 0))

        # Atalhos
        fa = ttk.LabelFrame(pai, text=" ATALHOS ", padding=8)
        fa.pack(fill="x", pady=(8, 0))
        ttk.Label(fa, style="Sec.TLabel", font=("Segoe UI", 8), justify="left", text=(
            "Ctrl+O abrir foto   Ctrl+W webcam\n"
            "Ctrl+S salvar   Ctrl+Shift+S salvar rápido\n"
            "Ctrl+Z desfazer   Ctrl+R resetar\n"
            "Enter aplicar   Esc cancelar prévia\n"
            "Mouse: clique coloca sticker, arraste\n"
            "move, botão direito remove, roda muda\n"
            "o tamanho e Shift+roda gira.")).pack(anchor="w")

    def _montar_painel_direito(self, pai: ttk.Frame) -> None:
        self.notebook = ttk.Notebook(pai)
        self.notebook.pack(fill="both", expand=True)
        aba_f = ttk.Frame(self.notebook, padding=8)
        aba_o = ttk.Frame(self.notebook, padding=8)
        aba_s = ttk.Frame(self.notebook, padding=8)
        self.notebook.add(aba_f, text="Filtros")
        self.notebook.add(aba_o, text="Operações (2 imagens)")
        self.notebook.add(aba_s, text="Stickers & Rosto")
        self.notebook.bind("<<NotebookTabChanged>>", lambda e: self._agendar_render())
        self._montar_aba_filtros(aba_f)
        self._montar_aba_operacoes(aba_o)
        self._montar_aba_stickers(aba_s)

    def _caixa_texto(self, pai, altura: int) -> tk.Text:
        quadro = ttk.Frame(pai)
        quadro.pack(fill="x", pady=(6, 0))
        txt = tk.Text(quadro, height=altura, wrap="word", bg=CORES["painel2"], fg=CORES["texto"],
                      relief="flat", padx=8, pady=6, font=("Segoe UI", 9), highlightthickness=0,
                      insertbackground=CORES["texto"])
        barra = ttk.Scrollbar(quadro, orient="vertical", command=txt.yview)
        txt.configure(yscrollcommand=barra.set, state="disabled")
        barra.pack(side="right", fill="y")
        txt.pack(side="left", fill="x", expand=True)
        return txt

    @staticmethod
    def _escrever(txt: tk.Text, conteudo: str) -> None:
        txt.configure(state="normal")
        txt.delete("1.0", "end")
        txt.insert("1.0", conteudo)
        txt.configure(state="disabled")

    def _montar_aba_filtros(self, aba: ttk.Frame) -> None:
        quadro = ttk.Frame(aba)
        quadro.pack(fill="x")
        self.tree_filtros = ttk.Treeview(quadro, show="tree", height=11, selectmode="browse")
        barra = ttk.Scrollbar(quadro, orient="vertical", command=self.tree_filtros.yview)
        self.tree_filtros.configure(yscrollcommand=barra.set)
        barra.pack(side="right", fill="y")
        self.tree_filtros.pack(side="left", fill="x", expand=True)
        for cat in CATEGORIAS:
            self.tree_filtros.insert("", "end", iid="cat:" + cat, text=cat, open=True)
        for f in FILTROS:
            self.tree_filtros.insert("cat:" + f.categoria, "end", iid=f.id, text="   " + f.nome)
        self.tree_filtros.bind("<<TreeviewSelect>>", self._ao_selecionar_filtro)

        self.txt_desc_filtro = self._caixa_texto(aba, 9)
        self._escrever(self.txt_desc_filtro, TEXTO_INICIAL_FILTRO)
        self.frame_params_filtro = ttk.LabelFrame(aba, text=" PARÂMETROS ", padding=8)
        self.frame_params_filtro.pack(fill="x", pady=(6, 0))
        ttk.Label(self.frame_params_filtro, text="Selecione um filtro.", style="Sec.TLabel").pack(anchor="w")

        rodape = ttk.Frame(aba)
        rodape.pack(fill="x", side="bottom", pady=(6, 0))
        ttk.Checkbutton(rodape, text="Pré-visualizar", variable=self.var_preview,
                        command=self._agendar_render).pack(side="left")
        ttk.Button(rodape, text="Aplicar filtro", style="Destaque.TButton",
                   command=self.aplicar_filtro).pack(side="right")

    def _montar_aba_operacoes(self, aba: ttk.Frame) -> None:
        fb = ttk.LabelFrame(aba, text=" IMAGEM B (segunda imagem) ", padding=8)
        fb.pack(fill="x")
        linha = ttk.Frame(fb)
        linha.pack(fill="x")
        self.lbl_thumb_b = tk.Label(linha, bg=CORES["painel2"], width=16, height=5, text="(nenhuma)",
                                    fg=CORES["texto2"])
        self.lbl_thumb_b.pack(side="left")
        col = ttk.Frame(linha)
        col.pack(side="left", fill="both", expand=True, padx=(8, 0))
        ttk.Button(col, text="Carregar arquivo...", command=self.carregar_imagem_b).pack(fill="x")
        ttk.Button(col, text="Usar imagem atual", command=self.usar_atual_como_b).pack(fill="x", pady=(4, 0))
        linha2 = ttk.Frame(fb)
        linha2.pack(fill="x", pady=(6, 0))
        ttk.Combobox(linha2, values=list(TEXTURAS), textvariable=self.var_textura,
                     state="readonly").pack(side="left", fill="x", expand=True)
        ttk.Button(linha2, text="Usar textura", command=self.usar_textura_como_b).pack(side="left", padx=(4, 0))
        self.lbl_nome_b = ttk.Label(fb, text="B: (nenhuma)", style="Sec.TLabel")
        self.lbl_nome_b.pack(anchor="w", pady=(4, 0))

        self.tree_ops = ttk.Treeview(aba, show="tree", height=7, selectmode="browse")
        self.tree_ops.pack(fill="x", pady=(6, 0))
        for o in OPERACOES:
            self.tree_ops.insert("", "end", iid=o.id, text="  " + o.nome)
        self.tree_ops.bind("<<TreeviewSelect>>", self._ao_selecionar_operacao)

        self.txt_desc_op = self._caixa_texto(aba, 6)
        self._escrever(self.txt_desc_op, TEXTO_INICIAL_OP)
        self.frame_params_op = ttk.LabelFrame(aba, text=" PARÂMETROS ", padding=8)
        self.frame_params_op.pack(fill="x", pady=(6, 0))
        ttk.Label(self.frame_params_op, text="Selecione uma operação.", style="Sec.TLabel").pack(anchor="w")
        rodape = ttk.Frame(aba)
        rodape.pack(fill="x", side="bottom", pady=(6, 0))
        ttk.Checkbutton(rodape, text="Pré-visualizar", variable=self.var_preview,
                        command=self._agendar_render).pack(side="left")
        ttk.Button(rodape, text="Aplicar operação", style="Destaque.TButton",
                   command=self.aplicar_operacao).pack(side="right")

    def _montar_aba_stickers(self, aba: ttk.Frame) -> None:
        ttk.Label(aba, style="Sec.TLabel", wraplength=360, font=("Segoe UI", 9), text=(
            "Escolha um sticker e clique na imagem para colocá-lo. Arraste para mover, "
            "botão direito remove, roda do mouse muda o tamanho (Shift+roda gira).")).pack(anchor="w")
        grade = ttk.Frame(aba)
        grade.pack(fill="x", pady=(6, 0))
        self._thumbs_stickers = []
        self._botoes_stickers = {}
        colunas = 4
        for i, st in enumerate(self.stickers):
            q = cv2.cvtColor(st.quadros[0], cv2.COLOR_BGRA2RGBA)
            im = Image.fromarray(q)
            im.thumbnail((58, 58), Image.LANCZOS)
            fundo = Image.new("RGBA", (62, 62), (0, 0, 0, 0))
            fundo.alpha_composite(im, ((62 - im.width) // 2, (62 - im.height) // 2))
            foto = ImageTk.PhotoImage(fundo)
            self._thumbs_stickers.append(foto)
            texto = "anim." if st.animado else ""
            b = ttk.Button(grade, image=foto, text=texto, compound="top", style="Sticker.TButton",
                           command=lambda s=st: self._selecionar_sticker(s))
            b.grid(row=i // colunas, column=i % colunas, padx=2, pady=2, sticky="nsew")
            self._botoes_stickers[st.id] = b
        for c in range(colunas):
            grade.columnconfigure(c, weight=1)
        if not self.stickers:
            ttk.Label(grade, text="Nenhum sticker encontrado em assets/stickers (veja stickers.json)",
                      style="Sec.TLabel").grid(row=0, column=0)
        self.lbl_sticker_sel = ttk.Label(aba, text="Nenhum sticker selecionado.", style="Sec.TLabel")
        self.lbl_sticker_sel.pack(anchor="w", pady=(4, 0))

        fs = ttk.LabelFrame(aba, text=" STICKER SELECIONADO / NOVO ", padding=8)
        fs.pack(fill="x", pady=(6, 0))
        self.lbl_escala = self._slider_simples(fs, "Tamanho (% da largura)", self.var_escala, 5, 90, self._ao_mudar_escala)
        self.lbl_angulo = self._slider_simples(fs, "Rotação (graus)", self.var_angulo, -180, 180, self._ao_mudar_angulo)
        b = ttk.Frame(fs)
        b.pack(fill="x", pady=(4, 0))
        ttk.Button(b, text="Remover selecionado", command=self.remover_sticker_ativo).pack(side="left", expand=True, fill="x")
        ttk.Button(b, text="Remover todos", command=self.remover_todos_stickers).pack(side="left", expand=True,
                                                                                    fill="x", padx=(4, 0))

        fr = ttk.LabelFrame(aba, text=" FILTROS DE ROSTO (detecção facial) ", padding=8)
        fr.pack(fill="x", pady=(6, 0))
        cb = ttk.Combobox(fr, values=OPCOES_ROSTO, textvariable=self.var_rosto, state="readonly")
        cb.pack(fill="x")
        cb.bind("<<ComboboxSelected>>", self._ao_mudar_rosto)
        self.lbl_desc_rosto = ttk.Label(fr, text=DESCRICOES_ROSTO["Nenhum"], style="Sec.TLabel",
                                        wraplength=350, font=("Segoe UI", 9))
        self.lbl_desc_rosto.pack(anchor="w", pady=(4, 0))
        ttk.Label(fr, text=self.detector.mensagem, wraplength=350, font=("Segoe UI", 8),
                  foreground=CORES["destaque"] if not self.detector.disponivel else CORES["texto2"]
                  ).pack(anchor="w", pady=(4, 0))

    def _slider_simples(self, pai, rotulo, var, de, ate, comando):
        topo = ttk.Frame(pai)
        topo.pack(fill="x")
        ttk.Label(topo, text=rotulo).pack(side="left")
        lbl = ttk.Label(topo, text=_fmt(var.get()), style="Valor.TLabel")
        lbl.pack(side="right")
        ttk.Scale(pai, from_=de, to=ate, variable=var,
                  command=lambda v: (lbl.configure(text=_fmt(round(float(v)))), comando())).pack(fill="x", pady=(0, 4))
        return lbl

    def _montar_params(self, frame: ttk.LabelFrame, params: list[Param], valores: dict) -> None:
        for w in frame.winfo_children():
            w.destroy()
        variaveis = self._vars_params[str(frame)] = []  # mantém referências às variáveis Tk
        if not params:
            ttk.Label(frame, text="Este item não possui parâmetros.", style="Sec.TLabel").pack(anchor="w")
            return
        for p in params:
            linha = ttk.Frame(frame)
            linha.pack(fill="x", pady=(0, 4))
            topo = ttk.Frame(linha)
            topo.pack(fill="x")
            ttk.Label(topo, text=p.rotulo).pack(side="left")
            if p.opcoes:
                var = tk.StringVar(value=valores[p.nome])
                cb = ttk.Combobox(linha, values=p.opcoes, textvariable=var, state="readonly")
                cb.pack(fill="x")
                cb.bind("<<ComboboxSelected>>",
                        lambda e, p=p, var=var: self._mudar_param(valores, p, var.get(), None))
            else:
                lbl = ttk.Label(topo, text=_fmt(valores[p.nome]), style="Valor.TLabel")
                lbl.pack(side="right")
                var = tk.DoubleVar(value=valores[p.nome])
                ttk.Scale(linha, from_=p.minimo, to=p.maximo, variable=var,
                          command=lambda v, p=p, lbl=lbl: self._mudar_param(valores, p, v, lbl)).pack(fill="x")
            variaveis.append(var)

    def _mudar_param(self, valores: dict, p: Param, valor, lbl) -> None:
        v = p.normalizar(valor)
        if valores.get(p.nome) == v:
            return
        valores[p.nome] = v
        if lbl is not None:
            lbl.configure(text=_fmt(v))
        self._agendar_render(30)

    def _atalhos(self) -> None:
        self.bind_all("<Control-o>", lambda e: self.abrir_foto())
        self.bind_all("<Control-w>", lambda e: self.alternar_webcam())
        self.bind_all("<Control-s>", lambda e: self.salvar())
        self.bind_all("<Control-S>", lambda e: self.salvar(rapido=True))
        self.bind_all("<Control-z>", lambda e: self.desfazer())
        self.bind_all("<Control-r>", lambda e: self.resetar())
        self.bind_all("<Return>", lambda e: self._aplicar_aba_atual())
        self.bind_all("<Escape>", lambda e: self._limpar_selecao("Prévia cancelada."))
        self.bind_all("<Delete>", lambda e: self.remover_sticker_ativo())

    def _imprimir_boas_vindas(self) -> None:
        console.banner(f"{NOME_APP} - editor de efeitos para imagens e vídeo (OpenCV {cv2.__version__})", [
            f"{len(FILTROS)} filtros | {len(OPERACOES)} operações | {len(self.stickers)} stickers",
            self.detector.mensagem,
            "As descrições dos filtros selecionados também aparecem aqui no console.",
        ])

    # Laço principal (vídeo e animação de stickers)

    def _laco(self) -> None:
        try:
            if self.modo == "video" and self.fonte_video is not None:
                self._passo_video()
            elif self.modo == "foto" and self._tem_animacao():
                agora = time.monotonic()
                if agora - self._t_anim >= 1 / 20:
                    self._t_anim = agora
                    self._compor_e_mostrar()
        except Exception as e: 
            traceback.print_exc()
            self._status(f"Erro: {e}")
        self.after(12, self._laco)

    def _tem_animacao(self) -> bool:
        if any(c.sticker.animado for c in self.colocados):
            return True
        itens = PRESETS_ROSTO.get(self.var_rosto.get(), [])
        return bool(self._rostos_atuais) and any(
            self.stickers_por_id.get(i.sticker) is not None and self.stickers_por_id[i.sticker].animado for i in itens)

    def _passo_video(self) -> None:
        fv = self.fonte_video
        if fv.estado == "abrindo":
            self._status("Abrindo a câmera/vídeo...")
            return
        if fv.estado == "erro":
            erro = fv.erro
            self.parar_video()
            messagebox.showerror("Vídeo", erro)
            return
        fid, frame = fv.ler()
        if frame is None or fid == self.ultimo_frame_id:
            return
        if self.ultimo_frame_id == -1:  # primeiro frame recebido
            origem = "Arquivo de vídeo" if fv.eh_arquivo else "Webcam"
            self._status(f"{origem} rodando: filtros, operações, stickers e filtros de rosto em tempo real. "
                         "Salvar grava o frame atual.")
        self.ultimo_frame_id = fid
        if self.var_espelhar.get() and not fv.eh_arquivo:
            frame = cv2.flip(frame, 1)
        frame = limitar_tamanho(frame, LADO_MAX_VIDEO)
        self.original = frame
        self._cache_base = self._finalizar_base(self.pipeline.aplicar(frame), frame)
        self._compor_e_mostrar()
        agora = time.perf_counter()
        dt = agora - self._t_frame
        self._t_frame = agora
        if dt > 0:
            self._fps = 0.9 * self._fps + 0.1 * (1.0 / dt) if self._fps else 1.0 / dt

    # Renderização

    def _etapa_preview(self) -> Etapa | None:
        if not self.var_preview.get():
            return None
        aba = self.notebook.index("current")
        canal = self.var_canal.get()
        if aba == ABA_FILTROS and self.filtro_sel is not None:
            return Etapa(self.filtro_sel, dict(self.params_filtro), canal)
        if aba == ABA_OPERACOES and self.op_sel is not None and self.img_b is not None:
            return Etapa(self.op_sel, dict(self.params_op), canal, self.img_b, self.nome_b, self._cache_b_prev)
        return None

    def _finalizar_base(self, img: np.ndarray, original: np.ndarray) -> np.ndarray:
        """Prévia + visualização do canal + efeitos de rosto (tudo menos os stickers)."""
        prev = self._etapa_preview()
        if prev is not None:
            img = prev.aplicar(img)
        img = visualizar_canal(img, self.var_canal.get(), self.var_canal_cinza.get())
        preset = self.var_rosto.get()
        if preset != "Nenhum" and self.detector.disponivel:
            if self.modo == "video":
                self._rostos_atuais = self.suavizador.atualizar(self.detector.detectar(original))
            else:
                if self.rostos_foto is None:
                    self.rostos_foto = self.detector.detectar(original)
                self._rostos_atuais = self.rostos_foto
            img = aplicar_efeito_rosto(img, self._rostos_atuais, preset)
        else:
            self._rostos_atuais = []
        return img

    def _renderizar_foto(self) -> None:
        self._render_agendado = None
        if self.modo != "foto" or self.resultado_etapas is None:
            return
        try:
            self._cache_base = self._finalizar_base(self.resultado_etapas, self.original)
        except Exception as e:
            traceback.print_exc()
            self._status(f"Erro ao aplicar: {e}")
            self._cache_base = self.resultado_etapas
        self._compor_e_mostrar()

    def _agendar_render(self, atraso: int = 15) -> None:
        if self.modo != "foto":
            return  # no vídeo, cada frame já é renderizado
        if self._render_agendado is not None:
            self.after_cancel(self._render_agendado)
        self._render_agendado = self.after(atraso, self._renderizar_foto)

    def _compor_e_mostrar(self) -> None:
        if self._cache_base is None:
            return
        saida = self._cache_base.copy()
        t = time.monotonic()
        preset = self.var_rosto.get()
        if preset in PRESETS_ROSTO and self._rostos_atuais:
            desenhar_stickers_rosto(saida, self._rostos_atuais, preset, self.stickers_por_id, t)
        for c in self.colocados:
            c.desenhar(saida, t)
        self._mostrar(saida)

    def _mostrar(self, img: np.ndarray) -> None:
        self.ultimo_exibido = img
        cw = max(1, self.canvas.winfo_width())
        ch = max(1, self.canvas.winfo_height())
        h, w = img.shape[:2]
        s = min(cw / w, ch / h)
        nw, nh = max(1, int(w * s)), max(1, int(h * s))
        vis = cv2.resize(img, (nw, nh), interpolation=cv2.INTER_AREA if s < 1 else cv2.INTER_LINEAR)
        foto = ImageTk.PhotoImage(Image.fromarray(cv2.cvtColor(vis, cv2.COLOR_BGR2RGB)))
        ox, oy = (cw - nw) // 2, (ch - nh) // 2
        if self._item_img is None:
            self._item_img = self.canvas.create_image(ox, oy, anchor="nw", image=foto)
            self.canvas.itemconfigure(self._txt_vazio, state="hidden")
        else:
            self.canvas.coords(self._item_img, ox, oy)
            self.canvas.itemconfigure(self._item_img, image=foto)
        self._foto_tk = foto
        self._exib = (ox, oy, s)
        self._atualizar_info()

    def _atualizar_info(self) -> None:
        if self.ultimo_exibido is None:
            self.lbl_info.configure(text="")
            return
        h, w = self.ultimo_exibido.shape[:2]
        partes = [f"{w}×{h} px", f"{len(self.pipeline)} etapa(s)", f"{len(self.colocados)} sticker(s)",
                  f"canal {self.var_canal.get()}"]
        if self.modo == "video":
            partes.append(f"{self._fps:.0f} FPS")
        if self._rostos_atuais:
            partes.append(f"{len(self._rostos_atuais)} rosto(s)")
        self.lbl_info.configure(text="   |   ".join(partes))

    def _status(self, texto: str) -> None:
        self.lbl_status.configure(text=texto)

    def _atualizar_modo(self) -> None:
        if self.modo == "foto":
            texto = "MODO FOTO"
        elif self.modo == "video":
            texto = "MODO VÍDEO - " + ("arquivo" if self.fonte_video and self.fonte_video.eh_arquivo else "webcam")
        else:
            texto = "SEM IMAGEM"
        self.lbl_modo.configure(text=texto)
        video_ativo = self.modo == "video" and self.fonte_video is not None
        self.btn_webcam.configure(text="Parar vídeo" if video_ativo else "Iniciar webcam")

    # Foto / vídeo

    def abrir_foto(self, caminho: str | None = None) -> None:
        if caminho is None:
            caminho = filedialog.askopenfilename(
                title="Abrir foto", initialdir=PASTA_IMAGENS,
                filetypes=[("Imagens", "*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp"), ("Todos", "*.*")])
        if not caminho:
            return
        img = ler_imagem(caminho)
        if img is None:
            messagebox.showerror("Abrir foto", f"Não foi possível ler a imagem:\n{caminho}")
            return
        if self.fonte_video is not None:
            self.parar_video()
        h0, w0 = img.shape[:2]
        img = limitar_tamanho(img, LADO_MAX_FOTO)
        self._definir_foto(img, os.path.basename(caminho), manter_edicao=False)
        msg = f"Foto aberta: {os.path.basename(caminho)} ({w0}×{h0})"
        if img.shape[:2] != (h0, w0):
            msg += f" - reduzida para {img.shape[1]}×{img.shape[0]} para edição"
        self._status(msg)
        console.log("foto", msg)

    def _definir_foto(self, img: np.ndarray, nome: str, manter_edicao: bool) -> None:
        self.modo = "foto"
        self.nome_atual = nome
        self.original = img
        self.rostos_foto = None
        if not manter_edicao:
            self.pipeline.limpar()
            self.colocados.clear()
            self.historico.clear()
            self.colocado_ativo = None
        self._recalcular_etapas()
        self._atualizar_lista_etapas()
        self._atualizar_modo()
        self._agendar_render(0)

    def alternar_webcam(self) -> None:
        if self.fonte_video is not None:
            # parar o vídeo volta para a tela inicial (sem imagem, sem etapas e sem stickers)
            self.parar_video()
            self._limpar_edicao()
            self._status("Vídeo parado. Abra uma foto (Ctrl+O) ou inicie a webcam (Ctrl+W).")
            console.log("vídeo", "parado - de volta à tela inicial")
        else:
            self.iniciar_video(self._indice_camera())

    # Câmeras conectadas (caixa de seleção do painel VÍDEO)

    def _atualizar_cameras(self) -> None:
        """Relê as câmeras conectadas e mantém a escolhida, se ela ainda existir."""
        indice_atual = self._indice_camera() if self.cameras else 0
        self.cameras = listar_cameras() or [(0, "Câmera 0")]
        rotulos = [f"{i}: {nome}" for i, nome in self.cameras]
        self.cmb_camera.configure(values=rotulos)
        indices = [i for i, _ in self.cameras]
        self.cmb_camera.current(indices.index(indice_atual) if indice_atual in indices else 0)

    def _indice_camera(self) -> int:
        """Número (para o OpenCV) da câmera escolhida na caixa de seleção."""
        pos = self.cmb_camera.current()
        return self.cameras[pos][0] if 0 <= pos < len(self.cameras) else 0

    def selecionar_camera(self, indice: int) -> None:
        indices = [i for i, _ in self.cameras]
        if indice not in indices:  # número fora da lista (ex.: --webcam 5): mostra assim mesmo
            self.cameras.append((indice, f"Câmera {indice}"))
            self.cmb_camera.configure(values=[f"{i}: {nome}" for i, nome in self.cameras])
            indices.append(indice)
        self.cmb_camera.current(indices.index(indice))

    def _ao_escolher_camera(self, _evento=None) -> None:
        # com a webcam ligada, troca de câmera na hora (mantém filtros e stickers)
        if self.fonte_video is not None and not self.fonte_video.eh_arquivo:
            self.iniciar_video(self._indice_camera())

    def abrir_video(self) -> None:
        caminho = filedialog.askopenfilename(
            title="Abrir arquivo de vídeo",
            filetypes=[("Vídeos", "*.mp4 *.avi *.mov *.mkv *.wmv *.webm"), ("Todos", "*.*")])
        if caminho:
            self.iniciar_video(caminho)

    def iniciar_video(self, fonte: int | str) -> None:
        if self.fonte_video is not None:
            self.parar_video()
        # etapas e stickers já aplicados são mantidos e passam a valer para o vídeo
        self.fonte_video = FonteVideo(fonte)
        self.fonte_video.iniciar()
        self.modo = "video"
        self.ultimo_frame_id = -1
        self.suavizador = SuavizadorRostos()
        self._fps = 0.0
        self._atualizar_modo()
        origem = f"arquivo {os.path.basename(fonte)}" if isinstance(fonte, str) else f"webcam {fonte}"
        self._status(f"Iniciando {origem}... (filtros, operações e stickers são aplicados em tempo real)")
        console.log("vídeo", f"iniciando {origem}")

    def parar_video(self) -> None:
        """Para a câmera/vídeo e tira a imagem da tela (volta a mostrar "Abra uma foto...")."""
        fv, self.fonte_video = self.fonte_video, None
        if fv is not None:
            fv.parar()
        self.modo = None
        self.original = None
        self.resultado_etapas = None
        self._cache_base = None
        self.ultimo_exibido = None
        if self._item_img is not None:
            self.canvas.delete(self._item_img)
            self._item_img = None
        self.canvas.itemconfigure(self._txt_vazio, state="normal")
        self._atualizar_modo()

    def salvar(self, rapido: bool = False) -> None:
        if self.ultimo_exibido is None:
            self._status("Nada para salvar: abra uma foto ou inicie a webcam.")
            return
        img = self.ultimo_exibido.copy()  # congela o frame atual
        os.makedirs(PASTA_SAIDAS, exist_ok=True)
        nome = time.strftime("instagramstories_%Y%m%d_%H%M%S.png")
        if rapido:
            caminho = os.path.join(PASTA_SAIDAS, nome)
        else:
            caminho = filedialog.asksaveasfilename(
                title="Salvar imagem", initialdir=PASTA_SAIDAS, initialfile=nome, defaultextension=".png",
                filetypes=[("PNG", "*.png"), ("JPEG", "*.jpg"), ("BMP", "*.bmp")])
            if not caminho:
                return
        if salvar_imagem(caminho, img):
            self._status(f"Imagem salva em {caminho}")
            console.log("salvo", caminho)
        else:
            messagebox.showerror("Salvar", f"Não foi possível salvar em:\n{caminho}")

    # Etapas (filtros/operações)

    def _recalcular_etapas(self) -> None:
        if self.original is None:
            self.resultado_etapas = None
            return
        if self.modo == "foto":
            self.config(cursor="watch")
            self.update_idletasks()
            try:
                self.resultado_etapas = self.pipeline.aplicar(self.original)
            finally:
                self.config(cursor="")

    def _atualizar_lista_etapas(self) -> None:
        self.lista_etapas.delete(0, "end")
        for i, e in enumerate(self.pipeline.etapas, 1):
            # nome na 1ª linha, parâmetros/canal na 2ª (o painel é estreito)
            self.lista_etapas.insert("end", f"{i}. {e.item.nome}")
            detalhes = e.detalhes()
            if detalhes:
                self.lista_etapas.insert("end", f"     {detalhes}")
                self.lista_etapas.itemconfigure("end", foreground=CORES["texto2"])
        if not self.pipeline.etapas and self.original is not None:  # sem imagem: lista vazia, como no início
            self.lista_etapas.insert("end", "(nenhuma - imagem original)")
        self.lista_etapas.see("end")
        self._atualizar_info()

    def _verificar_imagem(self) -> bool:
        if self.original is None:
            self._status("Abra uma foto ou inicie a webcam primeiro.")
            return False
        return True

    def aplicar_filtro(self) -> None:
        if self.filtro_sel is None:
            self._status("Selecione um filtro na lista.")
            return
        if not self._verificar_imagem():
            return
        self._adicionar_etapa(Etapa(self.filtro_sel, dict(self.params_filtro), self.var_canal.get()))

    def aplicar_operacao(self) -> None:
        if self.op_sel is None:
            self._status("Selecione uma operação na lista.")
            return
        if self.img_b is None:
            self._status("Escolha a imagem B (arquivo, textura ou imagem atual).")
            return
        if not self._verificar_imagem():
            return
        self._adicionar_etapa(Etapa(self.op_sel, dict(self.params_op), self.var_canal.get(),
                                    self.img_b, self.nome_b))

    def _aplicar_aba_atual(self) -> None:
        aba = self.notebook.index("current")
        if aba == ABA_FILTROS:
            self.aplicar_filtro()
        elif aba == ABA_OPERACOES:
            self.aplicar_operacao()

    def _adicionar_etapa(self, etapa: Etapa) -> None:
        if self.modo == "foto" and self.resultado_etapas is not None:
            self.config(cursor="watch")
            self.update_idletasks()
            try:
                self.resultado_etapas = etapa.aplicar(self.resultado_etapas)
            finally:
                self.config(cursor="")
        self.pipeline.adicionar(etapa)
        self.historico.append(("etapa", etapa))
        console.log("aplicado", etapa.resumo())
        self._limpar_selecao(f"Aplicado: {etapa.resumo()}. Selecione outro item para combinar.")
        self._atualizar_lista_etapas()

    def _limpar_selecao(self, mensagem: str = "") -> None:
        self.filtro_sel = None
        self.op_sel = None
        for tree in (self.tree_filtros, self.tree_ops):
            if tree.selection():
                tree.selection_remove(tree.selection())
        for frame in (self.frame_params_filtro, self.frame_params_op):
            for w in frame.winfo_children():
                w.destroy()
            ttk.Label(frame, text="Selecione um item.", style="Sec.TLabel").pack(anchor="w")
        if mensagem:
            texto = mensagem
            if mensagem.startswith("Aplicado"):
                texto = "✔ " + mensagem + "\n\nO próximo filtro/operação será aplicado sobre o resultado atual."
            self._escrever(self.txt_desc_filtro, texto)
            self._escrever(self.txt_desc_op, texto)
            self._status(mensagem)
        self._agendar_render(0)

    def desfazer(self) -> None:
        if not self.historico:
            self._status("Nada para desfazer.")
            return
        acao = self.historico.pop()
        tipo = acao[0]
        if tipo == "etapa":
            self.pipeline.remover_ultima()
            self._recalcular_etapas()
            self._status(f"Desfeito: {acao[1].resumo()}")
        elif tipo == "sticker":
            if acao[1] in self.colocados:
                self.colocados.remove(acao[1])
            self._status(f"Sticker removido: {acao[1].sticker.nome}")
        elif tipo == "remover_sticker":
            self.colocados.insert(min(acao[2], len(self.colocados)), acao[1])
            self._status(f"Sticker restaurado: {acao[1].sticker.nome}")
        elif tipo == "editar_sticker":
            st, (u, v, esc, ang) = acao[1], acao[2]
            st.u, st.v, st.escala, st.angulo = u, v, esc, ang
            self._status(f"Desfeita edição do sticker {st.sticker.nome}")
        self._atualizar_lista_etapas()
        self._agendar_render(0)

    def resetar(self) -> None:
        self._limpar_edicao()
        self._status("Resetado: todas as operações e stickers foram removidos (imagem original).")
        console.log("reset", "imagem original restaurada")

    def _limpar_edicao(self) -> None:
        """Remove etapas, stickers e filtro de rosto e volta canal e descrições ao início."""
        self.pipeline.limpar()
        self.colocados.clear()
        self.historico.clear()
        self.colocado_ativo = None
        self.var_rosto.set("Nenhum")
        self.lbl_desc_rosto.configure(text=DESCRICOES_ROSTO["Nenhum"])
        self.var_canal.set("RGB")
        self.var_canal_cinza.set(False)
        self._recalcular_etapas()
        self._atualizar_lista_etapas()
        self._limpar_selecao()
        self._escrever(self.txt_desc_filtro, TEXTO_INICIAL_FILTRO)
        self._escrever(self.txt_desc_op, TEXTO_INICIAL_OP)

    # Eventos dos painéis

    def _ao_mudar_canal(self) -> None:
        canal = self.var_canal.get()
        self._status(f"Canal: {canal} - filtros/operações atuam em: {DESCRICAO_CANAIS[canal]}.")
        self._agendar_render(0)

    def _ao_selecionar_filtro(self, _evento=None) -> None:
        sel = self.tree_filtros.selection()
        if not sel:
            return
        iid = sel[0]
        if iid not in FILTROS_POR_ID:
            self.filtro_sel = None
            self._escrever(self.txt_desc_filtro, f"Categoria: {iid[4:]}\n\nExpanda e selecione um filtro.")
            self._agendar_render()
            return
        f = FILTROS_POR_ID[iid]
        self.filtro_sel = f
        self.params_filtro = f.params_padrao()
        canal = self.var_canal.get()
        texto = f.texto_descricao() + f"\n\nCanal atual: {canal} ({DESCRICAO_CANAIS[canal]})."
        self._escrever(self.txt_desc_filtro, texto)
        console.bloco("filtro selecionado", texto)
        self._montar_params(self.frame_params_filtro, f.params, self.params_filtro)
        self._status(f"Prévia: {f.nome}. Ajuste os parâmetros e clique em 'Aplicar filtro' (Enter).")
        self._agendar_render()

    def _ao_selecionar_operacao(self, _evento=None) -> None:
        sel = self.tree_ops.selection()
        if not sel:
            return
        o = OPERACOES_POR_ID[sel[0]]
        self.op_sel = o
        self.params_op = o.params_padrao()
        texto = o.texto_descricao()
        if self.img_b is None:
            texto += "\n\n⚠ Escolha a imagem B acima para ver a prévia."
        self._escrever(self.txt_desc_op, texto)
        console.bloco("operação selecionada", texto)
        self._montar_params(self.frame_params_op, o.params, self.params_op)
        self._agendar_render()

    def _definir_b(self, img: np.ndarray, nome: str) -> None:
        self.img_b = img
        self.nome_b = nome
        self._cache_b_prev = {}
        rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        im = Image.fromarray(rgb)
        im.thumbnail((120, 80))
        self._thumb_b = ImageTk.PhotoImage(im)
        self.lbl_thumb_b.configure(image=self._thumb_b, text="", width=120, height=80)
        self.lbl_nome_b.configure(text=f"B: {nome} ({img.shape[1]}×{img.shape[0]})")
        self._status(f"Imagem B definida: {nome}. B é ajustada ao tamanho de A (sem distorcer).")
        self._agendar_render(0)

    def carregar_imagem_b(self) -> None:
        caminho = filedialog.askopenfilename(
            title="Carregar imagem B", initialdir=PASTA_IMAGENS,
            filetypes=[("Imagens", "*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp"), ("Todos", "*.*")])
        if not caminho:
            return
        img = ler_imagem(caminho)
        if img is None:
            messagebox.showerror("Imagem B", f"Não foi possível ler:\n{caminho}")
            return
        self._definir_b(limitar_tamanho(img, LADO_MAX_FOTO), os.path.basename(caminho))

    def usar_textura_como_b(self) -> None:
        nome = self.var_textura.get()
        h, w = self.original.shape[:2] if self.original is not None else (1280, 720)
        self._definir_b(gerar_textura(nome, h, w).copy(), nome)

    def usar_atual_como_b(self) -> None:
        if not self._verificar_imagem():
            return
        base = self.resultado_etapas if self.modo == "foto" else self.pipeline.aplicar(self.original)
        self._definir_b(base.copy(), "frame capturado" if self.modo == "video" else "imagem atual")

    # stickers
    def _selecionar_sticker(self, st) -> None:
        if self.sticker_sel is not None and self.sticker_sel.id in self._botoes_stickers:
            self._botoes_stickers[self.sticker_sel.id].configure(style="Sticker.TButton")
        if self.sticker_sel is st:  # clicar de novo desmarca
            self.sticker_sel = None
            self.lbl_sticker_sel.configure(text="Nenhum sticker selecionado.")
            return
        self.sticker_sel = st
        self._botoes_stickers[st.id].configure(style="StickerSel.TButton")
        extra = f" - animado, {len(st.quadros)} quadros a {st.fps:g} fps (spritesheet)" if st.animado else ""
        self.lbl_sticker_sel.configure(text=f"Selecionado: {st.nome}{extra}. Clique na imagem.")
        self._status(f"Sticker '{st.nome}' selecionado: clique na imagem para posicioná-lo.")

    def _ao_mudar_escala(self) -> None:
        c = self.colocado_ativo
        if c is not None and c in self.colocados:
            c.escala = self.var_escala.get() / 100
            self._redesenhar_stickers()

    def _ao_mudar_angulo(self) -> None:
        c = self.colocado_ativo
        if c is not None and c in self.colocados:
            c.angulo = self.var_angulo.get()
            self._redesenhar_stickers()

    def remover_sticker_ativo(self) -> None:
        c = self.colocado_ativo
        if c is not None and c in self.colocados:
            idx = self.colocados.index(c)
            self.colocados.remove(c)
            self.historico.append(("remover_sticker", c, idx))
            self.colocado_ativo = None
            self._status(f"Sticker removido: {c.sticker.nome} (Ctrl+Z desfaz)")
            self._redesenhar_stickers()

    def remover_todos_stickers(self) -> None:
        for i, c in enumerate(list(self.colocados)):
            self.historico.append(("remover_sticker", c, i))
        self.colocados.clear()
        self.colocado_ativo = None
        self._redesenhar_stickers()

    def _redesenhar_stickers(self) -> None:
        if self.modo == "foto":
            self._compor_e_mostrar()
        self._atualizar_info()

    def _ao_mudar_rosto(self, _evento=None) -> None:
        preset = self.var_rosto.get()
        self.lbl_desc_rosto.configure(text=DESCRICOES_ROSTO.get(preset, ""))
        console.log("filtro de rosto", f"{preset}: {DESCRICOES_ROSTO.get(preset, '')}")
        if preset != "Nenhum" and not self.detector.disponivel:
            messagebox.showinfo("Detecção de face", self.detector.mensagem)
            self.var_rosto.set("Nenhum")
            return
        if self.modo == "foto" and preset != "Nenhum":
            if self.rostos_foto is None:
                self.config(cursor="watch")
                self.update_idletasks()
                self.rostos_foto = self.detector.detectar(self.original)
                self.config(cursor="")
            n = len(self.rostos_foto)
            self._status(f"{n} rosto(s) detectado(s) com {self.detector.backend}." if n else
                         "Nenhum rosto detectado nesta foto.")
        self._agendar_render(0)

    # Mouse no canvas

    def _coord_imagem(self, ex: int, ey: int, limitar: bool = True):
        img = self.ultimo_exibido
        if img is None:
            return None
        ox, oy, s = self._exib
        h, w = img.shape[:2]
        x, y = (ex - ox) / s, (ey - oy) / s
        if limitar and not (0 <= x < w and 0 <= y < h):
            return None
        return x / w, y / h, w, h

    def _sticker_em(self, u, v, w, h):
        for c in reversed(self.colocados):
            if c.contem(u, v, w, h):
                return c
        return None

    def _ativar(self, c: StickerColocado) -> None:
        self.colocado_ativo = c
        self.var_escala.set(round(c.escala * 100))
        self.var_angulo.set(round(c.angulo))
        self.lbl_escala.configure(text=_fmt(round(c.escala * 100)))
        self.lbl_angulo.configure(text=_fmt(round(c.angulo)))

    def _ao_clicar(self, e) -> None:
        pos = self._coord_imagem(e.x, e.y)
        if pos is None:
            return
        u, v, w, h = pos
        alvo = self._sticker_em(u, v, w, h)
        if alvo is not None:  # começa a arrastar um sticker existente
            self.colocados.remove(alvo)
            self.colocados.append(alvo)  # traz para frente
            self._ativar(alvo)
            self._arrasto = [alvo, u - alvo.u, v - alvo.v, (alvo.u, alvo.v, alvo.escala, alvo.angulo), False]
            self._status(f"Sticker '{alvo.sticker.nome}' selecionado: arraste para mover.")
            return
        if self.notebook.index("current") != ABA_STICKERS:
            self._status("Para colocar stickers, abra a aba 'Stickers & Rosto' e escolha um sticker.")
            return
        if self.sticker_sel is None:
            self._status("Escolha um sticker na aba 'Stickers & Rosto' e clique na imagem.")
            return
        novo = StickerColocado(self.sticker_sel, u, v, self.var_escala.get() / 100, self.var_angulo.get())
        self.colocados.append(novo)
        self.historico.append(("sticker", novo))
        self.colocado_ativo = novo
        self._status(f"Sticker '{novo.sticker.nome}' colocado em ({int(u * w)}, {int(v * h)}).")
        self._redesenhar_stickers()

    def _ao_arrastar(self, e) -> None:
        if not self._arrasto:
            return
        pos = self._coord_imagem(e.x, e.y, limitar=False)
        if pos is None:
            return
        u, v, _, _ = pos
        st, du, dv = self._arrasto[0], self._arrasto[1], self._arrasto[2]
        st.u = min(max(u - du, -0.2), 1.2)
        st.v = min(max(v - dv, -0.2), 1.2)
        self._arrasto[4] = True
        self._redesenhar_stickers()

    def _ao_soltar(self, _e) -> None:
        if self._arrasto and self._arrasto[4]:
            self.historico.append(("editar_sticker", self._arrasto[0], self._arrasto[3]))
        self._arrasto = None

    def _ao_botao_direito(self, e) -> None:
        pos = self._coord_imagem(e.x, e.y)
        if pos is None:
            return
        alvo = self._sticker_em(*pos)
        if alvo is not None:
            self.colocado_ativo = alvo
            self.remover_sticker_ativo()

    def _ao_rolar(self, e) -> None:
        pos = self._coord_imagem(e.x, e.y)
        if pos is None:
            return
        alvo = self._sticker_em(*pos)
        if alvo is None:
            return
        passo = 1 if e.delta > 0 else -1
        self.historico.append(("editar_sticker", alvo, (alvo.u, alvo.v, alvo.escala, alvo.angulo)))
        if e.state & 0x0001:  # Shift: gira
            alvo.angulo = (alvo.angulo + 7.5 * passo + 180) % 360 - 180
        else:
            alvo.escala = min(max(alvo.escala * (1.1 ** passo), 0.03), 1.5)
        self._ativar(alvo)
        self._redesenhar_stickers()

    def _ao_mover_mouse(self, e) -> None:
        pos = self._coord_imagem(e.x, e.y)
        sobre = pos is not None and self._sticker_em(*pos) is not None
        self.canvas.configure(cursor="fleur" if sobre else "crosshair")

    def _ao_redimensionar_canvas(self, e) -> None:
        self.canvas.coords(self._txt_vazio, e.width / 2, e.height / 2)
        if self.modo == "foto" and self._cache_base is not None:
            self._compor_e_mostrar()

    def _fechar(self) -> None:
        if self.fonte_video is not None:
            self.fonte_video.parar()
        self.destroy()


def main(argv: list[str] | None = None) -> None:
    parser = argparse.ArgumentParser(description=f"{NOME_APP} - efeitos em imagens e vídeo com OpenCV")
    parser.add_argument("imagem", nargs="?", help="foto para abrir ao iniciar")
    parser.add_argument("--webcam", nargs="?", const=0, type=int, help="inicia a webcam (índice opcional)")
    parser.add_argument("--video", help="arquivo de vídeo para abrir ao iniciar")
    args = parser.parse_args(argv)
    if os.name == "nt":
        try:
            import ctypes
            ctypes.windll.shcore.SetProcessDpiAwareness(1)
        except Exception:
            pass
    app = App()
    if args.imagem:
        app.after(150, lambda: app.abrir_foto(args.imagem))
    elif args.video:
        app.after(150, lambda: app.iniciar_video(args.video))
    elif args.webcam is not None:
        app.selecionar_camera(args.webcam)
        app.after(150, lambda: app.iniciar_video(args.webcam))
    app.mainloop()
