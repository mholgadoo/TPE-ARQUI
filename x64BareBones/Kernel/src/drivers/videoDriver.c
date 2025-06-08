#include <videoDriver.h>
#include <font.h>

#define CHAR_COLOR 0xFFFFFF
#define CHAR_START_X 10
#define CHAR_START_Y 10
#define CHAR_SPACING 8

#define STDIN 0

static uint32_t cursor_x = CHAR_START_X;
static uint32_t cursor_y = CHAR_START_Y;
#define CURSOR_COLOR     0xFFFFFF
#define BACKGROUND_COLOR 0x000000
static int      cursor_visible = 1;
static uint32_t prev_cx = CHAR_START_X, prev_cy = CHAR_START_Y;
static void     drawCursorAt(uint32_t x, uint32_t y);


struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;
VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;

static int font_scale = 1; 

uint16_t getScreenWidth() {
    return VBE_mode_info->width;
}

uint16_t getScreenHeight() {
    return VBE_mode_info->height;
}

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y) {
    uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

void putChar(char c, uint32_t x, uint32_t y, uint32_t color) {
    uint8_t *glyph = getFontChar(c);
    if (!glyph) return;

    uint8_t base_width  = 8;
    uint8_t base_height = 16;
    int scale = getFontWidth() / base_width;

    // limpiar fondo de la celda
    
    (BACKGROUND_COLOR,
             x, y,
             base_width * scale,
             base_height * scale);

    // dibujar bits del glyph
    for (uint8_t row = 0; row < base_height; row++) {
        uint8_t rowBits = glyph[row];
        for (uint8_t col = 0; col < base_width; col++) {
            if ((rowBits >> (7 - col)) & 1) {
                for (int dx = 0; dx < scale; dx++) {
                    for (int dy = 0; dy < scale; dy++) {
                        putPixel(color,
                                 x + col * scale + dx,
                                 y + row * scale + dy);
                    }
                }
            }
        }
    }
}

void writeString(const char *str, int len) {
    if (!str || len <= 0) return;

    for (int i = 0; i < len; i++) {
        char c = str[i];
        if (!c) break;

        if (c == '\n') {
            cursor_y += getFontHeight();
            cursor_x = CHAR_START_X;
        } else if (c == '\t') {
            cursor_x += 4 * getFontWidth();
        } else if (c == '\b') {
            if (cursor_x >= CHAR_START_X) {
                cursor_x -= getFontWidth();
                drawRect(BACKGROUND_COLOR,
                         cursor_x, cursor_y,
                         getFontWidth(),
                         getFontHeight());
            }
        } else {
            putChar(c, cursor_x, cursor_y, CHAR_COLOR);
            cursor_x += getFontWidth();
        }
    }

    // borrar solo la línea del cursor anterior
    drawRect(BACKGROUND_COLOR,
             prev_cx,
             prev_cy + getFontHeight() - 2,
             getFontWidth(),
             2);
    // dibujar nueva línea de cursor
    drawRect(CURSOR_COLOR,
             cursor_x,
             cursor_y + getFontHeight() - 2,
             getFontWidth(),
             2);

    prev_cx = cursor_x;
    prev_cy = cursor_y;
}
void print_hex64(uint64_t value) {
    char hex[18];        // "0x" + 16 dígitos + '\0'
    hex[0] = '0';
    hex[1] = 'x';
    for (int i = 0; i < 16; i++) {
        int nibble = (value >> ((15 - i) * 4)) & 0xF;
        hex[2 + i] = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
    }
    // No '\0' necesario, imprime los 18 caracteres
    writeString(hex, 18);
}

void drawRect(uint32_t hexColor, uint64_t x, uint64_t y, uint64_t width, uint64_t height) {
	uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
    uint64_t bytesPerPixel = VBE_mode_info->bpp / 8;
    uint64_t pitch = VBE_mode_info->pitch;

    for (uint64_t i = 0; i < height; i++) {
        for (uint64_t j = 0; j < width; j++) {
            uint64_t offset = (x + j) * bytesPerPixel + (y + i) * pitch;
            framebuffer[offset]     = (hexColor) & 0xFF;
            framebuffer[offset + 1] = (hexColor >> 8) & 0xFF;
            framebuffer[offset + 2] = (hexColor >> 16) & 0xFF;
        }
    }
}

void clearScreen() {
    uint8_t *framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
    uint64_t screenSize = getScreenWidth() * getScreenHeight();
    uint64_t totalBytes = screenSize * 4;  // 4 bytes por pixel

    for (uint64_t i = 0; i < totalBytes; i++) {
        framebuffer[i] = 0; // negro absoluto, BGRA = 0x00 0x00 0x00 0x00
    }
    cursor_x = CHAR_START_X;
    cursor_y = CHAR_START_Y;
}

//funcion para ver el cursor
static void drawCursorAt(uint32_t x, uint32_t y) {
    // dibuja un bloque de una línea (underscore) al pie del glyph
    uint32_t w = getFontWidth();
    uint32_t h = 2;  // grosor de la línea
    drawRect(cursor_visible ? CURSOR_COLOR : BACKGROUND_COLOR,
             x, y + getFontHeight() - h,
             w, h);
    cursor_visible ^= 1;
}