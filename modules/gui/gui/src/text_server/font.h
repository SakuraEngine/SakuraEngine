/**************************************************************************/
/*  font.h                                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef FONT_H
#define FONT_H

#include "text_server/text_server.h"
#include "text_server/hashfuncs.h"

namespace godot {

class TextLine;
class TextParagraph;

/*************************************************************************/
/*  Font                                                                 */
/*************************************************************************/

class Font {
	struct ShapedTextKey {
		String text;
		int font_size = 14;
		float width = 0.f;
		BitField<TextServer::JustificationFlag> jst_flags = TextServer::JUSTIFICATION_NONE;
		BitField<TextServer::LineBreakFlag> brk_flags = TextServer::BREAK_MANDATORY;
		TextServer::Direction direction = TextServer::DIRECTION_AUTO;
		TextServer::Orientation orientation = TextServer::ORIENTATION_HORIZONTAL;

		bool operator==(const ShapedTextKey &p_b) const {
			return (font_size == p_b.font_size) && (width == p_b.width) && (jst_flags == p_b.jst_flags) && (brk_flags == p_b.brk_flags) && (direction == p_b.direction) && (orientation == p_b.orientation) && (text == p_b.text);
		}

		ShapedTextKey() {}
		ShapedTextKey(const String &p_text, int p_font_size, float p_width, BitField<TextServer::JustificationFlag> p_jst_flags, BitField<TextServer::LineBreakFlag> p_brk_flags, TextServer::Direction p_direction, TextServer::Orientation p_orientation) {
			text = p_text;
			font_size = p_font_size;
			width = p_width;
			jst_flags = p_jst_flags;
			brk_flags = p_brk_flags;
			direction = p_direction;
			orientation = p_orientation;
		}
	};

	struct ShapedTextKeyHasher {
		_FORCE_INLINE_ static uint32_t hash(const ShapedTextKey &p_a) {
			uint32_t hash = p_a.text.hash();
			hash = hash_murmur3_one_32(p_a.font_size, hash);
			hash = hash_murmur3_one_float(p_a.width, hash);
			const uint32_t _ = static_cast<uint32_t>(p_a.brk_flags | (p_a.jst_flags << 6) | (p_a.direction << 12) | (p_a.orientation << 15));
			hash = hash_murmur3_one_32(_, hash);
			return hash_fmix32(hash);
		}
	};

	// Shaped string cache.
	// mutable LRUCache<ShapedTextKey, Ref<TextLine>, ShapedTextKeyHasher> cache;
	// mutable LRUCache<ShapedTextKey, Ref<TextParagraph>, ShapedTextKeyHasher> cache_wrap;

protected:
	// Output.
	mutable TypedArray<RID> rids;
	mutable bool dirty_rids = true;

	// Fallbacks.
	static constexpr int MAX_FALLBACK_DEPTH = 64;
	TypedArray<Ref<Font>> fallbacks;

	static void _bind_methods();

	virtual void _update_rids_fb(const Font* p_f, int p_depth) const;
	virtual void _update_rids() const;
	virtual bool _is_cyclic(const Ref<Font>& p_f, int p_depth) const;

	void reset_state();

public:
	virtual void _invalidate_rids();

	static constexpr int DEFAULT_FONT_SIZE = 16;

	// Fallbacks.
	virtual void set_fallbacks(const TypedArray<Font> &p_fallbacks);
	virtual TypedArray<Ref<Font>> get_fallbacks() const;

	// Output.
	virtual RID find_variation(const VariationCoordinates&p_variation_coordinates, int p_face_index = 0, float p_strength = 0.0, Transform2D p_transform = Transform2D()) const { return RID(); };
	virtual RID _get_rid() const { return RID(); };
	virtual TypedArray<RID> get_rids() const;

	// Font metrics.
	virtual real_t get_height(int p_font_size) const;
	virtual real_t get_ascent(int p_font_size) const;
	virtual real_t get_descent(int p_font_size) const;
	virtual real_t get_underline_position(int p_font_size) const;
	virtual real_t get_underline_thickness(int p_font_size) const;

	virtual String get_font_name() const;
	virtual String get_font_style_name() const;
	virtual BitField<TextServer::FontStyle> get_font_style() const;
	virtual int get_font_weight() const;
	virtual int get_font_stretch() const;

	virtual int get_spacing(TextServer::SpacingType p_spacing) const { return 0; };
	virtual TextServerFeatures get_opentype_features() const;

	// Drawing string.
	virtual void set_cache_capacity(int p_single_line, int p_multi_line);

	virtual Size2 get_string_size(const String &p_text, HorizontalAlignment p_alignment = HORIZONTAL_ALIGNMENT_LEFT, float p_width = -1, int p_font_size = DEFAULT_FONT_SIZE, BitField<TextServer::JustificationFlag> p_jst_flags = TextServer::JUSTIFICATION_KASHIDA | TextServer::JUSTIFICATION_WORD_BOUND, TextServer::Direction p_direction = TextServer::DIRECTION_AUTO, TextServer::Orientation p_orientation = TextServer::ORIENTATION_HORIZONTAL) const;
	virtual Size2 get_multiline_string_size(const String &p_text, HorizontalAlignment p_alignment = HORIZONTAL_ALIGNMENT_LEFT, float p_width = -1, int p_font_size = DEFAULT_FONT_SIZE, int p_max_lines = -1, BitField<TextServer::LineBreakFlag> p_brk_flags = TextServer::BREAK_MANDATORY | TextServer::BREAK_WORD_BOUND, BitField<TextServer::JustificationFlag> p_jst_flags = TextServer::JUSTIFICATION_KASHIDA | TextServer::JUSTIFICATION_WORD_BOUND, TextServer::Direction p_direction = TextServer::DIRECTION_AUTO, TextServer::Orientation p_orientation = TextServer::ORIENTATION_HORIZONTAL) const;

	virtual void draw_string(RID p_canvas_item, const Point2 &p_pos, const String &p_text, HorizontalAlignment p_alignment = HORIZONTAL_ALIGNMENT_LEFT, float p_width = -1, int p_font_size = DEFAULT_FONT_SIZE, const Color &p_modulate = Color(1.0, 1.0, 1.0), BitField<TextServer::JustificationFlag> p_jst_flags = TextServer::JUSTIFICATION_KASHIDA | TextServer::JUSTIFICATION_WORD_BOUND, TextServer::Direction p_direction = TextServer::DIRECTION_AUTO, TextServer::Orientation p_orientation = TextServer::ORIENTATION_HORIZONTAL) const;
	virtual void draw_multiline_string(RID p_canvas_item, const Point2 &p_pos, const String &p_text, HorizontalAlignment p_alignment = HORIZONTAL_ALIGNMENT_LEFT, float p_width = -1, int p_font_size = DEFAULT_FONT_SIZE, int p_max_lines = -1, const Color &p_modulate = Color(1.0, 1.0, 1.0), BitField<TextServer::LineBreakFlag> p_brk_flags = TextServer::BREAK_MANDATORY | TextServer::BREAK_WORD_BOUND, BitField<TextServer::JustificationFlag> p_jst_flags = TextServer::JUSTIFICATION_KASHIDA | TextServer::JUSTIFICATION_WORD_BOUND, TextServer::Direction p_direction = TextServer::DIRECTION_AUTO, TextServer::Orientation p_orientation = TextServer::ORIENTATION_HORIZONTAL) const;

	virtual void draw_string_outline(RID p_canvas_item, const Point2 &p_pos, const String &p_text, HorizontalAlignment p_alignment = HORIZONTAL_ALIGNMENT_LEFT, float p_width = -1, int p_font_size = DEFAULT_FONT_SIZE, int p_size = 1, const Color &p_modulate = Color(1.0, 1.0, 1.0), BitField<TextServer::JustificationFlag> p_jst_flags = TextServer::JUSTIFICATION_KASHIDA | TextServer::JUSTIFICATION_WORD_BOUND, TextServer::Direction p_direction = TextServer::DIRECTION_AUTO, TextServer::Orientation p_orientation = TextServer::ORIENTATION_HORIZONTAL) const;
	virtual void draw_multiline_string_outline(RID p_canvas_item, const Point2 &p_pos, const String &p_text, HorizontalAlignment p_alignment = HORIZONTAL_ALIGNMENT_LEFT, float p_width = -1, int p_font_size = DEFAULT_FONT_SIZE, int p_max_lines = -1, int p_size = 1, const Color &p_modulate = Color(1.0, 1.0, 1.0), BitField<TextServer::LineBreakFlag> p_brk_flags = TextServer::BREAK_MANDATORY | TextServer::BREAK_WORD_BOUND, BitField<TextServer::JustificationFlag> p_jst_flags = TextServer::JUSTIFICATION_KASHIDA | TextServer::JUSTIFICATION_WORD_BOUND, TextServer::Direction p_direction = TextServer::DIRECTION_AUTO, TextServer::Orientation p_orientation = TextServer::ORIENTATION_HORIZONTAL) const;

	// Drawing char.
	virtual Size2 get_char_size(char32_t p_char, int p_font_size = DEFAULT_FONT_SIZE) const;
	virtual real_t draw_char(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, int p_font_size = DEFAULT_FONT_SIZE, const Color &p_modulate = Color(1.0, 1.0, 1.0)) const;
	virtual real_t draw_char_outline(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, int p_font_size = DEFAULT_FONT_SIZE, int p_size = 1, const Color &p_modulate = Color(1.0, 1.0, 1.0)) const;

	// Helper functions.
	virtual bool has_char(char32_t p_char) const;
	virtual String get_supported_chars() const;

	virtual bool is_language_supported(const String &p_language) const;
	virtual bool is_script_supported(const String &p_script) const;

	virtual TextServerFeatures get_supported_feature_list() const;
	virtual TextServerVariants get_supported_variation_list() const;
	virtual int64_t get_face_count() const;

	Font();
	~Font();
};

} // namespace godot
#endif // FONT_H
