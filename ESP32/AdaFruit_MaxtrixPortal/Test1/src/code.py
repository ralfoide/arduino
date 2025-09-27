
import board
import os
import wifi
import displayio
import terminalio
import vectorio
import time
from digitalio import DigitalInOut, Direction, Pull
from adafruit_matrixportal.matrixportal import MatrixPortal
from adafruit_matrixportal.matrix import Matrix
from adafruit_display_text.label import Label
from adafruit_bitmap_font import bitmap_font

# _mp = None
_button_down = None
_button_up = None

_cwd = ("/" + __file__).rsplit("/", 1)[
    0
]  # the current working directory (where this file is)

_small_font_path = _cwd + "/fonts/Arial-12.bdf"
_medium_font_path = _cwd + "/fonts/Arial-14.bdf"

def _init_wifi():
    # print(f"Connecting to {os.getenv('CIRCUITPY_WIFI_SSID')}")
    wifi.radio.connect(os.getenv("CIRCUITPY_WIFI_SSID"), os.getenv("CIRCUITPY_WIFI_PASSWORD"))
    # print(f"Connected to {os.getenv('CIRCUITPY_WIFI_SSID')}")
    # print(f"My IP address: {wifi.radio.ipv4_address}")
    # pool = socketpool.SocketPool(_wifi.radio)

def _init_buttons():
    global _button_down, _button_up
    _button_down = DigitalInOut(board.BUTTON_DOWN)
    _button_down.switch_to_input(pull=Pull.UP)
    _button_up = DigitalInOut(board.BUTTON_UP)
    _button_up.switch_to_input(pull=Pull.UP)

def _init_display():
    # global _matrix
    # _matrix = Matrix()
    pass

def test1():
    displayio.release_displays()
    _matrix = Matrix(
        width=64,
        height=64,
        bit_depth=3,
        serpentine=True,
        tile_rows=1,
        alt_addr_pins=[
            board.MTX_ADDRA,
            board.MTX_ADDRB,
            board.MTX_ADDRC,
            board.MTX_ADDRD,
            board.MTX_ADDRE
        ],
    )
    display = _matrix.display
    # small_font = bitmap_font.load_font(_small_font_path)
    # medium_font = bitmap_font.load_font(_medium_font_path)
    # glyphs = b"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-,.: "
    # small_font.load_glyphs(glyphs)
    # medium_font.load_glyphs(glyphs)

    # splash = displayio.Group()
    # background = displayio.OnDiskBitmap(open("loading.bmp", "rb"))
    # bg_sprite = displayio.TileGrid(
    #     background,
    #     pixel_shader=getattr(background, 'pixel_shader', displayio.ColorConverter()),
    # )
    # splash.append(bg_sprite)
    # display.root_group = splash

    gT = displayio.Group()
    gN = displayio.Group()
    # root_group.append(self)

    pal_blue = displayio.Palette(1)
    pal_blue[0] = 0x0000FF
    pal_green = displayio.Palette(1)
    pal_green[0] = 0x00FF00
    pal_red = displayio.Palette(1)
    pal_red[0] = 0xFF0000
    pal_green2 = displayio.Palette(1)
    pal_green2[0] = 0x002000
    pal_red2 = displayio.Palette(1)
    pal_red2[0] = 0x200000
    pal_gray = displayio.Palette(1)
    pal_gray[0] = 0x002000

    # circle = vectorio.Circle(
    #     pixel_shader=pal_blue,
    #      radius=12, x=32, y=16)
    # g.append(circle)

    r330T = vectorio.Rectangle(
        pixel_shader=pal_red,
        width=26,
        height=4,
        x=0,
        y=20
    )
    r330N = vectorio.Rectangle(
        pixel_shader=pal_green,
        width=26,
        height=4,
        x=0,
        y=20
    )
    gT.append(r330T)
    gN.append(r330N)

    r321T = vectorio.Rectangle(
        pixel_shader=pal_red,
        width=26,
        height=4,
        x=64-26,
        y=20-12
    )
    r321N = vectorio.Rectangle(
        pixel_shader=pal_red2,
        width=26,
        height=4,
        x=64-26,
        y=20-12
    )
    gT.append(r321T)
    gN.append(r321N)

    r320T = vectorio.Rectangle(
        pixel_shader=pal_green2,
        width=26,
        height=4,
        x=64-26,
        y=20
    )
    r320N = vectorio.Rectangle(
        pixel_shader=pal_green,
        width=26,
        height=4,
        x=64-26,
        y=20
    )
    gT.append(r320T)
    gN.append(r320N)

    pT = vectorio.Polygon(
        pixel_shader=pal_red,
        x=26,
        y=20,
        points=[ 
            (0,0), (0,4), 
            (64-26-26, -12+4), (64-26-26, -12),
            (64-26-26-1, -12), (0,-1) ]
    )
    gT.append(pT)

    rN = vectorio.Rectangle(
        pixel_shader=pal_green,
        x=26,
        y=20,
        width=64-26-26,
        height=4
    )
    gN.append(rN)

    text1 = Label(terminalio.FONT)
    text1.x = 0
    text1.y = 7
    text1.color = 0x808080
    text1.text = "T330"
    gT.append(text1)
    text1 = Label(terminalio.FONT)
    text1.x = 0
    text1.y = 7
    text1.color = 0x808080
    text1.text = "T330"
    gN.append(text1)

    # text2 = Label(terminalio.FONT)
    # text2.x = 32
    # text2.y = 20
    # text2.color = 0xFFFFFF
    # text2.text = "Line 2"
    # g.append(text2)


    thrown=False
    while True:
        if thrown:
            # r330.pixel_shader = pal_red
            # r321.pixel_shader = pal_red
            # r320.pixel_shader = pal_gray
            display.root_group = gT
        else:
            # r330.pixel_shader = pal_green
            # r321.pixel_shader = pal_gray
            # r320.pixel_shader = pal_green
            display.root_group = gN
        display.refresh(minimum_frames_per_second=0)
        time.sleep(0.5)
        if not _button_down.value or not _button_up.value:
            thrown = not thrown


def test2():
    _mp = MatrixPortal()
    _mp.set_background(0x000044)
    _mp.add_text(
        text="Medium Text",
        text_font = _medium_font_path,
        text_position=(4, (_mp.graphics.display.height // 2) - 1),
        text_color=0xEF7F31,
    )

def test3():
    import rgbmatrix
    import framebufferio

    displayio.release_displays()

    base_width = 64
    base_height = 32
    chain_across = 2
    tile_down = 2
    DISPLAY_WIDTH = base_width * chain_across
    DISPLAY_HEIGHT = base_height * tile_down
    matrix = rgbmatrix.RGBMatrix(
        width=DISPLAY_WIDTH,
        height=DISPLAY_HEIGHT,
        bit_depth=1,
        rgb_pins=[
            board.MTX_R1,
            board.MTX_G1,
            board.MTX_B1,
            board.MTX_R2,
            board.MTX_G2,
            board.MTX_B2
        ],
        addr_pins=[
            board.MTX_ADDRA,
            board.MTX_ADDRB,
            board.MTX_ADDRC,
            board.MTX_ADDRD
        ],
        clock_pin=board.MTX_CLK,
        latch_pin=board.MTX_LAT,
        output_enable_pin=board.MTX_OE,
        tile=tile_down, serpentine=True,
        doublebuffer=False
    )
    display = framebufferio.FramebufferDisplay(matrix, auto_refresh=False)
    # display.root_group = None

    # import vectorio
    # group = displayio.Group()

    # palette = displayio.Palette(1)
    # palette[0] = 0x125690

    # circle = vectorio.Circle(pixel_shader=palette, radius=12, x=32, y=16)
    # group.append(circle)

    # rectangle = vectorio.Rectangle(pixel_shader=palette, width=16, height=16, x=0, y=0)
    # group.append(rectangle)

    # points=[(5, 5), (100, 20), (20, 20), (20, 100)]
    # polygon = vectorio.Polygon(pixel_shader=palette, points=points, x=0, y=0)
    # group.append(polygon)

    # display.root_group = group

    import adafruit_display_text.label

    line1 = adafruit_display_text.label.Label(
        terminalio.FONT,
        color=0xff0000,
        text="Line 1")
    line1.x = 0
    line1.y = 8

    g = displayio.Group()
    g.append(line1)
    # g.append(line2)
    display.root_group = g

    while True:
        display.refresh(minimum_frames_per_second=0)


if __name__ == "__main__":
    _init_buttons()
    # _init_wifi()
    _init_display()
    test1()
    # test2()

