#ifndef VIDEOCARD_H
#define VIDEOCARD_H

/** @defgroup VideoCard VideoCard
 * @{
 * Functions to manipulate the video card
 */

 /**
  * @brief Function to get the horizontal resolution of the screen
  * @return Returns the horizontal resolution
  */
unsigned get_h_res();

/**
 * @brief Function to get the vertical resolution of the screen
 * @return Returns the vertical resolution
 */
unsigned get_v_res();

/**
 * @brief Function to get the number of bytes of each pixel
 * @return Returns the number of bytes of each pixel
 */
unsigned get_numBytes();

/**
 * @brief Function to get the number of bits of each pixel
 * @return Returns the number of bits of each pixel
 */
unsigned get_bits_per_pixel();

/**
 * @brief Function to get the information of a vbe mode
 * @param mode VBE mode
 * @param vmi_p Struct that will have the information of the mode
 * @return Return 0 upon success and non-zero otherwise
 */
int vbeGetModeInfo(uint16_t mode, vbe_mode_info_t *vmi_p);

/**
 * @brief Draw a pixel with a specified color
 * @param x, y Pixel coordinates
 * @param color Color of the pixel
 * @return Return 0 upon success and non-zero otherwise
 */
int drawPixel(uint16_t x, uint16_t y, uint32_t color);

/**
 * @brief Function to get the video memory buffer
 * @return Returns a pointer to the video memory buffer
 */
char *get_video_mem();

/**
 * @brief Function to get the second buffer
 * @return Returns a pointer to the second buffer
 */
char *get_second_buffer();

/**
 * @brief Free the two buffers
 */
void freeBuffers();

/**
 * @brief Copy the content of the second buffer to the video memory
 */
void copyToVRAM();

//Video Card MACROS

#define VBE_CALL 0x4F00       /**< @brief VBE call */
#define VBE_MODE_INFO 0x01    /**< @brief VBE function that returns the mode info */
#define VBE_SET_MODE 0x02     /**< @brief VBE function that sets the color mode */
#define VC_INT 0x10           /**< @brief Video Card interrupt */
#define INDEXED_MODE 0x105    /**< @brief Value of the indexed mode */

/**
 * @}
 */

#endif /*VIDEOCARD_H*/
