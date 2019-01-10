typedef struct {
  uint8_t VbeSignature[4];      /* VBE signature */
  uint16_t VbeVersion;          /* VBE Version */
  phys_bytes OemStringPtr;      /* Pointer to OEM String */
  uint8_t Capabilities[4];      /* Capabilities of graphics controller */
  phys_bytes VideoModePtr;      /* Pointer to VideoModeList */
  uint16_t TotalMemory;         /* Number of 64kb memory blocks: Added for VBE 2.0 */
  uint16_t OemSoftwareRev;      /* VBE implementation Software revision */
  phys_bytes OemVendorNamePtr;  /* Pointer to Vendor Name String */
  phys_bytes OemProductNamePtr; /* Pointer to Product Name String */
  phys_bytes OemProductRevPtr;  /* Pointer to Product Revision String */
  uint8_t Reserved[222];        /* Reserved for VBE implementation scratch area */
  uint8_t OemData[256];         /* Data Area for OEM Strings */
} __attribute__((packed)) VbeInfoBlock;

unsigned get_h_res();
unsigned get_v_res();
unsigned get_redScreenMask();
unsigned get_greenScreenMask();
unsigned get_blueScreenMask();
unsigned get_numBytes();
unsigned get_bits_per_pixel();
int vbeGetModeInfo(uint16_t mode, vbe_mode_info_t *vmi_p);
int drawPixel(uint16_t x, uint16_t y, uint32_t color);
char *get_video_mem();
VbeInfoBlock* getControllerInfo();

//Video Card MACROS
#define VBE_CALL 0x4F00
#define VBE_CONTROL_INFO 0x00 //returna VBE controller info
#define VBE_MODE_INFO 0x01    //return VBE mode info
#define VBE_SET_MODE 0x02     //set VBE mode
#define VC_INT 0x10
#define INDEXED_MODE 0x105
