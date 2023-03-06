#pragma once
#include "text_server/vector2.h"
#include "text_server/rid.h"
#include "text_server/containers.h"

namespace godot {
struct Image 
{
	enum Format
	{
		FORMAT_RGBA8,
		FORMAT_COUNT
	};

	[[nodiscard]] static Ref<Image> create_from_data(uint32_t w, uint32_t h, bool p_use_mipmaps, Format format, const Vector<uint8_t> &p_data);

	void generate_mipmaps();
	Span<uint8_t> get_data();
	uint32_t get_width() const;
	uint32_t get_height() const;
	Format get_format() const;

	void* data = nullptr;
};

using ImageFormat = Image::Format;

struct ImageTexture
{
	[[nodiscard]] static Ref<ImageTexture> create_from_image(Ref<Image> image);

	Size2 get_size() const;
	void update(const Ref<Image> image);
	RID get_rid() const;

	void* handle = nullptr;
};
} // namespace godot