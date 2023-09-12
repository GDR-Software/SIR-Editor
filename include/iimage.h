//
// Plugin interface for loading image files
//

#ifndef _IIMAGE_H_
#define _IIMAGE_H_

#define IMAGE_MAJOR "image"

// Load an image file
typedef void ( *PFN_ERPLUG_LOADIMAGE )( const char *name, byte **pic, uint32_t *width, uint32_t *height );

struct _ERPlugImageTable
{
	uint64_t m_nSize;
	PFN_ERPLUG_LOADIMAGE m_pfnLoadImage;
};

#endif // _IIMAGE_H_
