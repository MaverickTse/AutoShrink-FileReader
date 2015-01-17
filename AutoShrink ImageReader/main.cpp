#include <Windows.h>
#include "input.h"
#include "FreeImage.h"

// Global variables
typedef struct
{
	BOOL isMultiPage;
	FREE_IMAGE_FORMAT fiformat;
	FIMULTIBITMAP* mbitmap;
	FIBITMAP* sbitmap;
	UINT32 output_w;
	UINT32 output_h;
	INPUT_INFO* iip;
}FIHANDLE;

INPUT_PLUGIN_TABLE input_plugin_table = {
	INPUT_PLUGIN_FLAG_VIDEO,
	"AutoShrink Image Reader",
	"ImageFile\0*.bmp;*.jpg;*.jpeg;*.jp2;*.png;*.hdr;*.psd;*.dng;*.cr2;*.srf;*.sr2;*.pct;*.pcx;*.sgi;*.tga;*.webp;*.tiff;*.tif;*.ico;*.exr;*.mng;*.pbm;*.xbm"
	/*
	"Bitmap(*.bmp)\0*.bmp\0"
	"JPEG(*.jpg;*.jpeg;*.jp2)\0*.jpg;*.jpeg;*.jp2\0"
	"PNG(*.png)\0*.png\0"
	"HDR(*.hdr)\0*.hdr\0"
	"Adobe(*.psd;*.dng)\0*.psd;*.dng\0"
	//	"MacPICT(*.pct)\0*.pct\0"
	//"SGI(*.sgi)\0*.sgi\0"
	"Targa(*.tga)\0*.tga\0"
	"WebP(*.webp)\0*.webp\0"
	"TIFF(*.tiff;*.tif)\0*.tiff;*.tif\0"
	"ICON(*.ico)\0*.ico\0"
	"PCX(*.pcx)\0*.pcx\0"
//	"EXR(*.exr)\0*.exr\0"
//	"IFF(*.iff)\0*.iff\0"
	"WebP(*.webp)\0*.webp\0"*/,
	"AutoShrink Image Reader by MT Bld("__DATE__")",
	func_init,
	func_exit,
	func_open,
	func_close,
	func_info_get,
	func_read_video,
	NULL, //func_read audio: NA
	NULL, //func_is_keyframe: NA
	NULL  //func_config: RESERVED
};
/*
INPUT_PLUGIN_TABLE input_plugin_table_fmt2 = {
	INPUT_PLUGIN_FLAG_VIDEO,
	"AutoShrink 2-tier",
	"ImageFile\0*.jp2;*.j2k;*.j2c;*.jng;*.pct;*.pcx;*.tiff;*.tif;*.ico",
	"AutoShrink JPEG2000, PCT, PCX, TIFF, ICO",
	func_init,
	func_exit,
	func_open,
	func_close,
	func_info_get,
	func_read_video,
	NULL, //func_read audio: NA
	NULL, //func_is_keyframe: NA
	NULL  //func_config: RESERVED
};

INPUT_PLUGIN_TABLE input_plugin_table_fmt3 = {
	INPUT_PLUGIN_FLAG_VIDEO,
	"AutoShrink Rare",
	"ImageFile\0*.sgi;*.tga;*.webp;*.mng;*.dds;*.exr;*.g3;*.iff;*.lbm;*.pbm;*.pgm;*.wbm;*.xbm;*.xpm",
	"AutoShrink SGI,TGA,WEBP,EXR...",
	func_init,
	func_exit,
	func_open,
	func_close,
	func_info_get,
	func_read_video,
	NULL, //func_read audio: NA
	NULL, //func_is_keyframe: NA
	NULL  //func_config: RESERVED
};

INPUT_PLUGIN_TABLE input_plugin_table_fmt4 = {
	INPUT_PLUGIN_FLAG_VIDEO,
	"AutoShrink high-end",
	"ImageFile\0*.hdr;*.jxr;*.hdp;*.psd;*.ras;*.pcd",
	"AutoShrink HDR,JXR,PSD,PCD",
	func_init,
	func_exit,
	func_open,
	func_close,
	func_info_get,
	func_read_video,
	NULL, //func_read audio: NA
	NULL, //func_is_keyframe: NA
	NULL  //func_config: RESERVED
};

INPUT_PLUGIN_TABLE input_plugin_table_fmt5 = {
	INPUT_PLUGIN_FLAG_VIDEO,
	"AutoShrink Camera:Sony_Canon_Nikon",
	"ImageFile\0*.arw;*.cr2;*.crw;*.sr2;*.srf;*.nef;*.nrw",
	"AutoShrink RAW from Sony, Canon and Nikon cameras",
	func_init,
	func_exit,
	func_open,
	func_close,
	func_info_get,
	func_read_video,
	NULL, //func_read audio: NA
	NULL, //func_is_keyframe: NA
	NULL  //func_config: RESERVED
};

INPUT_PLUGIN_TABLE input_plugin_table_fmt6 = {
	INPUT_PLUGIN_FLAG_VIDEO,
	"AutoShrink Camera:Others",
	"ImageFile\0*.dcr;*.drf;*.dsc;*.erf;*.k25;*.kc2;*.orf;*.pef;*.ptx;*.rw2",
	"AutoShrink RAW from Kodak,Epson,Olympus,Pentax,Panasonic",
	func_init,
	func_exit,
	func_open,
	func_close,
	func_info_get,
	func_read_video,
	NULL, //func_read audio: NA
	NULL, //func_is_keyframe: NA
	NULL  //func_config: RESERVED
};

INPUT_PLUGIN_TABLE input_plugin_table_fmt7 = {
	INPUT_PLUGIN_FLAG_VIDEO,
	"AutoShrink Camera:Common",
	"ImageFile\0*.dng;*.raw",
	"AutoShrink Adobe DNG and general RAW",
	func_init,
	func_exit,
	func_open,
	func_close,
	func_info_get,
	func_read_video,
	NULL, //func_read audio: NA
	NULL, //func_is_keyframe: NA
	NULL  //func_config: RESERVED
};
*/
UINT32 aviutl_width = 0;
UINT32 aviutl_height = 0;

// Helper functions
WCHAR* U16Filename(LPSTR mb_filename) //convert mbcs filename to UTF-16
{
	WCHAR* u16path = new WCHAR[MAX_PATH];
	SecureZeroMemory(u16path, MAX_PATH * 2);
	size_t filename_len = strlen(mb_filename);
	int charCount = MultiByteToWideChar(CP_THREAD_ACP, NULL, mb_filename, filename_len, nullptr, 0);
	MultiByteToWideChar(CP_THREAD_ACP, NULL, mb_filename, filename_len, u16path, charCount);
	return u16path;
}

// AviUtl functions to be implemented

BOOL func_init(void)
{
#ifdef FREEIMAGE_LIB
	FreeImage_Initialise();
#endif // FREEIMAGE_LIB STATIC ONLY
	//Get AviUtl system width and height from INI
	WCHAR curr_dir[MAX_PATH] = { 0 };
	DWORD dir_len = GetCurrentDirectoryW(0, nullptr);
	GetCurrentDirectoryW(dir_len, curr_dir);
	wcscat_s(curr_dir, MAX_PATH, L"\\aviutl.ini");
	aviutl_width = GetPrivateProfileIntW(L"system", L"width", 1920, curr_dir);
	aviutl_height = GetPrivateProfileIntW(L"system", L"height", 1080, curr_dir);
	return TRUE;
}
BOOL func_exit(void)
{
#ifdef FREEIMAGE_LIB
	FreeImage_DeInitialise();
#endif // FREEIMAGE_LIB STATIC ONLY
	return TRUE;
}
INPUT_HANDLE func_open(LPSTR file)
{
	WCHAR* u16name = U16Filename(file);
	//Get UTF8 filename
	//size_t u8bytes = WideCharToMultiByte(CP_UTF8, 0, u16name, -1, nullptr, 0, NULL, NULL);
	//CHAR* u8name = new CHAR[u8bytes];
	//SecureZeroMemory(u8name, u8bytes);
	//WideCharToMultiByte(CP_UTF8, 0, u16name, -1, u8name, u8bytes, NULL, NULL);
	//

	FREE_IMAGE_FORMAT image_format = FreeImage_GetFileTypeU(u16name);
	if (image_format == FREE_IMAGE_FORMAT::FIF_UNKNOWN)
	{
		image_format = FreeImage_GetFIFFromFilenameU(u16name);
	}
	if (image_format == FREE_IMAGE_FORMAT::FIF_UNKNOWN)
	{
		MessageBox(nullptr, "Unknow Image Type", "AutoShrink Bitmap Reader Error", MB_OK);
		return nullptr;
	}
	BOOL isGIF = image_format == FREE_IMAGE_FORMAT::FIF_GIF;
	BOOL isICO = image_format == FREE_IMAGE_FORMAT::FIF_ICO;
	BOOL isTIFF = image_format == FREE_IMAGE_FORMAT::FIF_TIFF;

	FIHANDLE *fh = new FIHANDLE;
	INPUT_INFO *iip = new INPUT_INFO;
	SecureZeroMemory(iip, sizeof(INPUT_INFO));
	//Set Common iip member
	iip->flag = INPUT_INFO_FLAG_VIDEO | INPUT_INFO_FLAG_VIDEO_RANDOM_ACCESS;
	iip->format = new BITMAPINFOHEADER;
	SecureZeroMemory(iip->format, sizeof(BITMAPINFOHEADER));
	iip->format_size = sizeof(BITMAPINFOHEADER);
	iip->format->biBitCount = 32; //with transparency
	iip->format->biCompression = BI_RGB;
	iip->format->biPlanes = 1;
	iip->handler = NULL;
	fh->iip = iip;

	//if (isGIF || isICO || isTIFF)
	/*if (FALSE)
	{
		//Multi-page open
		fh->isMultiPage = TRUE;
		fh->fiformat = image_format;
		fh->mbitmap = FreeImage_OpenMultiBitmap(image_format, u8name, true, true, false);
		fh->sbitmap = FreeImage_LockPage(fh->mbitmap, 0); //the 1st frame as default
		//fh->iip->n = FreeImage_GetPageCount(fh->mbitmap);
		fh->iip->rate = 1;
		fh->iip->scale = 1;
		fh->iip->n = 1;
	}*/
	//else
	//{
		//simple open
		fh->isMultiPage = FALSE;
		fh->fiformat = image_format;
		fh->mbitmap = nullptr;
		fh->sbitmap = FreeImage_LoadU(image_format, u16name);

		fh->iip->rate = 1;
		fh->iip->scale = 1;
		fh->iip->n = 1;
	//}
	//check if rescale is needed
	UINT32 src_w, src_h;
	src_w = FreeImage_GetWidth(fh->sbitmap);
	src_h = FreeImage_GetHeight(fh->sbitmap);
	if ((src_w > aviutl_width) || (src_h > aviutl_height)) //if too large
	{
		//rescale dimensions
		double img_ar, scr_ar;
		img_ar = (double)src_w / (double)src_h;
		scr_ar = (double)aviutl_width / (double)aviutl_height;
		if (scr_ar > img_ar)
		{
			fh->output_w = src_w* aviutl_height / src_h;
			fh->output_h = aviutl_height;
		}
		else
		{
			fh->output_w = aviutl_width;
			fh->output_h = src_h*aviutl_width / src_w;
		}
	}
	else
	{
		//keep original otherwise
		fh->output_w = src_w;
		fh->output_h = src_h;
	}
	//Set dimension info in iip
	fh->iip->format->biHeight = fh->output_h;
	fh->iip->format->biWidth = fh->output_w;
	fh->iip->format->biSizeImage = fh->output_h * fh->output_w * 4;
	
	//delete[] u8name;
	delete[] u16name;
	return fh;
}
BOOL func_close(INPUT_HANDLE ih)
{
	FIHANDLE* fh = (FIHANDLE*)ih;
	if (!fh->mbitmap)
	{
		FreeImage_CloseMultiBitmap(fh->mbitmap);
	}
	if (!fh->sbitmap)
	{
		FreeImage_Unload(fh->sbitmap);
	}
	if (fh->iip->format)
	{
		delete fh->iip->format;
	}
	delete fh->iip;
	delete fh;
	return TRUE;
}
BOOL func_info_get(INPUT_HANDLE ih, INPUT_INFO *iip)
{
	
	if (ih)
	{
		FIHANDLE* fh = (FIHANDLE*)ih;
		iip->flag = INPUT_INFO_FLAG_VIDEO | INPUT_INFO_FLAG_VIDEO_RANDOM_ACCESS;
		iip->rate = 1;
		iip->scale = 1;
		iip->n = 1;
		iip->handler = NULL;
		iip->format = fh->iip->format;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	
}
int func_read_video(INPUT_HANDLE ih, int frame, void *buf)
{
	if (!ih) return 0;
	FIHANDLE* fh = (FIHANDLE*)ih;

		UINT32 src_w, src_h;
		src_w = FreeImage_GetWidth(fh->sbitmap);
		src_h = FreeImage_GetHeight(fh->sbitmap);
		FIBITMAP* fibuffer = nullptr;
		FIBITMAP* fismall = nullptr;
		if ((src_w > aviutl_width) || (src_h > aviutl_height))
		{
			//Shrink
			fismall = FreeImage_Rescale(fh->sbitmap, fh->output_w, fh->output_h, FREE_IMAGE_FILTER::FILTER_BOX);
		}
		//Convert to BGRA32
		//See if HDR is required
		if (fh->fiformat != FREE_IMAGE_FORMAT::FIF_HDR)
		{
			//Non-HDR image
			if (fismall)
			{
				fibuffer = FreeImage_ConvertTo32Bits(fismall);
			}
			else
			{
				fibuffer = FreeImage_ConvertTo32Bits(fh->sbitmap);
			}
			
		}
		else
		{
			if (fismall)
			{
				fibuffer = FreeImage_TmoDrago03(fismall);
			}
			else
			{
				fibuffer = FreeImage_TmoDrago03(fh->sbitmap);
			}
			
		}
		//Now fibuffer must have an image, we can clean up unused resource
		if (fismall)
		{
			FreeImage_Unload(fismall);
		}
		if (fh->sbitmap)
		{
			if (fh->isMultiPage)
			{
				FreeImage_UnlockPage(fh->mbitmap, fh->sbitmap, false);
				fh->sbitmap = nullptr;
				FreeImage_CloseMultiBitmap(fh->mbitmap);
				fh->mbitmap = nullptr;
			}
			else
			{
				FreeImage_Unload(fh->sbitmap);
				fh->sbitmap = nullptr;
			}
		}
		//Copy to AviUtl
		size_t stride = FreeImage_GetPitch(fibuffer);
		size_t output_size = stride * fh->output_h;
		BYTE* dest = (BYTE*)buf;
		for (UINT32 h = 0; h < fh->output_h; h++)
		{
			BYTE* scan = FreeImage_GetScanLine(fibuffer, h);
			memcpy_s(dest, stride, scan, stride);
			dest += stride;
		}
		FreeImage_Unload(fibuffer);
		return output_size;

}
//BOOL func_is_keyframe(INPUT_HANDLE ih, int frame);
//BOOL func_config(HWND hwnd, HINSTANCE dll_hinst);

//

//INPUT_PLUGIN_TABLE *filter_list[] = { &input_plugin_table_fmt1, &input_plugin_table_fmt2, &input_plugin_table_fmt3, &input_plugin_table_fmt4, &input_plugin_table_fmt5, &input_plugin_table_fmt6, &input_plugin_table_fmt7, NULL };

EXTERN_C INPUT_PLUGIN_TABLE __declspec(dllexport) * __stdcall GetInputPluginTable(void)
{
	return &input_plugin_table;
}