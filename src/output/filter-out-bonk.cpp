 /* BonkEnc Audio Encoder
  * Copyright (C) 2001-2004 Robert Kausch <robert.kausch@bonkenc.org>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the "GNU General Public License".
  *
  * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
  * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. */

#define __THROW_BAD_ALLOC exit(1)

#include <iolib-cxx.h>
#include <output/filter-out-bonk.h>
#include <dllinterfaces.h>
#include <memory.h>
#include <id3/tag.h>

FilterOutBONK::FilterOutBONK(bonkEncConfig *config, bonkFormatInfo *format) : OutputFilter(config, format)
{
	if (format->channels > 2)
	{
		QuickMessage("BonkEnc does not support more than 2 channels!", "Error", MB_OK, IDI_HAND);

		error = 1;

		return;
	}

	packageSize = int(1024.0 * format->rate / 44100) * format->channels * (currentConfig->bonk_lossless ? 1 : currentConfig->bonk_downsampling) * (format->bits / 8);
}

FilterOutBONK::~FilterOutBONK()
{
}

bool FilterOutBONK::Activate()
{
	d_out	= new OutStream(STREAM_DRIVER, driver);

	if (format->trackInfo->hasText && currentConfig->enable_tags && currentConfig->enable_id3)
	{
		ID3Tag		*tag = ex_ID3Tag_New();

		ex_ID3Tag_SetPadding(tag, false);

		ID3Frame	*artist = ex_ID3Frame_NewID(ID3FID_LEADARTIST);

		if (format->trackInfo->artist != NIL)
		{
			if (String::IsUnicode(format->trackInfo->artist))
			{
				ex_ID3Field_SetEncoding(ex_ID3Frame_GetField(artist, ID3FN_TEXT), ID3TE_UTF8);
				ex_ID3Field_SetASCII(ex_ID3Frame_GetField(artist, ID3FN_TEXT), (char *) format->trackInfo->artist.ConvertTo("UTF-8"));
			}
			else
			{
				ex_ID3Field_SetASCII(ex_ID3Frame_GetField(artist, ID3FN_TEXT), (char *) format->trackInfo->artist);
			}

			ex_ID3Tag_AddFrame(tag, artist);
		}

		ID3Frame	*title = ex_ID3Frame_NewID(ID3FID_TITLE);

		if (format->trackInfo->title != NIL)
		{
			if (String::IsUnicode(format->trackInfo->title))
			{
				ex_ID3Field_SetEncoding(ex_ID3Frame_GetField(title, ID3FN_TEXT), ID3TE_UTF8);
				ex_ID3Field_SetASCII(ex_ID3Frame_GetField(title, ID3FN_TEXT), (char *) format->trackInfo->title.ConvertTo("UTF-8"));
			}
			else
			{
				ex_ID3Field_SetASCII(ex_ID3Frame_GetField(title, ID3FN_TEXT), (char *) format->trackInfo->title);
			}

			ex_ID3Tag_AddFrame(tag, title);
		}

		ID3Frame	*album = ex_ID3Frame_NewID(ID3FID_ALBUM);

		if (format->trackInfo->album != NIL)
		{
			if (String::IsUnicode(format->trackInfo->album))
			{
				ex_ID3Field_SetEncoding(ex_ID3Frame_GetField(album, ID3FN_TEXT), ID3TE_UTF8);
				ex_ID3Field_SetASCII(ex_ID3Frame_GetField(album, ID3FN_TEXT), (char *) format->trackInfo->album.ConvertTo("UTF-8"));
			}
			else
			{
				ex_ID3Field_SetASCII(ex_ID3Frame_GetField(album, ID3FN_TEXT), (char *) format->trackInfo->album);
			}

			ex_ID3Tag_AddFrame(tag, album);
		}

		ID3Frame	*track = ex_ID3Frame_NewID(ID3FID_TRACKNUM);

		if (format->trackInfo->track > 0)
		{
			if (format->trackInfo->track < 10)	ex_ID3Field_SetASCII(ex_ID3Frame_GetField(track, ID3FN_TEXT), (char *) String("0").Append(String::FromInt(format->trackInfo->track)));
			else					ex_ID3Field_SetASCII(ex_ID3Frame_GetField(track, ID3FN_TEXT), (char *) String::FromInt(format->trackInfo->track));

			ex_ID3Tag_AddFrame(tag, track);
		}

		ID3Frame	*year = ex_ID3Frame_NewID(ID3FID_YEAR);

		if (format->trackInfo->year > 0)
		{
			ex_ID3Field_SetASCII(ex_ID3Frame_GetField(year, ID3FN_TEXT), (char *) String::FromInt(format->trackInfo->year));

			ex_ID3Tag_AddFrame(tag, year);
		}

		ID3Frame	*genre = ex_ID3Frame_NewID(ID3FID_CONTENTTYPE);

		if (format->trackInfo->genre != NIL)
		{
			if (String::IsUnicode(format->trackInfo->genre))
			{
				ex_ID3Field_SetEncoding(ex_ID3Frame_GetField(genre, ID3FN_TEXT), ID3TE_UTF8);
				ex_ID3Field_SetASCII(ex_ID3Frame_GetField(genre, ID3FN_TEXT), (char *) format->trackInfo->genre.ConvertTo("UTF-8"));
			}
			else
			{
				ex_ID3Field_SetASCII(ex_ID3Frame_GetField(genre, ID3FN_TEXT), (char *) format->trackInfo->genre);
			}

			ex_ID3Tag_AddFrame(tag, genre);
		}

		ID3Frame	*comment = ex_ID3Frame_NewID(ID3FID_COMMENT);

		if (String::IsUnicode(currentConfig->default_comment))
		{
			ex_ID3Field_SetEncoding(ex_ID3Frame_GetField(comment, ID3FN_TEXT), ID3TE_UTF8);
			ex_ID3Field_SetASCII(ex_ID3Frame_GetField(comment, ID3FN_TEXT), (char *) currentConfig->default_comment.ConvertTo("UTF-8"));
		}
		else
		{
			ex_ID3Field_SetASCII(ex_ID3Frame_GetField(comment, ID3FN_TEXT), (char *) currentConfig->default_comment);
		}

		ex_ID3Tag_AddFrame(tag, comment);

		unsigned char	*buffer = new unsigned char [32768];
		int		 size = ex_ID3Tag_Render(tag, buffer, ID3TT_ID3V2);

		d_out->OutputNumber(0, 1);
		d_out->OutputNumber(32, 1);
		d_out->OutputData((void *) buffer, size);
		d_out->OutputNumber(size + 10, 4);
		d_out->OutputString(" id3");

		delete [] buffer;

		ex_ID3Tag_Delete(tag);
		ex_ID3Frame_Delete(artist);
		ex_ID3Frame_Delete(title);
		ex_ID3Frame_Delete(album);
		ex_ID3Frame_Delete(track);
		ex_ID3Frame_Delete(year);
		ex_ID3Frame_Delete(genre);
		ex_ID3Frame_Delete(comment);
	}

	encoder	= ex_bonk_create_encoder(d_out,
		max((int) format->length, 0), format->rate, format->channels,
		currentConfig->bonk_lossless, currentConfig->bonk_jstereo,
		currentConfig->bonk_predictor, currentConfig->bonk_lossless ? 1 : currentConfig->bonk_downsampling,
		int(1024.0 * format->rate / 44100),
		0.05 * (double) currentConfig->bonk_quantization);

	return true;
}

bool FilterOutBONK::Deactivate()
{
	ex_bonk_close_encoder(encoder);

	delete d_out;

	return true;
}

int FilterOutBONK::WriteData(unsigned char *data, int size)
{
	int		 pos = d_out->GetPos();
	vector<int>	 samples(size / (format->bits / 8));

	for (int i = 0; i < size / (format->bits / 8); i++)
	{
		if (format->bits == 8)	samples[i] = (data[i] - 128) * 256;
		if (format->bits == 16)	samples[i] = ((short *) data)[i];
		if (format->bits == 24) samples[i] = (int) (data[3 * i] + 256 * data[3 * i + 1] + 65536 * data[3 * i + 2] - (data[3 * i + 2] & 128 ? 16777216 : 0)) / 256;
		if (format->bits == 32)	samples[i] = (int) ((long *) data)[i] / 65536;
	}

	ex_bonk_encode_packet(encoder, samples);

	return d_out->GetPos() - pos;
}
