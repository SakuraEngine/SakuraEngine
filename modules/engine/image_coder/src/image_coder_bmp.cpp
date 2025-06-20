#include "image_coder_bmp.hpp"
#include "SkrContainersDef/vector.hpp"
#include <bit>

namespace skr {

static inline bool BmpDimensionIsValid(int32_t Dim)
{
    //ensures we can do Width*4 in int32
    //	and then also call Align() on it
    const int32_t BmpMaxDimension = (INT32_MAX/4) - 2;

    // zero dimension could be technically valid, but we won't load it
    return Dim > 0 && Dim <= BmpMaxDimension;
}

static inline bool SafeAdvancePointer(const uint8_t*& OutPtr, const uint8_t* StartPtr, const uint8_t* EndPtr, ptrdiff_t Step)
{
	if ( Step < 0 || Step > (EndPtr - StartPtr) )
	{
		return false;
	}
	OutPtr = StartPtr + Step;
	return true;
}

BMPImageDecoder::BMPImageDecoder() SKR_NOEXCEPT
{

}

BMPImageDecoder::~BMPImageDecoder() SKR_NOEXCEPT
{

}

bool BMPImageDecoder::load_bmp_header() SKR_NOEXCEPT
{
    auto Buffer = encoded_view.data();
    bmhdr = (FBitmapInfoHeader *)(Buffer + sizeof(FBitmapFileHeader));
    bmfh = (const FBitmapFileHeader*)Buffer;

    const auto Width = bmhdr->biWidth;
    auto Height = std::abs(bmhdr->biHeight);
    if ( bHalfHeight )
	{
		Height /= 2;
	}
    setRawProps(Width, Height, IMAGE_CODER_COLOR_FORMAT_BGRA, 8);

    return true;
}

bool BMPImageDecoder::initialize(const uint8_t* data, uint64_t size) SKR_NOEXCEPT
{
    if (BaseImageDecoder::initialize(data, size))
    {
        return load_bmp_header();
    }
    return false;
}

EImageCoderFormat BMPImageDecoder::get_image_format() const SKR_NOEXCEPT
{
    return EImageCoderFormat::IMAGE_CODER_FORMAT_BMP;
}

#define ALIGN_UP(x, y) (((x) + ((y)-1)) & ~((y)-1))

bool BMPImageDecoder::decode(EImageCoderColorFormat in_format, uint32_t bit_depth) SKR_NOEXCEPT
{
	SKR_ASSERT(in_format == IMAGE_CODER_COLOR_FORMAT_BGRA || in_format == IMAGE_CODER_COLOR_FORMAT_RGBA);

    auto Buffer = encoded_view.data();
    auto BufferEnd = Buffer + encoded_view.size();
    EBitmapHeaderVersion HeaderVersion = EBitmapHeaderVersion::BHV_BITMAPINFOHEADER;
    auto BitsOffset = bmfh->bfOffBits;
    HeaderVersion = bmhdr->GetHeaderVersion();
    if ( HeaderVersion == EBitmapHeaderVersion::BHV_INVALID )
	{
		SKR_LOG_ERROR(u8"BitmapHeaderVersion invalid");
		return false;
	}

	const bool isRGBA = (in_format == IMAGE_CODER_COLOR_FORMAT_RGBA);
	/*
	UE_LOG(LogImageWrapper, Log, TEXT("BMP compression = (%i) BitCount = (%i)"), bmhdr->biCompression,bmhdr->biBitCount)
	UE_LOG(LogImageWrapper, Log, TEXT("BMP BitsOffset = (%i)"), BitsOffset)
	UE_LOG(LogImageWrapper, Log, TEXT("BMP biSize = (%i)"), bmhdr->biSize)
	*/

	if (bmhdr->biCompression != BCBI_RGB && bmhdr->biCompression != BCBI_BITFIELDS && bmhdr->biCompression != BCBI_ALPHABITFIELDS)
	{
		SKR_LOG_ERROR(u8"RLE compression of BMP images not supported");
		return false;
	}
	
	if (bmhdr->biPlanes != 1 )
	{
		SKR_LOG_ERROR(u8"BMP uses an unsupported biPlanes != 1");
		return false;
	}
	
	if (bmhdr->biBitCount != 8 && bmhdr->biBitCount != 16 && bmhdr->biBitCount != 24 && bmhdr->biBitCount != 32)
	{
		SKR_LOG_ERROR(u8"BMP uses an unsupported biBitCount (%i)", bmhdr->biBitCount);
		return false;
	}
	
	const uint8_t* Bits;
	if (!SafeAdvancePointer(Bits, Buffer, BufferEnd, BitsOffset))
	{
		SKR_LOG_ERROR(u8"Bmp read would overrun buffer");
		return false;
	}
	
	const uint8_t * AfterHeader;
	if (!SafeAdvancePointer(AfterHeader, (const uint8_t*)bmhdr, BufferEnd, bmhdr->biSize))
	{
		SKR_LOG_ERROR(u8"Bmp read would overrun buffer");
		return false;
	}

	// Set texture properties.
	// This should have already been set by LoadHeader from SetCompressed
    const auto Width = get_width();
    const auto Height = get_height();
	const bool bNegativeHeight = (bmhdr->biHeight < 0);
	if (!BmpDimensionIsValid(Width) || ! BmpDimensionIsValid(Height))
	{
		SKR_LOG_ERROR(u8"Bmp dimensions invalid");
		return false;
	}

	int64_t RawDataBytes = Width * (int64_t)Height * 4;
	if (RawDataBytes > INT32_MAX)
	{
		SKR_LOG_ERROR(u8"Bmp dimensions invalid");
		return false;
	}

    decoded_size = RawDataBytes;
    decoded_data = BMPImageDecoder::Allocate(decoded_size, get_alignment());

    uint8_t* ImageData = decoded_data;
	// Copy scanlines, accounting for scanline direction according to the Height field.
	const int32_t SrcBytesPerPel = (bmhdr->biBitCount / 8);
	SKR_ASSERT(SrcBytesPerPel * 8 == bmhdr->biBitCount);
	const int32_t SrcStride = ALIGN_UP(Width * SrcBytesPerPel, 4);
	if (SrcStride <= 0)
	{
		SKR_LOG_ERROR(u8"Bmp dimensions invalid");
		return false;
	}

	const int64_t SrcDataSize = SrcStride * (int64_t)Height;
	if (SrcDataSize > (BufferEnd - Bits))
	{
		SKR_LOG_ERROR(u8"Bmp read would overrun buffer");
		return false;
	}
	
	const int32_t SrcPtrDiff = bNegativeHeight ? SrcStride : -SrcStride;
	const uint8_t* SrcPtr = Bits + (bNegativeHeight ? 0 : Height - 1) * SrcStride;

	if (bmhdr->biBitCount == 8)
	{
		// Do palette.
		// If the number for color palette entries is 0, we need to default to 2^biBitCount entries.  In this case 2^8 = 256
		uint32_t clrPaletteCount = bmhdr->biClrUsed ? bmhdr->biClrUsed : 256;
		if ( clrPaletteCount > 256 )
		{
			SKR_LOG_ERROR(u8"Bmp paletteCount over 256");
			return false;
		}
		
		const uint8_t* bmpal = AfterHeader;
		if ( clrPaletteCount*4 > (BufferEnd - bmpal) )
		{
			SKR_LOG_ERROR(u8"Bmp read would overrun buffer");
			return false;
		}

        struct FColor
        {
            uint8_t B, G, R, A;
        };
		skr::Vector<FColor>	Palette;
		Palette.resize_unsafe(256);

		for (uint32_t i = 0; i < clrPaletteCount; i++)
		{
			Palette[i] = FColor(bmpal[i * 4 + 2], bmpal[i * 4 + 1], bmpal[i * 4 + 0], 255);
		}

		for(uint32_t i= clrPaletteCount; i < 256; i++)
		{
			Palette[i] = FColor(0, 0, 0, 255);
		}
		
		FColor* ImageColors = (FColor*)ImageData;
		for (int32_t Y = 0; Y < Height; Y++)
		{
			for (int32_t X = 0; X < Width; X++)
			{
				*ImageColors++ = Palette[SrcPtr[X]];
			}
			SrcPtr += SrcPtrDiff;
		}
	}
	else if ( bmhdr->biBitCount==24)
	{
		for (int32_t Y = 0; Y < Height; Y++)
		{
			const uint32_t _X = isRGBA ? 2u : 0u;
			const uint32_t _Z = isRGBA ? 0u : 2u;

			const uint8_t* SrcRowPtr = SrcPtr;
			for (int32_t X = 0; X < Width; X++)
			{
				ImageData[_X] = *SrcRowPtr++;
				ImageData[1u] = *SrcRowPtr++;
				ImageData[_Z] = *SrcRowPtr++;
				ImageData[3u] = 0xFF;
				ImageData += 4;
			}
			SrcPtr += SrcPtrDiff;
		}
	}
	else if ( bmhdr->biBitCount==32 && bmhdr->biCompression == BCBI_RGB)
	{
		// This comment was previously here :
		//  "In BCBI_RGB compression the last 8 bits of the pixel are not used."
		// -> this agrees with MSDN but does not match what Photoshop does in practice
		//  photoshop writes 32-bit ARGB with non-trivial A using BI_RGB
		//	see "porsche512a_ARGB.bmp" also "porsche512a_notadvanced.bmp"
		//	(both the "advanced" and regular photoshop save do this)
		// 

		uint64_t TotalA = 0;
		for (int32_t Y = 0; Y < Height; Y++)
		{
			// this is just a memcpy, except the accumulation of TotalA
			const uint8_t* SrcRowPtr = SrcPtr;
			for (int32_t X = 0; X < Width; X++)
			{
				*ImageData++ = *SrcRowPtr++;
				*ImageData++ = *SrcRowPtr++;
				*ImageData++ = *SrcRowPtr++;
				TotalA += *SrcRowPtr;
				*ImageData++ = *SrcRowPtr++; // was doing = 0xFF , ignoring fourth byte, as per MSDN
			}

			SrcPtr += SrcPtrDiff;
		}

		if ( TotalA == 0 )
		{
			// assume that this is actually XRGB and they wrote zeros in A
			// go back through and change all A's to 0xFF
			ImageData = decoded_data;
			for (int32_t Y = 0; Y < Height; Y++)
			{
				for (int32_t X = 0; X < Width; X++)
				{
					ImageData[3] = 0xFF;
					ImageData +=  4;
				}
			}
		}
	}
	else if ( bmhdr->biBitCount==16 && bmhdr->biCompression == BCBI_RGB)
	{
		// 16 bit BI_RGB is 555

		for (int32_t Y = 0; Y < Height; Y++)
		{
			for (int32_t X = 0; X < Width; X++)
			{
				const uint32_t SrcPixel = ((const uint16_t*)SrcPtr)[X];

				// Set the color values in BGRA order.
				uint32_t r = (SrcPixel>>10)&0x1f;;
				uint32_t g = (SrcPixel>> 5)&0x1f;
				uint32_t b = (SrcPixel    )&0x1f;

				*ImageData++ = (uint8_t) ( (b<<3) | (b>>2) );
				*ImageData++ = (uint8_t) ( (g<<3) | (g>>2) );
				*ImageData++ = (uint8_t) ( (r<<3) | (r>>2) );
				*ImageData++ = 0xFF; // 555 BI_RGB does not use alpha bit
			}

			SrcPtr += SrcPtrDiff;
		}
	}
	else if ( ( bmhdr->biBitCount==16 || bmhdr->biBitCount==32 ) && ( bmhdr->biCompression == BCBI_BITFIELDS || bmhdr->biCompression == BCBI_ALPHABITFIELDS) )
	{
		// Advance past the 40-byte header to get to the color masks :
		//	(note that some bmps have the 52 or 56 byte header with biSize=40 so you cannot check biSize to verify you have valid masks)
		//check( bmhdr->biSize >= 52 );
		//	in theory you could check BitsOffset

		const uint8_t * bmhdrEnd;
		if ( ! SafeAdvancePointer(bmhdrEnd, (const uint8_t *)bmhdr, BufferEnd, sizeof(FBitmapInfoHeader)) )
		{
			SKR_LOG_ERROR(u8"Bmp read would overrun buffer");
			return false;
		}

		// A 52 or 56 byte InfoHeader has masks after a BitmapInfoHeader
		//	a v4 info header also has them in the same place
		//	so reading them from there works in both cases
		const FBmiColorsMask* ColorMask = (FBmiColorsMask*)bmhdrEnd;
		if ( sizeof(FBmiColorsMask) > (BufferEnd - (const uint8_t*)ColorMask) )
		{
			SKR_LOG_ERROR(u8"Bmp read would overrun buffer");
			return false;
		}

		// Header version 4 introduced the option to declare custom color space, so we can't just assume sRGB past that version.
		//If the header version is V4 or higher we need to make sure we are still using sRGB format
		if (HeaderVersion >= EBitmapHeaderVersion::BHV_BITMAPV4HEADER)
		{
			const FBitmapInfoHeaderV4* bmhdrV4 = (FBitmapInfoHeaderV4*)bmhdr;
			if ( sizeof(FBitmapInfoHeaderV4) > (BufferEnd - (const uint8_t*)bmhdrV4) )
			{
				SKR_LOG_ERROR(u8"Bmp read would overrun buffer");
				return false;
			}
				
			if (bmhdrV4->biCSType != (uint32_t)EBitmapCSType::BCST_LCS_sRGB && bmhdrV4->biCSType != (uint32_t)EBitmapCSType::BCST_LCS_WINDOWS_COLOR_SPACE)
			{
				SKR_LOG_ERROR(u8"BMP uses an unsupported custom color space definition, sRGB color space will be used instead.");
			}
		}

		//Calculating the bit mask info needed to remap the pixels' color values.
		// note RGBAMask[3] can be reading past the end of the header
		//	but we will just read payload bits, and then replace it later when !bHasAlphaChannel
		uint32_t RGBAMask[4];
		float MappingRatio[4];
		for (uint32_t MaskIndex = 0; MaskIndex < 4; MaskIndex++)
		{
			uint32_t Mask = RGBAMask[MaskIndex] = ColorMask->RGBAMask[MaskIndex];
			if ( Mask == 0 )
			{
				MappingRatio[MaskIndex] = 0;
			}
			else
			{
				// count the number of bits on in Mask by counting the zeros on each side:
				int32_t TrailingBits = std::countr_zero(Mask);
				int32_t NumberOfBits = 32 - (TrailingBits + std::countl_zero(Mask));
				SKR_ASSERT( NumberOfBits > 0 );

				// note: when NumberOfBits is >= 18, this is not exact (differs from if done in doubles)
				//		but we still get output in [0,255] and the error is small, so let it be
				// use ldexpf to put the >>TrailingBits in the multiply
				MappingRatio[MaskIndex] = ldexpf( (255.f / ((1ULL<<NumberOfBits) - 1) ) , -TrailingBits );
			}
		}

		//In header pre-version 4, we should ignore the last 32bit (alpha) content.
		const bool bHasAlphaChannel = RGBAMask[3] != 0 && HeaderVersion >= EBitmapHeaderVersion::BHV_BITMAPV4HEADER;
		if ( bmhdr->biSize == 56 && RGBAMask[3] != 0 )
		{
			// 56-byte Adobe headers ("bmpv3") might also have valid alpha masks
			//	legacy Unreal import was treating them as bHasAlphaChannel=false
			//	perhaps they should in fact have their alpha mask respected
			// note that Adobe actually uses BI_RGB not BI_BITFIELDS for ARGB output in some cases
			SKR_LOG_ERROR(u8"Adobe 56-byte header might have alpha but we ignore it %08X", RGBAMask[3]);				
		}

		float AlphaBias = 0.5f;
		if ( ! bHasAlphaChannel )
		{
			RGBAMask[3] = 0;
			MappingRatio[3] = 0.f;
			AlphaBias = 255.f;
		}
		
		const uint32_t _R = 0, _G = 1, _B = 2, _A = 3;
		const uint32_t _X = isRGBA ? _R : _B;
		const uint32_t _Y = _G;
		const uint32_t _Z = isRGBA ? _B : _R;
		const uint32_t _W = _A;
		if ( bmhdr->biBitCount == 32 )
		{
			for (int32_t Y = 0; Y < Height; Y++)
			{
				for (int32_t X = 0; X < Width; X++)
				{
					const uint32_t SrcPixel = ((const uint32_t*)SrcPtr)[X];
					// Set the color values in BGRA order.
					//	integer output of RoundToInt will always fit in U8, no clamp needed
					*ImageData++ = (uint8_t) ((SrcPixel & RGBAMask[_X]) * MappingRatio[_X] + 0.5f);
					*ImageData++ = (uint8_t) ((SrcPixel & RGBAMask[_Y]) * MappingRatio[_Y] + 0.5f);
					*ImageData++ = (uint8_t) ((SrcPixel & RGBAMask[_Z]) * MappingRatio[_Z] + 0.5f);
					*ImageData++ = (uint8_t) ((SrcPixel & RGBAMask[_W]) * MappingRatio[_W] + AlphaBias);
				}

				SrcPtr += SrcPtrDiff;
			}
		}
		else
		{
			// code dupe: only change from above loop is the type cast on SrcPtr[X]
			SKR_ASSERT( bmhdr->biBitCount == 16 );
			for (int32_t Y = 0; Y < Height; Y++)
			{
				for (int32_t X = 0; X < Width; X++)
				{
					const uint32_t SrcPixel = ((const uint16_t*)SrcPtr)[X];

					// Set the color values in BGRA order.
					//	integer output of RoundToInt will always fit in U8, no clamp needed
					*ImageData++ = (uint8_t) ((SrcPixel & RGBAMask[_X]) * MappingRatio[_X] + 0.5f);
					*ImageData++ = (uint8_t) ((SrcPixel & RGBAMask[_Y]) * MappingRatio[_Y] + 0.5f);
					*ImageData++ = (uint8_t) ((SrcPixel & RGBAMask[_Z]) * MappingRatio[_Z] + 0.5f);
					*ImageData++ = (uint8_t) ((SrcPixel & RGBAMask[_W]) * MappingRatio[_W] + AlphaBias);
				}

				SrcPtr += SrcPtrDiff;
			}
		}
	}
	else
	{
		SKR_LOG_ERROR(u8"BMP uses an unsupported format (planes=%i, bitcount=%i, compression=%i", 
			bmhdr->biPlanes, bmhdr->biBitCount, bmhdr->biCompression);
		return false;
	}

    return true;
}

}