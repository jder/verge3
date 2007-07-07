/// The VERGE 3 Project is originally by Ben Eirich and is made available via
///  the BSD License.
///
/// Please see LICENSE in the project's root directory for the text of the
/// licensing agreement.  The CREDITS file in the same directory enumerates the
/// folks involved in this public build.
///
/// If you have altered this source file, please log your name, date, and what
/// changes you made below this line.


/****************************************************************
	xerxes engine
	a_image.cpp
 ****************************************************************/

#include "xerxes.h"

image* xLoadImage_int_respect8bitTransparency(char* fname)
{
	image* newimage;
	
	corona::Image* img;

	log("loading image %s", fname);
	if (Exist(fname))
	{
		img = corona::OpenImage(fname, corona::FF_AUTODETECT, corona::PF_DONTCARE);
	}
	else
	{
		VFILE* vf = vopen(fname);
		if (!vf)
		{
			err("loadimage: couldn't load image %s; couldnt find a file or a vfile", fname);
		}
		FILE* f = fopen("v3img_temp.$$$", "wb");
		if (!f)
		{
			// TODO: implement
		}
		int l = filesize(vf);
		char* buffer = new char[l];
		vread(buffer, l, vf);
		fwrite(buffer, 1, l, f);
		vclose(vf);
		fclose(f);
		img = corona::OpenImage("v3img_temp.$$$", corona::FF_AUTODETECT, corona::PF_DONTCARE);
		remove("v3img_temp.$$$");
	}

	if (!img)
	{
		err("loadimage: couldn't load image %s; corona just bombed.", fname);
	}

	if (img->getFormat() == corona::PF_I8)
	{
		int transparency_index = 0;
		if (img->getPaletteFormat() != corona::PF_R8G8B8)
		{
			//we can convert this one. we get it with new corona 1.0.2
			//8bpp gif loader
			if (img->getPaletteFormat() == corona::PF_R8G8B8A8)
			{
				unsigned char* p = (unsigned char*) img->getPalette();
				int c = img->getPaletteSize();
				unsigned char* ps = p + 4;
				unsigned char* pd = p + 3;
				for (int i = 1; i < c; i++)
				{
					*pd++ = *ps++;
					*pd++ = *ps++;
					*pd++ = *ps++;
					if (!*ps++)
					{
						transparency_index = i;
					}
				}
			}
			else
			{
				err("loadimage: couldnt load image %s; unexpected pixel format", fname);
			}
		}
			
		unsigned char* pal2 = (unsigned char*) img->getPalette();
		unsigned char pal[768];
		memcpy(pal, pal2, img->getPaletteSize()*3);		

		int ofs = 3*transparency_index;
		pal[ofs++] = 255;
		pal[ofs++] = 0;
		pal[ofs++] = 255;

		newimage = ImageFrom8bpp((unsigned char*) img->getPixels(), img->getWidth(), img->getHeight(), pal);
		delete img;
		return newimage;
	}
	else
	{
		img = corona::ConvertImage(img,corona::PF_R8G8B8);
		newimage=ImageFrom24bpp((unsigned char *)img->getPixels(),img->getWidth(),img->getHeight());
		delete img;
        return newimage;
	}
}

image *xLoadImage_int(char *fname,int tflag)
{
	image *newimage;

	corona::Image *img;

	log("loading image %s",fname);
	if (Exist(fname))
		img = corona::OpenImage(fname,corona::FF_AUTODETECT,corona::PF_DONTCARE);
	else
	{
		VFILE *vf = vopen(fname);
		if (!vf)
			err("loadimage: couldn't load image %s; couldnt find a file or a vfile",fname);
		FILE *f = fopen("v3img_temp.$$$","wb");
		int l = filesize(vf);
		char *buffer = new char[l];
		vread(buffer, l, vf);
		fwrite(buffer, 1, l, f);
		vclose(vf);
		fclose(f);
		img = corona::OpenImage("v3img_temp.$$$",corona::FF_AUTODETECT,corona::PF_DONTCARE);
		remove("v3img_temp.$$$");
	}
	if(!img)
		err("loadimage: couldn't load image %s; corona just bombed.",fname);
	if(img->getFormat()==corona::PF_I8)
	{
		if(img->getPaletteFormat()!=corona::PF_R8G8B8) {

			//we can convert this one. we get it with new corona 1.0.2
			//8bpp gif loader
			if(img->getPaletteFormat() == corona::PF_R8G8B8A8) {
				unsigned char *p=(unsigned char *)img->getPalette();
				int c = img->getPaletteSize();
				unsigned char *ps = p+4, *pd = p+3;
				for(int i=1;i<c;i++) {
					*pd++ = *ps++;
					*pd++ = *ps++;
					*pd++ = *ps++;
					ps++;
				}
			}
			else err("loadimage: couldnt load image %s; unexpected pixel format",fname);
		}

		unsigned char *pal2=(unsigned char *)img->getPalette();
		unsigned char pal[768];
		memcpy(pal, pal2, img->getPaletteSize()*3);
		if(tflag)
		{
			pal[0]=255; pal[1]=0; pal[2]=255;
		}
		newimage=ImageFrom8bpp((unsigned char *)img->getPixels(), img->getWidth(), img->getHeight(), pal);
		delete img;
		return newimage;
	}
	else
	{
		img = corona::ConvertImage(img,corona::PF_R8G8B8);
		newimage=ImageFrom24bpp((unsigned char *)img->getPixels(),img->getWidth(),img->getHeight());
		delete img;
        return newimage;
	}
}
//#endif

image *xLoadImage(char *fname)
{
	return xLoadImage_int(fname, 0);
}

image *xLoadImage0(char *fname)
{
	return xLoadImage_int(fname, 1);
}

image* xLoadImage8(char* fname)
{
	return xLoadImage_int_respect8bitTransparency(fname);
}