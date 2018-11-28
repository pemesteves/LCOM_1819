unsigned get_h_res();
unsigned get_v_res();
unsigned get_redScreenMask();
unsigned get_greenScreenMask();
unsigned get_blueScreenMask();
unsigned get_numBytes();
unsigned get_bits_per_pixel();
int vbeGetModeInfo(uint16_t mode, vbe_mode_info_t *vmi_p);
int drawPixel(uint16_t x, uint16_t y, uint32_t color);
char* get_video_mem();

//Video Card MACROS
#define VBE_CALL 0x4F00
#define VBE_MODE_INFO 0x01 //return VBE mode info
#define VBE_SET_MODE 0x02 //set VBE mode
#define VC_INT 0x10
#define INDEXED_MODE 0x105
