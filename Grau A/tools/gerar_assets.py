"""
Gera todas as imagens (pixel art) do jogo de forma procedural.

Uso   python3 tools/gerar_assets.py
"""
import math
import os
import random
from PIL import Image, ImageDraw

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets")
os.makedirs(OUT, exist_ok=True)
rng = random.Random(2026)


def save(img, name):
    img.save(os.path.join(OUT, name))
    print("gerado", name, img.size)


def clamp(v):
    return max(0, min(255, int(v)))


def shade(c, f):
    return (clamp(c[0] * f), clamp(c[1] * f), clamp(c[2] * f), c[3] if len(c) > 3 else 255)


# BLOCOS (atlas horizontal 16x16 por bloco). Indice 0 = ar (vazio).
# 0 ceu | 1 grama | 2 terra | 3 pedra | 4 carvao | 5 ferro | 6 madeira | 7 folhas | 8 brick
T = 16


def noisy_tile(base, dark, light, p_dark=0.22, p_light=0.12):
    im = Image.new("RGBA", (T, T), base + (255,))
    px = im.load()
    for y in range(T):
        for x in range(T):
            r = rng.random()
            if r < p_dark:
                px[x, y] = dark + (255,)
            elif r < p_dark + p_light:
                px[x, y] = light + (255,)
    return im


def edge_shade(im, f=0.82):
    px = im.load()
    for i in range(T):
        px[i, T - 1] = shade(px[i, T - 1], f)
        px[T - 1, i] = shade(px[T - 1, i], f)
    return im


def blob(px, cx, cy, color, hi):
    for dx, dy in [(0, 0), (1, 0), (0, 1), (-1, 0), (1, 1)]:
        x, y = cx + dx, cy + dy
        if 0 <= x < T and 0 <= y < T:
            px[x, y] = color + (255,)
    if 0 <= cx < T and 0 <= cy < T:
        px[cx, cy] = hi + (255,)


def make_tiles():
    tiles = []
    tiles.append(Image.new("RGBA", (T, T), (0, 0, 0, 0)))  # 0 ar

    dirt_base, dirt_dark, dirt_light = (134, 96, 67), (108, 74, 50), (158, 116, 82)

    # 1 grama
    g = noisy_tile(dirt_base, dirt_dark, dirt_light)
    px = g.load()
    for x in range(T):
        depth = 4 + (1 if rng.random() < 0.5 else 0) + (1 if rng.random() < 0.25 else 0)
        for y in range(depth):
            c = (84, 168, 56) if rng.random() > 0.2 else (66, 140, 46)
            if y == 0:
                c = (118, 200, 78) if rng.random() > 0.3 else (96, 180, 64)
            px[x, y] = c + (255,)
    tiles.append(edge_shade(g))

    # 2 terra
    d = noisy_tile(dirt_base, dirt_dark, dirt_light)
    px = d.load()
    for _ in range(3):
        x, y = rng.randrange(1, T - 2), rng.randrange(1, T - 2)
        px[x, y] = (150, 150, 150, 255)
        px[x + 1, y] = (120, 120, 124, 255)
    tiles.append(edge_shade(d))

    stone_base, stone_dark, stone_light = (124, 124, 132), (98, 98, 108), (150, 150, 158)

    # 3 pedra
    s = noisy_tile(stone_base, stone_dark, stone_light, 0.25, 0.14)
    px = s.load()
    x, y = rng.randrange(3, 12), rng.randrange(2, 6)
    for _ in range(6):
        px[x, y] = stone_dark + (255,)
        x += rng.choice([-1, 0, 1])
        y += 1
        x = max(0, min(T - 1, x))
        y = min(T - 1, y)
    tiles.append(edge_shade(s))

    # 4 carvao
    c = noisy_tile(stone_base, stone_dark, stone_light, 0.25, 0.14)
    px = c.load()
    for cx, cy in [(4, 4), (11, 6), (7, 11), (12, 12)]:
        blob(px, cx, cy, (28, 28, 34), (80, 80, 92))
    tiles.append(edge_shade(c))

    # 5 ferro
    f = noisy_tile(stone_base, stone_dark, stone_light, 0.25, 0.14)
    px = f.load()
    for cx, cy in [(3, 5), (10, 3), (6, 11), (12, 10)]:
        blob(px, cx, cy, (206, 140, 92), (240, 200, 150))
    tiles.append(edge_shade(f))

    # 6 madeira (tronco)
    w = Image.new("RGBA", (T, T), (115, 78, 45, 255))
    px = w.load()
    for x in range(T):
        col = (139, 98, 58) if x % 4 in (0, 1) else (96, 64, 36)
        if x in (0, T - 1):
            col = (70, 46, 26)
        for y in range(T):
            px[x, y] = col + (255,)
    for _ in range(4):
        x, y = rng.randrange(2, T - 2), rng.randrange(1, T - 2)
        px[x, y] = (60, 40, 22, 255)
        px[x, y + 1] = (60, 40, 22, 255)
    tiles.append(w)

    # 7 folhas
    lf = noisy_tile((46, 125, 50), (30, 95, 40), (84, 172, 72), 0.3, 0.2)
    px = lf.load()
    for _ in range(14):
        px[rng.randrange(T), rng.randrange(T)] = (0, 0, 0, 0)
    tiles.append(lf)

    # 8 brick
    b = noisy_tile((44, 44, 54), (26, 26, 34), (66, 66, 78), 0.35, 0.2)
    tiles.append(edge_shade(b, 0.7))

    atlas = Image.new("RGBA", (T * len(tiles), T), (0, 0, 0, 0))
    for i, t in enumerate(tiles):
        atlas.paste(t, (i * T, 0))
    save(atlas, "tiles.png")


# RACHADURA (4 frames 16x16) feedback visual enquanto o bloco e minerado
def make_crack():
    frames = 4
    atlas = Image.new("RGBA", (T * frames, T), (0, 0, 0, 0))
    segments = []
    x, y = 8, 1
    for _ in range(14):
        nx = max(0, min(T - 1, x + rng.choice([-2, -1, 0, 1, 2])))
        ny = min(T - 1, y + rng.choice([1, 1, 2]))
        segments.append(((x, y), (nx, ny)))
        x, y = nx, ny
    branch = []
    x, y = 8, 6
    for _ in range(8):
        nx = max(0, min(T - 1, x + rng.choice([1, 2, 2])))
        ny = max(0, min(T - 1, y + rng.choice([-1, 0, 1])))
        branch.append(((x, y), (nx, ny)))
        x, y = nx, ny
    left = []
    x, y = 8, 9
    for _ in range(7):
        nx = max(0, min(T - 1, x - rng.choice([1, 2, 2])))
        ny = max(0, min(T - 1, y + rng.choice([-1, 0, 1])))
        left.append(((x, y), (nx, ny)))
        x, y = nx, ny
    all_segs = segments + branch + left
    for f in range(frames):
        im = Image.new("RGBA", (T, T), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        count = int(len(all_segs) * (f + 1) / frames)
        for a, b in all_segs[:count]:
            d.line([a, b], fill=(10, 8, 12, 210))
        atlas.paste(im, (f * T, 0))
    save(atlas, "crack.png")


# PERSONAGEM (celula 24x24, 6 colunas x 4 animacoes)
# linhas 0 parado | 1 andando | 2 pulando | 3 minerando
CW = CH = 24
SKIN, HAIR = (240, 196, 150), (96, 58, 30)
SHIRT, SHIRT_D = (52, 120, 200), (38, 90, 160)
PANTS, BOOT = (60, 56, 96), (74, 44, 26)
OUTLINE = (24, 20, 30)


def draw_player(bob=0, leg_front=0, leg_back=0, arm_front=(0, 0), arm_back=(0, 0),
                legs_bent=False, pick=None):
    """Desenha o personagem virado para a direita. Retorna imagem 24x24."""
    im = Image.new("RGBA", (CW, CH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    top = 3 + bob

    # perna de tras
    lx = 10 + leg_back
    d.rectangle([lx, 17, lx + 2, 21], fill=shade(PANTS, 0.8))
    d.rectangle([lx, 22, lx + 3, 23], fill=shade(BOOT, 0.8))
    # braco de tras
    ax, ay = 11 + arm_back[0], 11 + bob + arm_back[1]
    d.rectangle([ax, ay, ax + 1, ay + 5], fill=shade(SKIN, 0.85))
    # tronco
    d.rectangle([9, 10 + bob, 14, 17], fill=SHIRT)
    d.rectangle([9, 10 + bob, 9, 17], fill=SHIRT_D)
    d.rectangle([9, 16, 14, 17], fill=PANTS)
    # cabeca
    d.rectangle([8, top, 15, top + 7], fill=SKIN)
    d.rectangle([8, top - 1, 15, top + 2], fill=HAIR)
    d.rectangle([8, top, 9, top + 4], fill=HAIR)
    d.rectangle([13, top + 3, 14, top + 4], fill=(255, 255, 255))
    d.point((14, top + 4), fill=(20, 20, 40))
    # perna da frente
    fx = 12 + leg_front
    if legs_bent:
        d.rectangle([fx, 17, fx + 2, 19], fill=PANTS)
        d.rectangle([fx + 1, 20, fx + 4, 21], fill=BOOT)
    else:
        d.rectangle([fx, 17, fx + 2, 21], fill=PANTS)
        d.rectangle([fx, 22, fx + 3, 23], fill=BOOT)
    # braco da frente
    ax, ay = 12 + arm_front[0], 11 + bob + arm_front[1]
    d.rectangle([ax, ay, ax + 1, ay + 5], fill=SKIN)
    d.rectangle([ax, ay, ax + 1, ay + 1], fill=SHIRT)

    # picareta (cabo + cabeca)
    if pick is not None:
        (x0, y0), (x1, y1) = pick
        d.line([(x0, y0), (x1, y1)], fill=(120, 80, 40, 255), width=1)
        d.rectangle([x1 - 1, y1 - 2, x1 + 1, y1 + 1], fill=(170, 176, 186, 255))
        d.point((x1, y1 - 2), fill=(230, 236, 246, 255))
    return im


def make_player():
    cols, rows = 6, 4
    atlas = Image.new("RGBA", (CW * cols, CH * rows), (0, 0, 0, 0))

    # 0 parado respirando
    idle_bob = [0, 0, 1, 1]
    for i in range(4):
        atlas.paste(draw_player(bob=idle_bob[i]), (i * CW, 0 * CH))

    # 1 andando 6 quadros
    swing = [-3, -2, 0, 2, 3, 0]
    bobs = [0, 1, 0, 0, 1, 0]
    for i in range(6):
        s = swing[i]
        atlas.paste(
            draw_player(bob=bobs[i], leg_front=s, leg_back=-s,
                        arm_front=(-s // 2, 0), arm_back=(s // 2, 0)),
            (i * CW, 1 * CH))

    # 2 pulando subindo / descendo
    atlas.paste(draw_player(legs_bent=True, arm_front=(2, -3), arm_back=(-1, -3), leg_front=2, leg_back=-1),
                (0, 2 * CH))
    atlas.paste(draw_player(legs_bent=False, arm_front=(3, -2), arm_back=(-2, -2), leg_front=3, leg_back=-2),
                (1 * CW, 2 * CH))

    # 3 minerando balanca a picareta em 4 quadros
    swings = [
        ((14, 12), (11, 4)),   # levantada atras
        ((15, 12), (16, 3)),   # para cima
        ((15, 13), (20, 9)),   # para frente
        ((15, 14), (20, 16)),  # para baixo
    ]
    arm_pos = [(-1, -4), (1, -5), (3, -2), (3, 1)]
    for i in range(4):
        atlas.paste(draw_player(arm_front=arm_pos[i], pick=swings[i]), (i * CW, 3 * CH))

    save(atlas, "player.png")


# SLIME (16x16, 4 quadros de pulo, estica)
def make_slime():
    frames = 4
    atlas = Image.new("RGBA", (16 * frames, 16), (0, 0, 0, 0))
    shapes = [(14, 8), (12, 11), (10, 14), (15, 7)]  # (largura, altura)
    BODY, DARK, HI = (84, 206, 96, 235), (36, 128, 62, 255), (190, 250, 190, 255)
    for i, (w, h) in enumerate(shapes):
        im = Image.new("RGBA", (16, 16), (0, 0, 0, 0))
        d = ImageDraw.Draw(im)
        x0 = (16 - w) // 2
        d.ellipse([x0, 16 - h, x0 + w - 1, 15], fill=BODY)
        px = im.load()
        # contorno pixels transparentes vizinhos de pixels preenchidos
        outline = []
        for y in range(16):
            for x in range(16):
                if px[x, y][3] == 0:
                    for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
                        nx, ny = x + dx, y + dy
                        if 0 <= nx < 16 and 0 <= ny < 16 and px[nx, ny][3] > 0 and px[nx, ny] != DARK:
                            outline.append((x, y))
                            break
        for p in outline:
            px[p] = DARK
        px[x0 + 3, 16 - h + 2] = HI
        px[x0 + 4, 16 - h + 2] = HI
        ey = 16 - h + max(3, h // 2 - 1)
        ex = 8 - w // 4
        for e in (ex - 1, ex + w // 2 - 1):
            px[e, ey] = (10, 30, 20, 255)
            px[e, ey + 1] = (10, 30, 20, 255)
        atlas.paste(im, (i * 16, 0))
    save(atlas, "slime.png")


# HUD coracoes e slots da barra de itens
HEART = [
    "..XX.XX..",
    ".XXXXXXX.",
    "XXXXXXXXX",
    "XXXXXXXXX",
    ".XXXXXXX.",
    "..XXXXX..",
    "...XXX...",
    "....X....",
]


def make_hearts():
    atlas = Image.new("RGBA", (32, 16), (0, 0, 0, 0))
    px = atlas.load()
    for idx, (fill, out, hi) in enumerate([
        ((222, 44, 56), (92, 10, 22), (255, 150, 150)),
        ((58, 34, 40), (30, 16, 22), (58, 34, 40)),
    ]):
        ox, oy = idx * 16 + 3, 4
        fills = set()
        for y, row in enumerate(HEART):
            for x, ch in enumerate(row):
                if ch == "X":
                    fills.add((x, y))
        for (x, y) in fills:
            px[ox + x, oy + y] = fill + (255,)
        for (x, y) in fills:
            for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
                n = (x + dx, y + dy)
                if n not in fills and 0 <= ox + n[0] < atlas.width and 0 <= oy + n[1] < 16:
                    px[ox + n[0], oy + n[1]] = out + (255,)
        px[ox + 2, oy + 1] = hi + (255,)
        px[ox + 1, oy + 2] = hi + (255,)
    save(atlas, "hearts.png")


def make_slots():
    atlas = Image.new("RGBA", (40, 20), (0, 0, 0, 0))
    d = ImageDraw.Draw(atlas)
    d.rectangle([0, 0, 19, 19], fill=(20, 22, 34, 170), outline=(150, 156, 176, 255))
    d.rectangle([20, 0, 39, 19], fill=(40, 40, 20, 190), outline=(255, 214, 64, 255))
    d.rectangle([21, 1, 38, 18], outline=(255, 240, 160, 255))
    save(atlas, "slots.png")


# CAMADAS DE FUNDO (512x288, repetem sem emenda na horizontal)
BW, BH = 512, 288


def make_sky():
    im = Image.new("RGBA", (64, BH))
    px = im.load()
    top, mid, bot = (52, 116, 206), (120, 190, 240), (198, 230, 248)
    for y in range(BH):
        t = y / (BH - 1)
        if t < 0.6:
            a, b, u = top, mid, t / 0.6
        else:
            a, b, u = mid, bot, (t - 0.6) / 0.4
        c = tuple(int(a[k] + (b[k] - a[k]) * u) for k in range(3))
        for x in range(64):
            px[x, y] = c + (255,)
    save(im, "bg_sky.png")


def make_stars():
    im = Image.new("RGBA", (BW, BH), (0, 0, 0, 0))
    px = im.load()
    for _ in range(170):
        x, y = rng.randrange(BW), rng.randrange(int(BH * 0.7))
        c = rng.choice([(255, 255, 255), (255, 244, 200), (200, 220, 255)])
        px[x, y] = c + (255,)
        if rng.random() < 0.15:
            for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
                px[(x + dx) % BW, max(0, y + dy)] = c + (130,)
    save(im, "bg_stars.png")


def make_clouds():
    im = Image.new("RGBA", (BW, BH), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    for _ in range(7):
        cx, cy = rng.randrange(BW), rng.randrange(20, 120)
        w = rng.randrange(50, 100)
        for ox in (-BW, 0, BW):  # desenha 3x para a emenda ficar perfeita
            x = cx + ox
            d.ellipse([x, cy, x + w, cy + 22], fill=(255, 255, 255, 235))
            d.ellipse([x + w * 0.2, cy - 10, x + w * 0.65, cy + 16], fill=(255, 255, 255, 235))
            d.ellipse([x + w * 0.5, cy - 4, x + w * 1.0, cy + 20], fill=(255, 255, 255, 235))
            d.rectangle([x + 4, cy + 14, x + w - 2, cy + 20], fill=(226, 236, 248, 235))
    save(im, "bg_clouds.png")


def ridge_layer(name, base, amp, harmonics, color_top, color_body, spikes=False,
                trees=False, phase=0.0):
    """Silhueta periodica soma de senos com ciclos inteiros -> sem emenda."""
    im = Image.new("RGBA", (BW, BH), (0, 0, 0, 0))
    px = im.load()
    for x in range(BW):
        y = base
        for k, a, ph in harmonics:
            v = math.sin(2 * math.pi * k * x / BW + ph + phase)
            if spikes:
                v = 1 - 2 * abs(v)  # picos pontudos (montanhas)
            y += a * v * amp
        ytop = int(y)
        for yy in range(max(0, ytop), BH):
            depth = yy - ytop
            col = color_top if depth < 3 else color_body
            if depth >= 3:
                f = 1 - min(0.35, (depth - 3) / 260.0)
                col = tuple(clamp(c * f) for c in color_body)
            px[x, yy] = col + (255,)
    if trees:
        d = ImageDraw.Draw(im)
        for _ in range(46):
            x = rng.randrange(BW)
            ytop = None
            for yy in range(BH):
                if px[x, yy][3] > 0:
                    ytop = yy
                    break
            if ytop is None:
                continue
            h = rng.randrange(10, 20)
            for ox in (-BW, 0, BW):
                xx = x + ox
                d.rectangle([xx, ytop - 3, xx + 1, ytop + 1], fill=(40, 70, 40, 255))
                d.polygon([(xx - 4, ytop - 2), (xx + 5, ytop - 2), (xx + 0.5, ytop - h)],
                          fill=(30, 92, 52, 255))
    save(im, name)


def make_backgrounds():
    make_sky()
    make_stars()
    make_clouds()
    ridge_layer("bg_mountains.png", 150, 38, [(2, 1.0, 0.4), (5, 0.45, 1.7), (9, 0.2, 2.9)],
                (214, 226, 244), (104, 128, 170), spikes=True)
    ridge_layer("bg_hills_far.png", 200, 22, [(2, 1.0, 1.0), (4, 0.6, 0.2), (7, 0.25, 2.2)],
                (98, 172, 98), (62, 128, 86))
    ridge_layer("bg_hills_near.png", 238, 16, [(3, 1.0, 2.0), (6, 0.5, 0.9), (11, 0.2, 1.4)],
                (74, 150, 74), (44, 104, 62), trees=True)


# ITENS (picareta e espada, 16x16, apontando para CIMA o jogo gira a espada)
def make_items():
    atlas = Image.new("RGBA", (32, 16), (0, 0, 0, 0))
    d = ImageDraw.Draw(atlas)
    WOODC, WOODD = (140, 92, 48, 255), (100, 64, 32, 255)
    METAL, METALD, METALH = (176, 184, 198, 255), (120, 128, 144, 255), (236, 242, 252, 255)
    # 0 picareta (cabo vertical + cabeca no topo)
    d.rectangle([7, 4, 8, 15], fill=WOODC)
    d.line([(7, 4), (7, 15)], fill=WOODD)
    d.rectangle([2, 2, 13, 4], fill=METAL)
    d.rectangle([1, 3, 2, 5], fill=METALD)
    d.rectangle([13, 3, 14, 5], fill=METALD)
    d.line([(3, 2), (12, 2)], fill=METALH)
    # 1 espada (lamina ate o topo, guarda e cabo embaixo)
    ox = 16
    d.rectangle([ox + 7, 1, ox + 8, 11], fill=METAL)
    d.line([(ox + 7, 1), (ox + 7, 11)], fill=METALH)
    d.line([(ox + 8, 2), (ox + 8, 11)], fill=METALD)
    d.point((ox + 7, 0), fill=METALH)
    d.rectangle([ox + 4, 11, ox + 11, 12], fill=(214, 168, 56, 255))
    d.rectangle([ox + 7, 13, ox + 8, 15], fill=WOODC)
    d.point((ox + 7, 15), fill=(214, 168, 56, 255))
    save(atlas, "items.png")


# ZUMBI (celula 24x24, 4 quadros andando, 1 animacao) reaproveita o corpo do
# personagem trocando a paleta e com os bracos esticados para frente
def make_zombie():
    global SKIN, HAIR, SHIRT, SHIRT_D, PANTS, BOOT
    saved = (SKIN, HAIR, SHIRT, SHIRT_D, PANTS, BOOT)
    SKIN, HAIR = (112, 168, 96), (52, 60, 40)
    SHIRT, SHIRT_D = (96, 84, 120), (70, 60, 92)
    PANTS, BOOT = (64, 72, 56), (50, 40, 30)
    atlas = Image.new("RGBA", (CW * 4, CH), (0, 0, 0, 0))
    swing = [-2, 0, 2, 0]
    for i in range(4):
        s = swing[i]
        atlas.paste(draw_player(bob=(1 if i % 2 else 0), leg_front=s, leg_back=-s,
                                arm_front=(3, -2), arm_back=(2, -1)), (i * CW, 0))
    SKIN, HAIR, SHIRT, SHIRT_D, PANTS, BOOT = saved
    # olhos vermelhos
    px = atlas.load()
    for i in range(4):
        for y in range(CH):
            for x in range(i * CW, (i + 1) * CW):
                if px[x, y][:3] == (255, 255, 255):
                    px[x, y] = (230, 50, 50, 255)
    save(atlas, "zombie.png")


# DROPS (12x12) linha 0 = moeda girando (4 quadros), linha 1 = coracao pulsando
def make_drops():
    atlas = Image.new("RGBA", (48, 24), (0, 0, 0, 0))
    d = ImageDraw.Draw(atlas)
    widths = [8, 5, 2, 5]
    for i, w in enumerate(widths):
        cx = i * 12 + 6
        x0, x1 = cx - w // 2, cx + (w - 1) // 2 + 1
        d.ellipse([x0 - 1, 1, x1, 10], fill=(122, 82, 10, 255))          # contorno
        d.ellipse([x0, 2, x1 - 1, 9], fill=(250, 208, 64, 255))
        if w > 3:
            d.point((x0 + 1, 3), fill=(255, 246, 190, 255))
    pulse = [0, 1, 0, -1]
    for i, p in enumerate(pulse):
        ox, oy = i * 12 + 1, 12 + 1
        pts = [(1, 2), (2, 1), (3, 1), (4, 2), (5, 3), (6, 2), (7, 1), (8, 1), (9, 2), (9, 4),
               (8, 5), (7, 6), (6, 7), (5, 8), (4, 7), (3, 6), (2, 5), (1, 4)]
        poly = [(ox + x, oy + y) for x, y in pts]
        d.polygon(poly, fill=(226, 48, 62, 255), outline=(94, 10, 24, 255))
        d.point((ox + 3, oy + 3), fill=(255, 170, 170, 255))
        if p:
            d.point((ox + 5, oy + 9), fill=(0, 0, 0, 0))
    save(atlas, "drops.png")


if __name__ == "__main__":
    make_tiles()
    make_crack()
    make_player()
    make_slime()
    make_hearts()
    make_items()
    make_zombie()
    make_drops()
    make_slots()
    make_backgrounds()
    print("ok -> assets/")
