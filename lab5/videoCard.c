#include <lcom/lcf.h>
#include <machine/int86.h>

#include "i8254.h"
#include "videoCard.h"
#include <math.h>

//vbe_mode_info_t vmi_p;
static unsigned h_res;
static unsigned v_res;
static unsigned bits_per_pixel;
static unsigned numBytes;
static unsigned redScreenMask;
static unsigned greenScreenMask;
static unsigned blueScreenMask;
static char *video_mem;

void *(vg_init)(uint16_t mode) {
  vbe_mode_info_t vmi_p;

  if (vbeGetModeInfo(mode, &vmi_p))
    return NULL;

  h_res = vmi_p.XResolution;
  v_res = vmi_p.YResolution;
  bits_per_pixel = vmi_p.BitsPerPixel;
  numBytes = ceil(bits_per_pixel / 8.0);
  redScreenMask = vmi_p.RedMaskSize;
  greenScreenMask = vmi_p.GreenMaskSize;
  blueScreenMask = vmi_p.BlueMaskSize;

  int r;
  struct minix_mem_range mr;                                           /*physical memory range*/
  unsigned int vram_base = vmi_p.PhysBasePtr;                          /*VRAM’s physical addresss*/
  unsigned int vram_size = h_res * v_res * ceil(bits_per_pixel / 8.0); /*VRAM’s size,but you can use the frame - buffer size, instead*/

  mr.mr_base = (phys_bytes) vram_base;
  mr.mr_limit = mr.mr_base + vram_size;
  if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
    panic("sys_privctl (ADD_MEM) failed: %d\n", r);

  /*Map memory*/
  video_mem = vm_map_phys(SELF, (void *) mr.mr_base, vram_size);

  if (video_mem == MAP_FAILED)
    panic("couldn’t map video memory");

  struct reg86u reg86;
  memset(&reg86, 0, sizeof(reg86));
  reg86.u.w.ax = VBE_CALL | VBE_SET_MODE; // VBE call, function 02 -- set VBE mode
  reg86.u.w.bx = BIT(14) | mode;          // set bit 14: linear framebuffer
  reg86.u.b.intno = VC_INT;

  if (sys_int86(&reg86) != OK) {
    printf("set_vbe_mode: sys_int86() failed \n", mode);
    return NULL;
  }

  return video_mem;
}

int(vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  while (len > 0) {
    drawPixel(x, y, color);
    x++;
    len--;
  }

  return 0;
}

int(vg_draw_rectangle)(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color) {
  while (height > 0) {

    if (vg_draw_hline(x, y, width, color))
      return 1;
    y++;
    height--;
  }
  return 0;
}

int drawPixel(uint16_t x, uint16_t y, uint32_t color) {
  if (x >= h_res || y >= v_res)
    return 0;

  char *tmp_mem = video_mem;
  tmp_mem += (y * h_res + x) * numBytes;
  uint8_t tmp_color = 0;
  for (uint8_t i = 0; i < numBytes; i++) {
    tmp_color = color >> 8 * i;
    *tmp_mem = tmp_color;
    tmp_mem++;
  }

  return 0;
}

unsigned get_h_res() {
  return h_res;
}

unsigned get_v_res() {
  return v_res;
}

unsigned get_redScreenMask() {
  return redScreenMask;
}

unsigned get_greenScreenMask() {
  return greenScreenMask;
}

unsigned get_blueScreenMask() {
  return blueScreenMask;
}

unsigned get_numBytes() {
  return numBytes;
}

unsigned get_bits_per_pixel() {
  return bits_per_pixel;
}

char *get_video_mem() {
  return video_mem;
}

int vbeGetModeInfo(uint16_t mode, vbe_mode_info_t *vmi_p) {
  if (lm_init(false) == NULL)
    return 1;
  phys_bytes buf;
  struct reg86u r;
  memset(&r, 0, sizeof(r));
  mmap_t m;
  lm_alloc(sizeof(vbe_mode_info_t), &m); /*use liblm.a to initialize buf*/
  buf = m.phys;
  r.u.w.ax = VBE_CALL | VBE_MODE_INFO;
  /*VBE get mode info*/
  /*translate the buffer linear address to a far pointer*/
  r.u.w.es = PB2BASE(buf);
  /*set a segment base*/
  r.u.w.di = PB2OFF(buf);
  /*set the offset accordingly*/
  r.u.w.cx = mode;
  r.u.b.intno = VC_INT;
  if (sys_int86(&r) != OK) {
    return 1; /*call BIOS*/
  }

  *vmi_p = *(vbe_mode_info_t *) (m.virt);
  lm_free(&m);
  return 0;
}

VbeInfoBlock *getControllerInfo() {

  VbeInfoBlock *vInfo = NULL;
  vInfo->VbeSignature[0] = 'V';
  vInfo->VbeSignature[1] = 'B';
  vInfo->VbeSignature[2] = 'E';
  vInfo->VbeSignature[3] = '2';

  phys_bytes buf;
  struct reg86u r;
  memset(&r, 0, sizeof(r));
  mmap_t m;
  lm_alloc(sizeof(VbeInfoBlock), &m); /*use liblm.a to initialize buf*/
  buf = m.phys;
  r.u.w.ax = VBE_CALL | VBE_CONTROL_INFO;
  /*VBE get mode info*/
  /*translate the buffer linear address to a far pointer*/
  r.u.w.es = PB2BASE(buf);
  /*set a segment base*/
  r.u.w.di = PB2OFF(buf);
  /*set the offset accordingly*/
  r.u.b.intno = VC_INT;
  if (sys_int86(&r) != OK) {
    return NULL; /*call BIOS*/
  }

  vInfo = (VbeInfoBlock *) (m.virt);
  lm_free(&m);
  return vInfo;
}
