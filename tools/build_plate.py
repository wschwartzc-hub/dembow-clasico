#!/usr/bin/env python3
"""Arma la imagen base de Dembow Clásico a partir de la placa de metal de ECO (tools/eco_plate.jpg):

    Resources/ui/plate.jpg       la placa con el logo nuevo (sin el logo ni el emblema de ECO)
    Resources/ui/drawer_bg.jpg   charola del teclado (se abre debajo)
    Resources/icon/icon.png      ícono de la app

    python3 tools/build_plate.py
"""
from pathlib import Path
import numpy as np
from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent.parent
UI = ROOT / "Resources" / "ui"
FONTS = ROOT / "Resources" / "fonts"
rng = np.random.default_rng(11)


def feather_mask(size, box, radius, blur):
    m = Image.new("L", size, 0)
    ImageDraw.Draw(m).rounded_rectangle(box, radius=radius, fill=255)
    return m.filter(ImageFilter.GaussianBlur(blur))


def patch(img, src_box, dst_xy, mask_radius=10, blur=6, ellipse=False):
    """Copia un pedazo limpio de textura encima de algo que se quiere borrar."""
    piece = img.crop(src_box)
    w, h = piece.size
    m = Image.new("L", (w, h), 0)
    d = ImageDraw.Draw(m)
    inset = blur * 2
    if ellipse:
        d.ellipse((inset, inset, w - inset, h - inset), fill=255)
    else:
        d.rounded_rectangle((inset, inset, w - inset, h - inset), radius=mask_radius, fill=255)
    m = m.filter(ImageFilter.GaussianBlur(blur))
    img.paste(piece, dst_xy, m)


def noise(h, w, sigma, amp):
    n = rng.standard_normal((h, w)).astype(np.float32)
    im = Image.fromarray(((n - n.min()) / (n.max() - n.min()) * 255).astype(np.uint8))
    if sigma > 0:
        im = im.filter(ImageFilter.GaussianBlur(sigma))
    a = np.asarray(im).astype(np.float32) / 255.0
    return (a - a.mean()) / (a.std() + 1e-6) * amp


def distressed_text(img, xy, text, font, fill, grunge=0.45, stroke=0):
    """Texto con desgaste (como el logo de ECO)."""
    W, H = img.size
    layer = Image.new("L", (W, H), 0)
    ImageDraw.Draw(layer).text(xy, text, font=font, fill=255, stroke_width=stroke, stroke_fill=255)
    a = np.asarray(layer).astype(np.float32) / 255.0
    # manchas y rayones que se comen la pintura
    g = noise(H, W, 0.6, 1.0) + noise(H, W, 3.0, 0.7)
    wear = np.clip(1.0 - grunge * np.clip(g - 0.6, 0, 3), 0, 1)
    a *= wear
    col = np.zeros((H, W, 3), np.float32)
    col[:] = np.array(fill, np.float32)
    shade = 0.85 + 0.15 * noise(H, W, 2.0, 1.0)
    col *= np.clip(shade, 0.6, 1.2)[..., None]
    base = np.asarray(img).astype(np.float32)
    # sombra
    sh = np.asarray(layer.filter(ImageFilter.GaussianBlur(2))).astype(np.float32) / 255.0
    sh = np.roll(np.roll(sh, 2, axis=0), 1, axis=1)
    base *= (1.0 - 0.55 * sh)[..., None]
    out = base * (1 - a[..., None]) + col * a[..., None]
    return Image.fromarray(np.clip(out, 0, 255).astype(np.uint8))


def make_drawer(W=1650, H=220):
    base = np.zeros((H, W, 3), np.float32)
    base[:] = np.array([33, 35, 36], np.float32)
    base += (noise(H, W, 0, 5.0) + noise(H, W, 6, 4.0) + noise(H, W, 40, 5.0))[..., None]
    scratch = Image.new("L", (W, H), 0)
    d = ImageDraw.Draw(scratch)
    for _ in range(300):
        x, y = rng.uniform(0, W), rng.uniform(0, H)
        ang = rng.normal(0.0, 0.5)
        ln = rng.uniform(8, 90)
        d.line([(x, y), (x + np.cos(ang) * ln, y + np.sin(ang) * ln)], fill=int(rng.uniform(20, 70)), width=1)
    base += (np.asarray(scratch.filter(ImageFilter.GaussianBlur(0.4))).astype(np.float32) / 255.0)[..., None] * 28.0
    recess = Image.new("L", (W, H), 0)
    ImageDraw.Draw(recess).rounded_rectangle((16, 52, W - 17, H - 14), radius=10, fill=255)
    rm = np.asarray(recess).astype(np.float32) / 255.0
    inner = np.zeros((H, W, 3), np.float32)
    inner[:] = np.array([11, 13, 14], np.float32)
    inner += (noise(H, W, 0, 1.8) + noise(H, W, 18, 1.6))[..., None]
    shadow = np.asarray(recess.filter(ImageFilter.GaussianBlur(9))).astype(np.float32) / 255.0
    inner *= (0.45 + 0.55 * np.clip((shadow - 0.35) / 0.65, 0, 1))[..., None]
    base = base * (1 - rm[..., None]) + inner * rm[..., None]
    yy = np.mgrid[0:H, 0:W][0]
    top = np.clip(1.0 - yy / 16.0, 0, 1) ** 1.6
    base *= (1.0 - 0.75 * top)[..., None]
    mask = Image.new("L", (W, H), 0)
    ImageDraw.Draw(mask).rounded_rectangle((2, -30, W - 3, H - 2), radius=16, fill=255)
    m = np.asarray(mask.filter(ImageFilter.GaussianBlur(0.6))).astype(np.float32) / 255.0
    outside = np.zeros((H, W, 3), np.float32)
    outside[:] = np.array([5, 7, 4], np.float32)
    base = base * m[..., None] + outside * (1 - m[..., None])
    return Image.fromarray(np.clip(base, 0, 255).astype(np.uint8))


def make_icon(size=1024):
    s = size
    im = Image.new("RGBA", (s, s), (0, 0, 0, 0))
    plate = Image.open(UI / "plate.jpg").convert("RGB").crop((480, 290, 1100, 910)).resize((s, s), Image.LANCZOS)
    mask = Image.new("L", (s, s), 0)
    ImageDraw.Draw(mask).rounded_rectangle((40, 40, s - 40, s - 40), radius=200, fill=255)
    im.paste(plate, (0, 0), mask)
    d = ImageDraw.Draw(im)
    lime = (173, 243, 90, 255)
    grey = (215, 218, 220, 255)
    # radio (boombox)
    x0, y0, x1, y1 = 190, 360, 834, 760
    d.rounded_rectangle((x0, y0, x1, y1), radius=60, outline=grey, width=22)
    d.arc((330, 200, 694, 520), 200, 340, fill=grey, width=22)
    for cx in (330, 694):
        d.ellipse((cx - 120, 560 - 120, cx + 120, 560 + 120), outline=grey, width=18)
        d.ellipse((cx - 62, 560 - 62, cx + 62, 560 + 62), outline=lime, width=18)
        d.ellipse((cx - 16, 560 - 16, cx + 16, 560 + 16), fill=lime)
    d.rounded_rectangle((440, 420, 584, 500), radius=12, outline=lime, width=12)
    return im


def make_installer_background(scale=1, dark=True):
    """Fondo del instalador de macOS: la grabadora en líneas y el nombre abajo a la izquierda."""
    W, H = 620 * scale, 418 * scale
    im = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    line = (215, 218, 220, 255) if dark else (40, 40, 40, 255)
    accent = (173, 243, 90, 255) if dark else (60, 120, 20, 255)
    k = scale
    lw = max(1, int(2.2 * k))
    # grabadora: centrada en x=92, cuerpo de 112x70
    cx, top = 92 * k, 258 * k
    bx0, by0, bx1, by1 = cx - 58 * k, top + 18 * k, cx + 58 * k, top + 88 * k
    d.rounded_rectangle((bx0, by0, bx1, by1), radius=10 * k, outline=line, width=lw)
    d.rounded_rectangle((cx - 30 * k, top + 4 * k, cx + 30 * k, by0), radius=4 * k, outline=line, width=lw)
    d.line((cx + 36 * k, by0, cx + 52 * k, top - 10 * k), fill=line, width=lw)
    d.ellipse((cx + 50 * k, top - 13 * k, cx + 55 * k, top - 8 * k), fill=line)
    for sx in (cx - 30 * k, cx + 30 * k):
        sy = top + 56 * k
        d.ellipse((sx - 20 * k, sy - 20 * k, sx + 20 * k, sy + 20 * k), outline=line, width=lw)
        d.ellipse((sx - 11 * k, sy - 11 * k, sx + 11 * k, sy + 11 * k), outline=accent, width=lw)
        d.ellipse((sx - 3 * k, sy - 3 * k, sx + 3 * k, sy + 3 * k), fill=accent)
    d.rounded_rectangle((cx - 9 * k, top + 26 * k, cx + 9 * k, top + 38 * k), radius=2 * k, outline=accent, width=lw)
    # nombre
    big = ImageFont.truetype(str(FONTS / "Barlow-SemiBold.ttf"), 30 * k)
    small = ImageFont.truetype(str(FONTS / "Barlow-Medium.ttf"), 11 * k)

    def spaced(y, text, font, fill, gap):
        widths = [font.getlength(ch) for ch in text]
        total = sum(widths) + gap * (len(text) - 1)
        x = cx - total / 2
        for ch, w in zip(text, widths):
            d.text((x, y), ch, font=font, fill=fill)
            x += w + gap

    spaced(354 * k, "DEMBOW", big, accent, 3 * k)
    spaced(390 * k, "CLÁSICO", small, line, 4 * k)
    return im


def main():
    img = Image.open(ROOT / "tools" / "eco_plate.jpg").convert("RGB")

    # --- borrar el logo "ECO" y los LEDs (se cubren con textura limpia del mismo encabezado)
    patch(img, (322, 34, 566, 104), (68, 34), blur=3, mask_radius=4)
    # --- borrar el emblema: sus líneas son delgadas, así que se "rellenan" con lo que las rodea
    import cv2
    a = cv2.cvtColor(np.asarray(img), cv2.COLOR_RGB2BGR)
    x0, y0, x1, y1 = 712, 322, 920, 486
    roi = a[y0:y1, x0:x1].astype(np.float32)
    lum = roi.mean(axis=2)
    local = cv2.GaussianBlur(lum, (0, 0), 6)
    green = (roi[..., 1] > roi[..., 2] + 12) & (roi[..., 1] > roi[..., 0] + 12)
    m = ((lum > local + 14) | green).astype(np.uint8) * 255
    m = cv2.dilate(m, np.ones((5, 5), np.uint8), iterations=1)
    full = np.zeros(a.shape[:2], np.uint8)
    full[y0:y1, x0:x1] = m
    a = cv2.inpaint(a, full, 6, cv2.INPAINT_TELEA)
    img = Image.fromarray(cv2.cvtColor(a, cv2.COLOR_BGR2RGB))

    # el relleno queda liso y oscuro: se cubre con metal rayado del mismo panel (espejado para que no se repita)
    clean = img.copy()

    def texture(src, dst, mirror, blur=12, inset_y=4):
        piece = clean.crop(src)
        if mirror:
            piece = piece.transpose(Image.FLIP_LEFT_RIGHT)
        w, h = piece.size
        m = Image.new("L", (w, h), 0)
        ImageDraw.Draw(m).rectangle((blur * 2, inset_y, w - blur * 2, h - inset_y), fill=255)
        img.paste(piece, dst, m.filter(ImageFilter.GaussianBlur(blur)))

    panel = (530, 298, 705, 480)
    texture(panel, (640, 298), True)
    texture(panel, (770, 298), False)
    texture(panel, (890, 298), True)
    edge = (530, 474, 705, 500)          # borde entre el panel y la barra (tenía una mancha)
    texture(edge, (660, 474), True, blur=10, inset_y=0)
    texture(edge, (800, 474), False, blur=10, inset_y=0)
    texture((560, 498, 650, 530), (650, 498), True, blur=8, inset_y=0)

    # --- logo nuevo
    big = ImageFont.truetype(str(FONTS / "Barlow-SemiBold.ttf"), 50)
    small = ImageFont.truetype(str(FONTS / "Barlow-SemiBold.ttf"), 25)
    img = distressed_text(img, (92, 38), "DEMBOW", big, (205, 204, 198), grunge=0.28, stroke=2)
    # CLÁSICO en verde, con letras separadas
    x = 338
    for ch in "CLÁSICO":
        img = distressed_text(img, (x, 56), ch, small, (150, 230, 70), grunge=0.25, stroke=0)
        x += small.getlength(ch) + 5

    img.save(UI / "plate.jpg", quality=92, optimize=True)
    make_drawer().save(UI / "drawer_bg.jpg", quality=90, optimize=True)
    icon_dir = ROOT / "Resources" / "icon"
    icon_dir.mkdir(parents=True, exist_ok=True)
    make_icon().save(icon_dir / "icon.png", optimize=True)
    assets = ROOT / "installer" / "assets"
    for scale, suffix in ((1, ""), (2, "@2x")):
        make_installer_background(scale, dark=False).save(assets / f"background{suffix}.png", optimize=True)
        make_installer_background(scale, dark=True).save(assets / f"background-dark{suffix}.png", optimize=True)
    print("listo:", UI / "plate.jpg", UI / "drawer_bg.jpg", icon_dir / "icon.png")


if __name__ == "__main__":
    main()
