"""Draws the countdown's seasonal motifs: flat pictures of 600 x 400 pixels, one per kind of day.

Each scene is drawn on a stage of 1200 x 1200 that sits right of the middle of a wider picture, so its subject stays
in view where the tile lays the number over the left part.

The tile shows them through the Tessera app, which fetches them from this repository's main branch
(raw.githubusercontent.com). Run it again after a change: python3 motifs/draw.py (needs Pillow).
"""
import math
import os
import random

from PIL import Image, ImageDraw, ImageFilter

S = 1200          # the stage, drawn at three times the size, then scaled down: smooth edges
W, H = 1800, 1200  # the whole picture
X0 = 450           # where the stage starts in it
OUT = (600, 400)
HERE = os.path.dirname(os.path.abspath(__file__))


class Stage:
    """ImageDraw on the stage: x of every shape moves by X0. A shape that reaches past both edges of the stage (a
    ground, a strip of sea) is stretched to the whole width of the picture."""

    def __init__(self, img):
        self.d = ImageDraw.Draw(img)

    @staticmethod
    def _box(box):
        x0, y0, x1, y1 = box
        if x0 <= 0 and x1 >= S:
            return [-1, y0, W + 1, y1]
        return [x0 + X0, y0, x1 + X0, y1]

    @staticmethod
    def _pts(pts):
        return [(x + X0, y) for x, y in pts]

    def ellipse(self, box, **kw):
        self.d.ellipse(self._box(box), **kw)

    def rectangle(self, box, **kw):
        self.d.rectangle(self._box(box), **kw)

    def rounded_rectangle(self, box, r, **kw):
        self.d.rounded_rectangle(self._box(box), r, **kw)

    def arc(self, box, a, b, **kw):
        self.d.arc(self._box(box), a, b, **kw)

    def pieslice(self, box, a, b, **kw):
        self.d.pieslice(self._box(box), a, b, **kw)

    def polygon(self, pts, **kw):
        self.d.polygon(self._pts(pts), **kw)

    def line(self, pts, **kw):
        self.d.line(self._pts(pts), **kw)


def stage(img):
    return Stage(img)


def gradient(top, bottom):
    img = Image.new('RGB', (W, H))
    d = ImageDraw.Draw(img)
    for y in range(H):
        t = y / (H - 1)
        d.line([(0, y), (W, y)], fill=tuple(int(a + (b - a) * t) for a, b in zip(top, bottom)))
    return img


def glow(img, xy, r, color, strength=0.55):
    layer = Image.new('RGBA', (W, H), (0, 0, 0, 0))
    x = xy[0] + X0
    ImageDraw.Draw(layer).ellipse([x - r, xy[1] - r, x + r, xy[1] + r], fill=color + (int(255 * strength),))
    layer = layer.filter(ImageFilter.GaussianBlur(r / 2))
    img.paste(layer, (0, 0), layer)


def circle(d, x, y, r, fill, outline=None, width=0):
    d.ellipse([x - r, y - r, x + r, y + r], fill=fill, outline=outline, width=width)


def star(d, x, y, r, fill, points=5, inner=0.45, turn=-90):
    pts = []
    for i in range(points * 2):
        rr = r if i % 2 == 0 else r * inner
        a = math.radians(turn + i * 180 / points)
        pts.append((x + rr * math.cos(a), y + rr * math.sin(a)))
    d.polygon(pts, fill=fill)


def snow(d, rnd, n, color=(255, 255, 255), rmin=4, rmax=12, top=0, bottom=S):
    for _ in range(n):
        circle(d, rnd.uniform(-X0, W - X0), rnd.uniform(top, bottom), rnd.uniform(rmin, rmax), color)


def save(img, name):
    img.resize(OUT, Image.LANCZOS).convert('RGB').save(os.path.join(HERE, name + '.png'), optimize=True)


def christmas():
    rnd = random.Random(1)
    img = gradient((18, 32, 72), (40, 70, 130))
    glow(img, (600, 330), 330, (120, 160, 255), 0.25)
    d = stage(img)
    snow(d, rnd, 70, (230, 238, 255), 3, 9, 0, 1000)
    d.rectangle([0, 1010, S, S], fill=(236, 242, 252))
    d.ellipse([-200, 950, 700, 1150], fill=(236, 242, 252))
    d.ellipse([500, 970, 1400, 1160], fill=(226, 234, 248))
    d.rectangle([555, 900, 645, 1030], fill=(110, 70, 40))
    greens = [(26, 120, 70), (32, 140, 80), (40, 158, 92)]
    for i, (top, bottom, half) in enumerate([(240, 560, 230), (400, 760, 320), (580, 940, 410)]):
        d.polygon([(600, top), (600 - half, bottom), (600 + half, bottom)], fill=greens[i])
    d.line([(420, 560), (760, 470)], fill=(250, 214, 90), width=14)
    d.line([(330, 760), (840, 640)], fill=(250, 214, 90), width=14)
    for x, y, c in [(520, 500, (230, 60, 70)), (690, 540, (250, 200, 60)), (430, 700, (90, 170, 255)),
                    (640, 690, (230, 60, 70)), (770, 740, (250, 250, 250)), (360, 880, (250, 200, 60)),
                    (560, 860, (90, 170, 255)), (820, 900, (230, 60, 70)), (700, 870, (250, 250, 250))]:
        circle(d, x, y, 30, c)
        circle(d, x - 9, y - 9, 8, (255, 255, 255))
    glow(img, (600, 225), 110, (255, 220, 120), 0.6)
    d = stage(img)
    star(d, 600, 230, 85, (255, 214, 80))
    for x, w, h, c, r in [(240, 180, 140, (220, 60, 70), (250, 214, 90)), (880, 160, 120, (60, 150, 230), (255, 255, 255))]:
        d.rectangle([x, 1060 - h, x + w, 1060], fill=c)
        d.rectangle([x + w / 2 - 12, 1060 - h, x + w / 2 + 12, 1060], fill=r)
        d.rectangle([x, 1060 - h * 0.6, x + w, 1060 - h * 0.6 + 22], fill=r)
    save(img, 'christmas')


def advent():
    rnd = random.Random(2)
    img = gradient((52, 22, 30), (96, 40, 44))
    d = stage(img)
    snow(d, rnd, 40, (255, 230, 220), 2, 6)
    d.ellipse([150, 700, 1050, 1080], fill=(30, 100, 60))
    d.ellipse([250, 760, 950, 1000], fill=(40, 125, 74))
    for i in range(26):
        a = i / 26 * 2 * math.pi
        circle(d, 600 + 420 * math.cos(a), 890 + 160 * math.sin(a), 46, (28 + (i % 3) * 10, 110 + (i % 2) * 20, 66))
    for x in (260, 470, 730, 940):
        d.rounded_rectangle([x - 52, 500 if x in (470, 730) else 560, x + 52, 900], 14, fill=(200, 40, 50))
    for x, y in ((260, 560), (470, 500), (730, 500), (940, 560)):
        d.line([(x, y), (x, y - 40)], fill=(40, 30, 30), width=8)
        glow(img, (x, y - 95), 90, (255, 190, 80), 0.55)
        d = stage(img)
        d.ellipse([x - 26, y - 150, x + 26, y - 40], fill=(255, 200, 70))
        d.ellipse([x - 12, y - 115, x + 12, y - 55], fill=(255, 245, 200))
    for x, y in ((380, 980), (600, 1050), (820, 980), (180, 860), (1020, 860)):
        circle(d, x, y, 34, (230, 60, 70))
        circle(d, x - 10, y - 10, 9, (255, 220, 220))
    save(img, 'advent')


def new_year():
    rnd = random.Random(3)
    img = gradient((10, 12, 40), (40, 24, 80))
    d = stage(img)
    snow(d, rnd, 90, (200, 200, 255), 2, 5, 0, 900)
    for cx, cy, r, col in [(330, 360, 240, (255, 90, 120)), (820, 300, 280, (255, 210, 80)),
                           (640, 640, 200, (100, 220, 255)), (250, 760, 150, (180, 120, 255))]:
        glow(img, (cx, cy), r * 0.7, col, 0.35)
        d = stage(img)
        for i in range(24):
            a = i / 24 * 2 * math.pi
            x1, y1 = cx + r * 0.25 * math.cos(a), cy + r * 0.25 * math.sin(a)
            x2, y2 = cx + r * math.cos(a), cy + r * math.sin(a)
            d.line([(x1, y1), (x2, y2)], fill=col, width=10)
            circle(d, x2, y2, 14, (255, 255, 255))
    d.rectangle([0, 1020, S, S], fill=(14, 14, 30))
    x = -X0
    while x < W - X0:
        w, h = rnd.randint(70, 150), rnd.randint(80, 260)
        d.rectangle([x, 1030 - h, x + w, 1030], fill=(20, 20, 44))
        for wy in range(int(1030 - h + 20), 1010, 46):
            for wx in range(x + 16, x + w - 20, 36):
                if rnd.random() < 0.4:
                    d.rectangle([wx, wy, wx + 14, wy + 20], fill=(255, 210, 120))
        x += w + 6
    save(img, 'new_year')


def egg(d, x, y, w, h, base, stripe, dots):
    d.ellipse([x - w, y - h, x + w, y + h * 0.8], fill=base)
    d.rectangle([x - w * 0.95, y - 10, x + w * 0.95, y + 22], fill=stripe)
    for dx in (-0.5, 0, 0.5):
        circle(d, x + dx * w, y - h * 0.45, 14, dots)


def easter():
    rnd = random.Random(4)
    img = gradient((180, 225, 255), (230, 245, 255))
    d = stage(img)
    circle(d, 980, 220, 120, (255, 230, 120))
    d.ellipse([-300, 760, 1500, 1500], fill=(120, 200, 110))
    d.ellipse([-200, 840, 900, 1400], fill=(100, 185, 95))
    for _ in range(160):
        x, y = rnd.uniform(-X0, W - X0), rnd.uniform(800, S)
        d.line([(x, y), (x + rnd.uniform(-12, 12), y - rnd.uniform(30, 60))], fill=(80, 165, 80), width=7)
    egg(d, 380, 820, 130, 170, (255, 140, 170), (255, 220, 90), (255, 255, 255))
    egg(d, 700, 760, 150, 195, (130, 190, 255), (255, 255, 255), (255, 140, 170))
    egg(d, 960, 880, 110, 145, (255, 215, 90), (150, 210, 120), (130, 190, 255))
    for x, y in ((180, 1000), (1080, 1060), (560, 1080)):
        for i in range(5):
            a = i / 5 * 2 * math.pi
            circle(d, x + 26 * math.cos(a), y + 26 * math.sin(a), 20, (255, 255, 255))
        circle(d, x, y, 16, (255, 210, 80))
    save(img, 'easter')


def tulip(d, x, y, color, h):
    d.line([(x, y), (x, y - h)], fill=(70, 150, 70), width=16)
    d.ellipse([x - 70, y - h * 0.5, x - 5, y - h * 0.5 + 120], fill=(80, 170, 80))
    d.polygon([(x - 60, y - h), (x + 60, y - h), (x + 52, y - h - 90), (x + 20, y - h - 50), (x, y - h - 110),
               (x - 20, y - h - 50), (x - 52, y - h - 90)], fill=color)
    d.ellipse([x - 62, y - h - 60, x + 62, y - h + 40], fill=color)


def spring():
    img = gradient((170, 220, 255), (225, 245, 255))
    d = stage(img)
    for cx, cy in ((250, 220), (820, 320)):
        for dx, dy, r in ((0, 0, 70), (70, -20, 90), (150, 0, 70), (75, 30, 70)):
            circle(d, cx + dx, cy + dy, r, (255, 255, 255))
    d.ellipse([-300, 880, 1500, 1500], fill=(140, 210, 120))
    for x, c, h in ((200, (255, 90, 110), 380), (400, (255, 200, 60), 470), (600, (210, 110, 230), 420),
                    (800, (255, 140, 60), 500), (1000, (255, 90, 150), 400)):
        tulip(d, x, 1060, c, h)
    save(img, 'spring')


def summer():
    img = gradient((80, 180, 250), (190, 235, 255))
    glow(img, (880, 280), 260, (255, 230, 120), 0.6)
    d = stage(img)
    circle(d, 880, 280, 140, (255, 214, 70))
    d.rectangle([0, 700, S, 900], fill=(40, 140, 210))
    for y in range(720, 900, 40):
        for x in range(-X0, W - X0, 160):
            d.arc([x, y, x + 120, y + 30], 200, 340, fill=(220, 245, 255), width=6)
    d.ellipse([-200, 840, 1400, 1400], fill=(250, 220, 150))
    d.line([(330, 1000), (240, 520)], fill=(150, 100, 60), width=18)
    d.pieslice([40, 380, 520, 700], 180, 360, fill=(240, 80, 90))
    for a in (200, 240, 280, 320):
        r = math.radians(a)
        d.line([(280, 540), (280 + 240 * math.cos(r), 540 + 160 * math.sin(r))], fill=(255, 255, 255), width=10)
    d.rounded_rectangle([600, 960, 960, 1010], 20, fill=(70, 170, 230))
    d.rounded_rectangle([600, 1010, 960, 1040], 12, fill=(255, 255, 255))
    circle(d, 1050, 1010, 50, (255, 120, 80))
    circle(d, 1050, 1010, 50, None, (255, 255, 255), 10)
    save(img, 'summer')


def leaf(d, x, y, r, a, color):
    pts = []
    for i in range(40):
        t = i / 39 * 2 * math.pi
        px, py = r * math.cos(t), r * 0.55 * math.sin(t) * (1 if math.cos(t) < 0.9 else 0.4)
        pts.append((x + px * math.cos(a) - py * math.sin(a), y + px * math.sin(a) + py * math.cos(a)))
    d.polygon(pts, fill=color)


def autumn():
    rnd = random.Random(5)
    img = gradient((255, 190, 120), (240, 130, 80))
    d = stage(img)
    d.ellipse([-300, 900, 1500, 1500], fill=(170, 90, 50))
    for _ in range(26):
        leaf(d, rnd.uniform(-X0, W - X0), rnd.uniform(0, 900), rnd.uniform(40, 70), rnd.uniform(0, math.pi),
             rnd.choice([(230, 90, 40), (250, 170, 40), (200, 60, 40), (240, 130, 30)]))
    for cx, cy, w, h in ((430, 900, 260, 200), (820, 960, 180, 140)):
        for dx, ww in ((-0.55, 0.5), (0.55, 0.5), (0, 0.6)):
            d.ellipse([cx + dx * w - ww * w, cy - h, cx + dx * w + ww * w, cy + h], fill=(240, 120, 30))
        for dx in (-0.55, 0, 0.55):
            d.line([(cx + dx * w * 0.8, cy - h * 0.85), (cx + dx * w * 0.8, cy + h * 0.85)], fill=(215, 95, 25), width=8)
        d.rounded_rectangle([cx - 18, cy - h - 60, cx + 18, cy - h + 10], 10, fill=(90, 120, 50))
    save(img, 'autumn')


def halloween():
    rnd = random.Random(6)
    img = gradient((40, 20, 70), (120, 50, 70))
    d = stage(img)
    circle(d, 880, 260, 150, (255, 235, 180))
    circle(d, 950, 220, 140, None)
    snow(d, rnd, 50, (255, 240, 200), 2, 5, 0, 700)
    for bx, by in ((300, 260), (420, 180), (560, 300)):
        d.polygon([(bx, by), (bx - 70, by - 30), (bx - 40, by), (bx - 70, by + 10), (bx, by + 10),
                   (bx + 70, by + 10), (bx + 40, by), (bx + 70, by - 30)], fill=(30, 10, 40))
    d.ellipse([-300, 960, 1500, 1500], fill=(30, 20, 40))
    cx, cy, w, h = 600, 840, 330, 250
    for dx, ww in ((-0.55, 0.5), (0.55, 0.5), (0, 0.6)):
        d.ellipse([cx + dx * w - ww * w, cy - h, cx + dx * w + ww * w, cy + h], fill=(245, 130, 30))
    d.rounded_rectangle([cx - 22, cy - h - 70, cx + 22, cy - h + 10], 10, fill=(90, 120, 50))
    glow_c = (255, 230, 90)
    d.polygon([(470, 760), (560, 760), (515, 680)], fill=glow_c)
    d.polygon([(640, 760), (730, 760), (685, 680)], fill=glow_c)
    d.polygon([(420, 880), (780, 880), (740, 960), (680, 920), (600, 980), (520, 920), (460, 960)], fill=glow_c)
    save(img, 'halloween')


def winter():
    rnd = random.Random(7)
    img = gradient((120, 170, 230), (210, 230, 250))
    d = stage(img)
    snow(d, rnd, 120, (255, 255, 255), 4, 12)
    d.ellipse([-300, 860, 1500, 1500], fill=(245, 250, 255))
    circle(d, 600, 880, 210, (255, 255, 255), (200, 215, 235), 8)
    circle(d, 600, 600, 150, (255, 255, 255), (200, 215, 235), 8)
    circle(d, 600, 390, 110, (255, 255, 255), (200, 215, 235), 8)
    d.rectangle([520, 220, 680, 300], fill=(40, 40, 60))
    d.rectangle([480, 290, 720, 320], fill=(40, 40, 60))
    circle(d, 565, 375, 14, (40, 40, 60))
    circle(d, 635, 375, 14, (40, 40, 60))
    d.polygon([(600, 400), (600, 430), (720, 420)], fill=(255, 140, 40))
    d.rounded_rectangle([470, 470, 730, 520], 20, fill=(220, 50, 60))
    d.rounded_rectangle([640, 480, 690, 640], 20, fill=(220, 50, 60))
    for y in (560, 630, 700):
        circle(d, 600, y, 14, (40, 40, 60))
    d.line([(460, 590), (300, 470)], fill=(110, 70, 40), width=14)
    d.line([(740, 590), (900, 470)], fill=(110, 70, 40), width=14)
    save(img, 'winter')


def birthday():
    rnd = random.Random(8)
    img = gradient((255, 220, 235), (255, 245, 250))
    d = stage(img)
    for _ in range(80):
        x, y = rnd.uniform(-X0, W - X0), rnd.uniform(0, 600)
        c = rnd.choice([(255, 90, 120), (90, 170, 255), (255, 200, 60), (130, 210, 120), (190, 120, 255)])
        d.rectangle([x, y, x + 22, y + 12], fill=c)
    for bx, by, c in ((200, 300, (255, 90, 120)), (1000, 260, (90, 170, 255)), (1060, 520, (255, 200, 60))):
        d.line([(bx, by + 110), (bx + 20, by + 380)], fill=(150, 150, 150), width=4)
        d.ellipse([bx - 85, by - 110, bx + 85, by + 110], fill=c)
        circle(d, bx - 30, by - 45, 18, (255, 255, 255))
    d.rounded_rectangle([250, 860, 950, 1080], 30, fill=(250, 200, 220))
    d.rounded_rectangle([330, 660, 870, 880], 30, fill=(255, 230, 240))
    for x in range(330, 870, 90):
        d.ellipse([x, 840, x + 90, 920], fill=(255, 230, 240))
    for x in range(250, 950, 100):
        d.ellipse([x, 1040, x + 100, 1110], fill=(250, 200, 220))
    d.rectangle([150, 1080, 1050, 1110], fill=(255, 255, 255))
    for x, c in ((450, (90, 170, 255)), (600, (255, 90, 120)), (750, (255, 200, 60))):
        d.rounded_rectangle([x - 16, 520, x + 16, 670], 8, fill=c)
        glow(img, (x, 480), 60, (255, 200, 80), 0.6)
        d = stage(img)
        d.ellipse([x - 20, 430, x + 20, 515], fill=(255, 190, 60))
        d.ellipse([x - 8, 460, x + 8, 505], fill=(255, 245, 200))
    save(img, 'birthday')


def travel():
    img = gradient((90, 170, 240), (200, 230, 255))
    d = stage(img)
    for cx, cy in ((200, 300), (780, 220), (900, 640)):
        for dx, dy, r in ((0, 0, 70), (70, -20, 90), (150, 0, 70), (75, 30, 70)):
            circle(d, cx + dx, cy + dy, r, (255, 255, 255))
    d.ellipse([-300, 920, 1500, 1500], fill=(120, 190, 110))
    d.rounded_rectangle([340, 700, 860, 1080], 50, fill=(230, 90, 70))
    d.rounded_rectangle([500, 620, 700, 720], 30, fill=None, outline=(80, 60, 60), width=26)
    for x in (440, 760):
        d.rectangle([x - 14, 700, x + 14, 1080], fill=(200, 70, 55))
    d.rounded_rectangle([380, 840, 820, 880], 14, fill=(250, 210, 90))
    for x in (420, 780):
        circle(d, x, 1100, 30, (60, 50, 50))
    d.polygon([(250, 470), (720, 430), (760, 455), (720, 480), (250, 490)], fill=(255, 255, 255))
    d.polygon([(470, 455), (380, 330), (430, 330), (580, 450)], fill=(240, 240, 250))
    d.polygon([(480, 470), (390, 600), (440, 600), (590, 475)], fill=(240, 240, 250))
    d.polygon([(250, 470), (215, 390), (255, 390), (300, 470)], fill=(240, 240, 250))
    save(img, 'travel')


if __name__ == '__main__':
    for draw in (christmas, advent, new_year, easter, spring, summer, autumn, halloween, winter, birthday, travel):
        draw()
    print('motifs drawn in', HERE)
