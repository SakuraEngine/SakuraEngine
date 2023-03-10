#pragma once
#include "text_server/vector2.h"
#include "text_server/rid.h"
#include "text_server/rid_owner.h"
#include "text_server/containers.h"

namespace skr { namespace gdi { struct IGDIRenderer; struct IGDIImage; struct IGDITexture;  }  }

namespace godot {
struct Image 
{
	enum Format
	{
		FORMAT_None,
		FORMAT_RGB8,
		FORMAT_RGBA8,
		FORMAT_LA8,
		FORMAT_R8,
		FORMAT_COUNT
	};

	[[nodiscard]] static Ref<Image> create_from_data(skr::gdi::IGDIRenderer* renderer, uint32_t w, uint32_t h,
		bool p_use_mipmaps, Format format, const Span<const uint8_t> &p_data);

	void generate_mipmaps();
	Span<const uint8_t> get_data();
	uint32_t get_width() const;
	uint32_t get_height() const;
	Format get_format() const;

	skr::gdi::IGDIImage* underlying = nullptr;
};
using ImageFormat = Image::Format;

struct ImageTexture
{
	[[nodiscard]] static Ref<ImageTexture> create_from_image(skr::gdi::IGDIRenderer* renderer, Ref<Image> image);

	Size2 get_size() const;
	void update(const Ref<Image> image);
	RID get_rid() const;

	RID rid = {};
	skr::gdi::IGDITexture* underlying = nullptr;

	static RID_PtrOwner<ImageTexture> texture_owner;
};
} // namespace godot