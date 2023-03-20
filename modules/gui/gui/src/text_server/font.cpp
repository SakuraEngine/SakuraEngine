/**************************************************************************/
/*  font.cpp                                                              */
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

#include "text_server/font.h"
#include "text_server/text_line.h"
#include "text_server/text_paragraph.h"

namespace godot {
/*************************************************************************/
/*  Font                                                                 */
/*************************************************************************/

void Font::_bind_methods() {

}

void Font::_update_rids_fb(const Font* p_f, int p_depth) const {
	ERR_FAIL_COND(p_depth > MAX_FALLBACK_DEPTH);
	if (p_f) {
		RID rid = p_f->_get_rid();
		if (rid.is_valid()) {
			rids.push_back(rid);
		}
		const TypedArray<Ref<Font>>& _fallbacks = p_f->get_fallbacks();
		for (int i = 0; i < _fallbacks.size(); i++) {
			_update_rids_fb(_fallbacks[i].get(), p_depth + 1);
		}
	}
}

void Font::_update_rids() const {
	rids.clear();
	_update_rids_fb(const_cast<Font*>(this), 0);
	dirty_rids = false;
}

void Font::_invalidate_rids() {
	rids.clear();
	dirty_rids = true;

	cache.clear();
	cache_wrap.clear();

	// emit_changed();
}

bool Font::_is_cyclic(const Ref<Font> &p_f, int p_depth) const {
	ERR_FAIL_COND_V(p_depth > MAX_FALLBACK_DEPTH, true);
	if (p_f.is_null()) {
		return false;
	}
	if (p_f.get() == this) {
		return true;
	}
	for (int i = 0; i < p_f->fallbacks.size(); i++) {
		const Ref<Font> &f = p_f->fallbacks[i];
		if (_is_cyclic(f, p_depth + 1)) {
			return true;
		}
	}
	return false;
}

void Font::reset_state() {
	_invalidate_rids();
}

// Fallbacks.
void Font::set_fallbacks(const TypedArray<Font> &p_fallbacks) {
	SKR_UNIMPLEMENTED_FUNCTION();
	/*
	for (int i = 0; i < p_fallbacks.size(); i++) {
		const Ref<Font> &f = p_fallbacks[i];
		ERR_FAIL_COND_MSG(_is_cyclic(f, 0), "Cyclic font fallback.");
	}
	for (int i = 0; i < fallbacks.size(); i++) {
		Ref<Font> f = fallbacks[i];
		if (f.is_valid()) {
			f->disconnect(CoreStringNames::get_singleton()->changed, callable_mp(this, &Font::_invalidate_rids));
		}
	}
	fallbacks = p_fallbacks;
	for (int i = 0; i < fallbacks.size(); i++) {
		Ref<Font> f = fallbacks[i];
		if (f.is_valid()) {
			f->connect(CoreStringNames::get_singleton()->changed, callable_mp(this, &Font::_invalidate_rids), CONNECT_REFERENCE_COUNTED);
		}
	}
	_invalidate_rids();
	*/
}

TypedArray<Ref<Font>> Font::get_fallbacks() const {
	return fallbacks;
}

// Output.
TypedArray<RID> Font::get_rids() const {
	if (dirty_rids) {
		_update_rids();
	}
	return rids;
}

// Drawing string.
real_t Font::get_height(int p_font_size) const {
	if (dirty_rids) {
		_update_rids();
	}
	real_t ret = 0.f;
	for (int i = 0; i < rids.size(); i++) {
		ret = (real_t)MAX(ret, TS->font_get_ascent(rids[i], p_font_size) + TS->font_get_descent(rids[i], p_font_size));
	}
	return ret + get_spacing(TextServer::SPACING_BOTTOM) + get_spacing(TextServer::SPACING_TOP);
}

real_t Font::get_ascent(int p_font_size) const {
	if (dirty_rids) {
		_update_rids();
	}
	real_t ret = 0.f;
	for (int i = 0; i < rids.size(); i++) {
		ret = (real_t)MAX(ret, TS->font_get_ascent(rids[i], p_font_size));
	}
	return ret + get_spacing(TextServer::SPACING_TOP);
}

real_t Font::get_descent(int p_font_size) const {
	if (dirty_rids) {
		_update_rids();
	}
	real_t ret = 0.f;
	for (int i = 0; i < rids.size(); i++) {
		ret = (real_t)MAX(ret, TS->font_get_descent(rids[i], p_font_size));
	}
	return ret + get_spacing(TextServer::SPACING_BOTTOM);
}

real_t Font::get_underline_position(int p_font_size) const {
	if (dirty_rids) {
		_update_rids();
	}
	real_t ret = 0.f;
	for (int i = 0; i < rids.size(); i++) {
		ret = (real_t)MAX(ret, TS->font_get_underline_position(rids[i], p_font_size));
	}
	return ret + get_spacing(TextServer::SPACING_TOP);
}

real_t Font::get_underline_thickness(int p_font_size) const {
	if (dirty_rids) {
		_update_rids();
	}
	real_t ret = 0.f;
	for (int i = 0; i < rids.size(); i++) {
		ret = (real_t)MAX(ret, TS->font_get_underline_thickness(rids[i], p_font_size));
	}
	return ret;
}

String Font::get_font_name() const {
	return TS->font_get_name(_get_rid());
}

String Font::get_font_style_name() const {
	return TS->font_get_style_name(_get_rid());
}

BitField<TextServer::FontStyle> Font::get_font_style() const {
	return TS->font_get_style(_get_rid());
}

int Font::get_font_weight() const {
	return static_cast<int>(TS->font_get_weight(_get_rid()));
}

int Font::get_font_stretch() const {
	return static_cast<int>(TS->font_get_stretch(_get_rid()));
}

TextServerFeatures Font::get_opentype_features() const {
	return TextServerFeatures();
}

// Drawing string.
void Font::set_cache_capacity(int p_single_line, int p_multi_line) {
	cache.resize(p_single_line);
	cache_wrap.resize(p_multi_line);
}

Size2 Font::get_string_size(const String &p_text, HorizontalAlignment p_alignment, float p_width, int p_font_size, BitField<TextServer::JustificationFlag> p_jst_flags, TextServer::Direction p_direction, TextServer::Orientation p_orientation) const {
	bool fill = (p_alignment == HORIZONTAL_ALIGNMENT_FILL);
	ShapedTextKey key = ShapedTextKey(p_text, p_font_size, fill ? p_width : 0.0f, fill ? p_jst_flags : TextServer::JUSTIFICATION_NONE, TextServer::BREAK_NONE, p_direction, p_orientation);

	Ref<TextLine> buffer;
	if (cache.contains(key)) {
		buffer = cache.get(key);
	} else {
		buffer.instantiate();
		buffer->set_direction(p_direction);
		buffer->set_orientation(p_orientation);
		buffer->add_string(p_text, Ref<Font>(const_cast<Font*>(this)), p_font_size);
		cache.insert(key, buffer);
	}
	
	buffer->set_width(p_width);
	buffer->set_horizontal_alignment(p_alignment);
	if (p_alignment == HORIZONTAL_ALIGNMENT_FILL) {
		buffer->set_flags(p_jst_flags);
	}

	return buffer->get_size();
}

Size2 Font::get_multiline_string_size(const String &p_text, HorizontalAlignment p_alignment, float p_width, int p_font_size, int p_max_lines, BitField<TextServer::LineBreakFlag> p_brk_flags, BitField<TextServer::JustificationFlag> p_jst_flags, TextServer::Direction p_direction, TextServer::Orientation p_orientation) const {
	ShapedTextKey key = ShapedTextKey(p_text, p_font_size, p_width, p_jst_flags, p_brk_flags, p_direction, p_orientation);

	Ref<TextParagraph> lines_buffer;
	if (cache_wrap.contains(key)) {
		lines_buffer = cache_wrap.get(key);
	} else {
		lines_buffer.instantiate();
		lines_buffer->set_direction(p_direction);
		lines_buffer->set_orientation(p_orientation);
		lines_buffer->add_string(p_text, Ref<Font>(const_cast<Font*>(this)), p_font_size);
		lines_buffer->set_width(p_width);
		lines_buffer->set_break_flags(p_brk_flags);
		lines_buffer->set_justification_flags(p_jst_flags);
		cache_wrap.insert(key, lines_buffer);
	}

	lines_buffer->set_alignment(p_alignment);
	lines_buffer->set_max_lines_visible(p_max_lines);

	return lines_buffer->get_size();
}

void Font::draw_string(RID p_canvas_item, const Point2 &p_pos, const String &p_text, HorizontalAlignment p_alignment, float p_width, int p_font_size, const Color &p_modulate, BitField<TextServer::JustificationFlag> p_jst_flags, TextServer::Direction p_direction, TextServer::Orientation p_orientation) const {
	bool fill = (p_alignment == HORIZONTAL_ALIGNMENT_FILL);
	ShapedTextKey key = ShapedTextKey(p_text, p_font_size, fill ? p_width : 0.f, fill ? p_jst_flags : TextServer::JUSTIFICATION_NONE, TextServer::BREAK_NONE, p_direction, p_orientation);

	Ref<TextLine> buffer;
	if (cache.contains(key)) {
		buffer = cache.get(key);
	} else {
		buffer.instantiate();
		buffer->set_direction(p_direction);
		buffer->set_orientation(p_orientation);
		buffer->add_string(p_text, Ref<Font>(const_cast<Font*>(this)), p_font_size);
		cache.insert(key, buffer);
	}
	
	Vector2 ofs = p_pos;
	if (p_orientation == TextServer::ORIENTATION_HORIZONTAL) {
		ofs.y -= buffer->get_line_ascent();
	} else {
		ofs.x -= buffer->get_line_ascent();
	}

	buffer->set_width(p_width);
	buffer->set_horizontal_alignment(p_alignment);
	if (p_alignment == HORIZONTAL_ALIGNMENT_FILL) {
		buffer->set_flags(p_jst_flags);
	}

	buffer->draw(p_canvas_item, ofs, p_modulate);
}

void Font::draw_multiline_string(RID p_canvas_item, const Point2 &p_pos, const String &p_text, HorizontalAlignment p_alignment, float p_width, int p_font_size, int p_max_lines, const Color &p_modulate, BitField<TextServer::LineBreakFlag> p_brk_flags, BitField<TextServer::JustificationFlag> p_jst_flags, TextServer::Direction p_direction, TextServer::Orientation p_orientation) const {
	ShapedTextKey key = ShapedTextKey(p_text, p_font_size, p_width, p_jst_flags, p_brk_flags, p_direction, p_orientation);

	Ref<TextParagraph> lines_buffer;
	if (cache_wrap.contains(key)) {
		lines_buffer = cache_wrap.get(key);
	} else {
		lines_buffer.instantiate();
		lines_buffer->set_direction(p_direction);
		lines_buffer->set_orientation(p_orientation);
		lines_buffer->add_string(p_text, Ref<Font>(const_cast<Font*>(this)), p_font_size);
		lines_buffer->set_width(p_width);
		lines_buffer->set_break_flags(p_brk_flags);
		lines_buffer->set_justification_flags(p_jst_flags);
		cache_wrap.insert(key, lines_buffer);
	}

	Vector2 ofs = p_pos;
	if (p_orientation == TextServer::ORIENTATION_HORIZONTAL) {
		ofs.y -= lines_buffer->get_line_ascent(0);
	} else {
		ofs.x -= lines_buffer->get_line_ascent(0);
	}

	lines_buffer->set_alignment(p_alignment);
	lines_buffer->set_max_lines_visible(p_max_lines);

	lines_buffer->draw(p_canvas_item, ofs, p_modulate);
}

void Font::draw_string_outline(RID p_canvas_item, const Point2 &p_pos, const String &p_text, HorizontalAlignment p_alignment, float p_width, int p_font_size, int p_size, const Color &p_modulate, BitField<TextServer::JustificationFlag> p_jst_flags, TextServer::Direction p_direction, TextServer::Orientation p_orientation) const {
	bool fill = (p_alignment == HORIZONTAL_ALIGNMENT_FILL);
	ShapedTextKey key = ShapedTextKey(p_text, p_font_size, fill ? p_width : 0.f, fill ? p_jst_flags : TextServer::JUSTIFICATION_NONE, TextServer::BREAK_NONE, p_direction, p_orientation);

	Ref<TextLine> buffer;
	if (cache.contains(key)) {
		buffer = cache.get(key);
	} else {
		buffer.instantiate();
		buffer->set_direction(p_direction);
		buffer->set_orientation(p_orientation);
		buffer->add_string(p_text, Ref<Font>(const_cast<Font*>(this)), p_font_size);
		cache.insert(key, buffer);
	}

	Vector2 ofs = p_pos;
	if (p_orientation == TextServer::ORIENTATION_HORIZONTAL) {
		ofs.y -= buffer->get_line_ascent();
	} else {
		ofs.x -= buffer->get_line_ascent();
	}

	buffer->set_width(p_width);
	buffer->set_horizontal_alignment(p_alignment);
	if (p_alignment == HORIZONTAL_ALIGNMENT_FILL) {
		buffer->set_flags(p_jst_flags);
	}

	buffer->draw_outline(p_canvas_item, ofs, p_size, p_modulate);
}

void Font::draw_multiline_string_outline(RID p_canvas_item, const Point2 &p_pos, const String &p_text, HorizontalAlignment p_alignment, float p_width, int p_font_size, int p_max_lines, int p_size, const Color &p_modulate, BitField<TextServer::LineBreakFlag> p_brk_flags, BitField<TextServer::JustificationFlag> p_jst_flags, TextServer::Direction p_direction, TextServer::Orientation p_orientation) const {
	ShapedTextKey key = ShapedTextKey(p_text, p_font_size, p_width, p_jst_flags, p_brk_flags, p_direction, p_orientation);

	Ref<TextParagraph> lines_buffer;
	if (cache_wrap.contains(key)) {
		lines_buffer = cache_wrap.get(key);
	} else {
		lines_buffer.instantiate();
		lines_buffer->set_direction(p_direction);
		lines_buffer->set_orientation(p_orientation);
		lines_buffer->add_string(p_text, Ref<Font>(const_cast<Font*>(this)), p_font_size);
		lines_buffer->set_width(p_width);
		lines_buffer->set_break_flags(p_brk_flags);
		lines_buffer->set_justification_flags(p_jst_flags);
		cache_wrap.insert(key, lines_buffer);
	}

	Vector2 ofs = p_pos;
	if (p_orientation == TextServer::ORIENTATION_HORIZONTAL) {
		ofs.y -= lines_buffer->get_line_ascent(0);
	} else {
		ofs.x -= lines_buffer->get_line_ascent(0);
	}

	lines_buffer->set_alignment(p_alignment);
	lines_buffer->set_max_lines_visible(p_max_lines);

	lines_buffer->draw_outline(p_canvas_item, ofs, p_size, p_modulate);
}

// Drawing char.
Size2 Font::get_char_size(char32_t p_char, int p_font_size) const {
	if (dirty_rids) {
		_update_rids();
	}
	for (int i = 0; i < rids.size(); i++) {
		if (TS->font_has_char(rids[i], p_char)) {
			int32_t glyph = (int32_t)TS->font_get_glyph_index(rids[i], p_font_size, p_char, 0);
			return Size2(TS->font_get_glyph_advance(rids[i], p_font_size, glyph).x, get_height(p_font_size));
		}
	}
	return Size2();
}

real_t Font::draw_char(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, int p_font_size, const Color &p_modulate) const {
	if (dirty_rids) {
		_update_rids();
	}
	for (int i = 0; i < rids.size(); i++) {
		if (TS->font_has_char(rids[i], p_char)) {
			int32_t glyph = (int32_t)TS->font_get_glyph_index(rids[i], p_font_size, p_char, 0);
			TS->font_draw_glyph(rids[i], p_canvas_item, p_font_size, p_pos, glyph, p_modulate);
			return TS->font_get_glyph_advance(rids[i], p_font_size, glyph).x;
		}
	}
	return 0.f;
}

real_t Font::draw_char_outline(RID p_canvas_item, const Point2 &p_pos, char32_t p_char, int p_font_size, int p_size, const Color &p_modulate) const {
	if (dirty_rids) {
		_update_rids();
	}
	for (int i = 0; i < rids.size(); i++) {
		if (TS->font_has_char(rids[i], p_char)) {
			int32_t glyph = (int32_t)TS->font_get_glyph_index(rids[i], p_font_size, p_char, 0);
			TS->font_draw_glyph_outline(rids[i], p_canvas_item, p_font_size, p_size, p_pos, glyph, p_modulate);
			return TS->font_get_glyph_advance(rids[i], p_font_size, glyph).x;
		}
	}
	return 0.f;
}

// Helper functions.
bool Font::has_char(char32_t p_char) const {
	if (dirty_rids) {
		_update_rids();
	}
	for (int i = 0; i < rids.size(); i++) {
		if (TS->font_has_char(rids[i], p_char)) {
			return true;
		}
	}
	return false;
}

String Font::get_supported_chars() const {
	if (dirty_rids) {
		_update_rids();
	}
	String chars;
	for (int i = 0; i < rids.size(); i++) {
		String data_chars = TS->font_get_supported_chars(rids[i]);
		for (int j = 0; j < data_chars.length(); j++) {
			if (chars.find_char(data_chars[j]) == -1) {
				chars += data_chars[j];
			}
		}
	}
	return chars;
}

bool Font::is_language_supported(const String &p_language) const {
	return TS->font_is_language_supported(_get_rid(), p_language);
}

bool Font::is_script_supported(const String &p_script) const {
	return TS->font_is_script_supported(_get_rid(), p_script);
}

TextServerFeatures Font::get_supported_feature_list() const {
	return TS->font_supported_feature_list(_get_rid());
}

TextServerVariants Font::get_supported_variation_list() const {
	return TS->font_supported_variation_list(_get_rid());
}

int64_t Font::get_face_count() const {
	return TS->font_get_face_count(_get_rid());
}

Font::Font() {
	cache.resize(64);
	cache_wrap.resize(16);
}

Font::~Font() {
}

/*************************************************************************/
/*  FontFile                                                             */
/*************************************************************************/

_FORCE_INLINE_ void FontFile::_clear_cache() {
	for (int i = 0; i < cache.size(); i++) {
		if (cache[i].is_valid()) {
			TS->free_rid(cache[i]);
			cache[i] = RID();
		}
	}
}

_FORCE_INLINE_ void FontFile::_ensure_rid(int p_cache_index) const {
	if (unlikely(p_cache_index >= cache.size())) {
		cache.resize(p_cache_index + 1);
	}
	if (unlikely(!cache[p_cache_index].is_valid())) {
		cache[p_cache_index] = TS->create_font();
		TS->font_set_data_ptr(cache[p_cache_index], data_ptr, data_size);
		TS->font_set_antialiasing(cache[p_cache_index], antialiasing);
		TS->font_set_generate_mipmaps(cache[p_cache_index], mipmaps);
		TS->font_set_multichannel_signed_distance_field(cache[p_cache_index], msdf);
		TS->font_set_msdf_pixel_range(cache[p_cache_index], msdf_pixel_range);
		TS->font_set_msdf_size(cache[p_cache_index], msdf_size);
		TS->font_set_fixed_size(cache[p_cache_index], fixed_size);
		TS->font_set_force_autohinter(cache[p_cache_index], force_autohinter);
		TS->font_set_allow_system_fallback(cache[p_cache_index], allow_system_fallback);
		TS->font_set_hinting(cache[p_cache_index], hinting);
		TS->font_set_subpixel_positioning(cache[p_cache_index], subpixel_positioning);
		TS->font_set_oversampling(cache[p_cache_index], oversampling);
	}
}

void FontFile::_convert_packed_8bit(Ref<Image> &p_source, int p_page, int p_sz) {
	int w = p_source->get_width();
	int h = p_source->get_height();

	auto imgdata = p_source->get_data();
	const uint8_t *r = imgdata.data();

	PackedByteArray imgdata_r;
	imgdata_r.resize(w * h * 2);
	uint8_t *wr = imgdata_r.ptrw();

	PackedByteArray imgdata_g;
	imgdata_g.resize(w * h * 2);
	uint8_t *wg = imgdata_g.ptrw();

	PackedByteArray imgdata_b;
	imgdata_b.resize(w * h * 2);
	uint8_t *wb = imgdata_b.ptrw();

	PackedByteArray imgdata_a;
	imgdata_a.resize(w * h * 2);
	uint8_t *wa = imgdata_a.ptrw();

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int ofs_src = (i * w + j) * 4;
			int ofs_dst = (i * w + j) * 2;
			wr[ofs_dst + 0] = 255;
			wr[ofs_dst + 1] = r[ofs_src + 0];
			wg[ofs_dst + 0] = 255;
			wg[ofs_dst + 1] = r[ofs_src + 1];
			wb[ofs_dst + 0] = 255;
			wb[ofs_dst + 1] = r[ofs_src + 2];
			wa[ofs_dst + 0] = 255;
			wa[ofs_dst + 1] = r[ofs_src + 3];
		}
	}

	auto renderer = TS->get_gdi_renderer();
	Ref<Image> img_r = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_r);
	set_texture_image(0, Vector2i(p_sz, 0), p_page * 4 + 0, img_r);
	Ref<Image> img_g = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_g);
	set_texture_image(0, Vector2i(p_sz, 0), p_page * 4 + 1, img_g);
	Ref<Image> img_b = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_b);
	set_texture_image(0, Vector2i(p_sz, 0), p_page * 4 + 2, img_b);
	Ref<Image> img_a = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_a);
	set_texture_image(0, Vector2i(p_sz, 0), p_page * 4 + 3, img_a);
}

void FontFile::_convert_packed_4bit(Ref<Image> &p_source, int p_page, int p_sz) {
	int w = p_source->get_width();
	int h = p_source->get_height();

	auto imgdata = p_source->get_data();
	const uint8_t *r = imgdata.data();

	PackedByteArray imgdata_r;
	imgdata_r.resize(w * h * 2);
	uint8_t *wr = imgdata_r.ptrw();

	PackedByteArray imgdata_g;
	imgdata_g.resize(w * h * 2);
	uint8_t *wg = imgdata_g.ptrw();

	PackedByteArray imgdata_b;
	imgdata_b.resize(w * h * 2);
	uint8_t *wb = imgdata_b.ptrw();

	PackedByteArray imgdata_a;
	imgdata_a.resize(w * h * 2);
	uint8_t *wa = imgdata_a.ptrw();

	PackedByteArray imgdata_ro;
	imgdata_ro.resize(w * h * 2);
	uint8_t *wro = imgdata_ro.ptrw();

	PackedByteArray imgdata_go;
	imgdata_go.resize(w * h * 2);
	uint8_t *wgo = imgdata_go.ptrw();

	PackedByteArray imgdata_bo;
	imgdata_bo.resize(w * h * 2);
	uint8_t *wbo = imgdata_bo.ptrw();

	PackedByteArray imgdata_ao;
	imgdata_ao.resize(w * h * 2);
	uint8_t *wao = imgdata_ao.ptrw();

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int ofs_src = (i * w + j) * 4;
			int ofs_dst = (i * w + j) * 2;
			wr[ofs_dst + 0] = 255;
			wro[ofs_dst + 0] = 255;
			if (r[ofs_src + 0] > 0x0F) {
				wr[ofs_dst + 1] = (r[ofs_src + 0] - 0x0F) * 2;
				wro[ofs_dst + 1] = 0;
			} else {
				wr[ofs_dst + 1] = 0;
				wro[ofs_dst + 1] = r[ofs_src + 0] * 2;
			}
			wg[ofs_dst + 0] = 255;
			wgo[ofs_dst + 0] = 255;
			if (r[ofs_src + 1] > 0x0F) {
				wg[ofs_dst + 1] = (r[ofs_src + 1] - 0x0F) * 2;
				wgo[ofs_dst + 1] = 0;
			} else {
				wg[ofs_dst + 1] = 0;
				wgo[ofs_dst + 1] = r[ofs_src + 1] * 2;
			}
			wb[ofs_dst + 0] = 255;
			wbo[ofs_dst + 0] = 255;
			if (r[ofs_src + 2] > 0x0F) {
				wb[ofs_dst + 1] = (r[ofs_src + 2] - 0x0F) * 2;
				wbo[ofs_dst + 1] = 0;
			} else {
				wb[ofs_dst + 1] = 0;
				wbo[ofs_dst + 1] = r[ofs_src + 2] * 2;
			}
			wa[ofs_dst + 0] = 255;
			wao[ofs_dst + 0] = 255;
			if (r[ofs_src + 3] > 0x0F) {
				wa[ofs_dst + 1] = (r[ofs_src + 3] - 0x0F) * 2;
				wao[ofs_dst + 1] = 0;
			} else {
				wa[ofs_dst + 1] = 0;
				wao[ofs_dst + 1] = r[ofs_src + 3] * 2;
			}
		}
	}

	auto renderer = TS->get_gdi_renderer();
	Ref<Image> img_r = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_r);
	set_texture_image(0, Vector2i(p_sz, 0), p_page * 4 + 0, img_r);
	Ref<Image> img_g = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_g);
	set_texture_image(0, Vector2i(p_sz, 0), p_page * 4 + 1, img_g);
	Ref<Image> img_b = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_b);
	set_texture_image(0, Vector2i(p_sz, 0), p_page * 4 + 2, img_b);
	Ref<Image> img_a = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_a);
	set_texture_image(0, Vector2i(p_sz, 0), p_page * 4 + 3, img_a);

	Ref<Image> img_ro = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_ro);
	set_texture_image(0, Vector2i(p_sz, 1), p_page * 4 + 0, img_ro);
	Ref<Image> img_go = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_go);
	set_texture_image(0, Vector2i(p_sz, 1), p_page * 4 + 1, img_go);
	Ref<Image> img_bo = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_bo);
	set_texture_image(0, Vector2i(p_sz, 1), p_page * 4 + 2, img_bo);
	Ref<Image> img_ao = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_ao);
	set_texture_image(0, Vector2i(p_sz, 1), p_page * 4 + 3, img_ao);
}

void FontFile::_convert_rgba_4bit(Ref<Image> &p_source, int p_page, int p_sz) {
	int w = p_source->get_width();
	int h = p_source->get_height();

	auto imgdata = p_source->get_data();
	const uint8_t *r = imgdata.data();

	PackedByteArray imgdata_g;
	imgdata_g.resize(w * h * 4);
	uint8_t *wg = imgdata_g.ptrw();

	PackedByteArray imgdata_o;
	imgdata_o.resize(w * h * 4);
	uint8_t *wo = imgdata_o.ptrw();

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int ofs = (i * w + j) * 4;

			if (r[ofs + 0] > 0x7F) {
				wg[ofs + 0] = r[ofs + 0];
				wo[ofs + 0] = 0;
			} else {
				wg[ofs + 0] = 0;
				wo[ofs + 0] = r[ofs + 0] * 2;
			}
			if (r[ofs + 1] > 0x7F) {
				wg[ofs + 1] = r[ofs + 1];
				wo[ofs + 1] = 0;
			} else {
				wg[ofs + 1] = 0;
				wo[ofs + 1] = r[ofs + 1] * 2;
			}
			if (r[ofs + 2] > 0x7F) {
				wg[ofs + 2] = r[ofs + 2];
				wo[ofs + 2] = 0;
			} else {
				wg[ofs + 2] = 0;
				wo[ofs + 2] = r[ofs + 2] * 2;
			}
			if (r[ofs + 3] > 0x7F) {
				wg[ofs + 3] = r[ofs + 3];
				wo[ofs + 3] = 0;
			} else {
				wg[ofs + 3] = 0;
				wo[ofs + 3] = r[ofs + 3] * 2;
			}
		}
	}

	auto renderer = TS->get_gdi_renderer();
	Ref<Image> img_g = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_RGBA8, imgdata_g);
	set_texture_image(0, Vector2i(p_sz, 0), p_page, img_g);

	Ref<Image> img_o = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_RGBA8, imgdata_o);
	set_texture_image(0, Vector2i(p_sz, 1), p_page, img_o);
}

void FontFile::_convert_mono_8bit(Ref<Image> &p_source, int p_page, int p_ch, int p_sz, int p_ol) {
	int w = p_source->get_width();
	int h = p_source->get_height();

	auto imgdata = p_source->get_data();
	const uint8_t *r = imgdata.data();

	int size = 4;
	if (p_source->get_format() == Image::FORMAT_L8) {
		size = 1;
		p_ch = 0;
	}

	PackedByteArray imgdata_g;
	imgdata_g.resize(w * h * 2);
	uint8_t *wg = imgdata_g.ptrw();

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int ofs_src = (i * w + j) * size;
			int ofs_dst = (i * w + j) * 2;
			wg[ofs_dst + 0] = 255;
			wg[ofs_dst + 1] = r[ofs_src + p_ch];
		}
	}

	auto renderer = TS->get_gdi_renderer();
	Ref<Image> img_g = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_g);
	set_texture_image(0, Vector2i(p_sz, p_ol), p_page, img_g);
}

void FontFile::_convert_mono_4bit(Ref<Image> &p_source, int p_page, int p_ch, int p_sz, int p_ol) {
	int w = p_source->get_width();
	int h = p_source->get_height();

	auto imgdata = p_source->get_data();
	const uint8_t *r = imgdata.data();

	int size = 4;
	if (p_source->get_format() == Image::FORMAT_L8) {
		size = 1;
		p_ch = 0;
	}

	PackedByteArray imgdata_g;
	imgdata_g.resize(w * h * 2);
	uint8_t *wg = imgdata_g.ptrw();

	PackedByteArray imgdata_o;
	imgdata_o.resize(w * h * 2);
	uint8_t *wo = imgdata_o.ptrw();

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int ofs_src = (i * w + j) * size;
			int ofs_dst = (i * w + j) * 2;
			wg[ofs_dst + 0] = 255;
			wo[ofs_dst + 0] = 255;
			if (r[ofs_src + p_ch] > 0x7F) {
				wg[ofs_dst + 1] = r[ofs_src + p_ch];
				wo[ofs_dst + 1] = 0;
			} else {
				wg[ofs_dst + 1] = 0;
				wo[ofs_dst + 1] = r[ofs_src + p_ch] * 2;
			}
		}
	}

	auto renderer = TS->get_gdi_renderer();
	Ref<Image> img_g = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_g);
	set_texture_image(0, Vector2i(p_sz, 0), p_page, img_g);

	Ref<Image> img_o = Image::create_from_data(renderer, w, h, 0, Image::FORMAT_LA8, imgdata_o);
	set_texture_image(0, Vector2i(p_sz, p_ol), p_page, img_o);
}

bool FontFile::_set(const StringName &p_name, const Variant &p_value) {
	SKR_UNIMPLEMENTED_FUNCTION();
	return false;
}

bool FontFile::_get(const StringName &p_name, Variant &r_ret) const {
	SKR_UNIMPLEMENTED_FUNCTION();
	return false;
}

/*
void FontFile::_get_property_list(List<PropertyInfo> *p_list) const {
	SKR_UNIMPLEMENTED_FUNCTION();
}

void FontFile::reset_state() {
	_clear_cache();
	data.clear();
	data_ptr = nullptr;
	data_size = 0;
	cache.clear();

	antialiasing = TextServer::FONT_ANTIALIASING_GRAY;
	mipmaps = false;
	msdf = false;
	force_autohinter = false;
	allow_system_fallback = true;
	hinting = TextServer::HINTING_LIGHT;
	subpixel_positioning = TextServer::SUBPIXEL_POSITIONING_DISABLED;
	msdf_pixel_range = 14;
	msdf_size = 128;
	fixed_size = 0;
	oversampling = 0.f;

	Font::reset_state();
}
*/

/*************************************************************************/

// OEM encoding mapping for 0x80..0xFF range.
static const char32_t _oem_to_unicode[][129] = {
	U"\u20ac\ufffe\u201a\ufffe\u201e\u2026\u2020\u2021\ufffe\u2030\u0160\u2039\u015a\u0164\u017d\u0179\ufffe\u2018\u2019\u201c\u201d\u2022\u2013\u2014\ufffe\u2122\u0161\u203a\u015b\u0165\u017e\u017a\xa0\u02c7\u02d8\u0141\xa4\u0104\xa6\xa7\xa8\xa9\u015e\xab\xac\xad\xae\u017b\xb0\xb1\u02db\u0142\xb4\xb5\xb6\xb7\xb8\u0105\u015f\xbb\u013d\u02dd\u013e\u017c\u0154\xc1\xc2\u0102\xc4\u0139\u0106\xc7\u010c\xc9\u0118\xcb\u011a\xcd\xce\u010e\u0110\u0143\u0147\xd3\xd4\u0150\xd6\xd7\u0158\u016e\xda\u0170\xdc\xdd\u0162\xdf\u0155\xe1\xe2\u0103\xe4\u013a\u0107\xe7\u010d\xe9\u0119\xeb\u011b\xed\xee\u010f\u0111\u0144\u0148\xf3\xf4\u0151\xf6\xf7\u0159\u016f\xfa\u0171\xfc\xfd\u0163\u02d9", // 1250 - Latin 2
	U"\u0402\u0403\u201a\u0453\u201e\u2026\u2020\u2021\u20ac\u2030\u0409\u2039\u040a\u040c\u040b\u040f\u0452\u2018\u2019\u201c\u201d\u2022\u2013\u2014\ufffe\u2122\u0459\u203a\u045a\u045c\u045b\u045f\xa0\u040e\u045e\u0408\xa4\u0490\xa6\xa7\u0401\xa9\u0404\xab\xac\xad\xae\u0407\xb0\xb1\u0406\u0456\u0491\xb5\xb6\xb7\u0451\u2116\u0454\xbb\u0458\u0405\u0455\u0457\u0410\u0411\u0412\u0413\u0414\u0415\u0416\u0417\u0418\u0419\u041a\u041b\u041c\u041d\u041e\u041f\u0420\u0421\u0422\u0423\u0424\u0425\u0426\u0427\u0428\u0429\u042a\u042b\u042c\u042d\u042e\u042f\u0430\u0431\u0432\u0433\u0434\u0435\u0436\u0437\u0438\u0439\u043a\u043b\u043c\u043d\u043e\u043f\u0440\u0441\u0442\u0443\u0444\u0445\u0446\u0447\u0448\u0449\u044a\u044b\u044c\u044d\u044e\u044f", // 1251 - Cyrillic
	U"\u20ac\ufffe\u201a\u0192\u201e\u2026\u2020\u2021\u02c6\u2030\u0160\u2039\u0152\ufffe\u017d\ufffe\ufffe\u2018\u2019\u201c\u201d\u2022\u2013\u2014\u02dc\u2122\u0161\u203a\u0153\ufffe\u017e\u0178\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff", // 1252 - Latin 1
	U"\u20ac\ufffe\u201a\u0192\u201e\u2026\u2020\u2021\ufffe\u2030\ufffe\u2039\ufffe\ufffe\ufffe\ufffe\ufffe\u2018\u2019\u201c\u201d\u2022\u2013\u2014\ufffe\u2122\ufffe\u203a\ufffe\ufffe\ufffe\ufffe\xa0\u0385\u0386\xa3\xa4\xa5\xa6\xa7\xa8\xa9\ufffe\xab\xac\xad\xae\u2015\xb0\xb1\xb2\xb3\u0384\xb5\xb6\xb7\u0388\u0389\u038a\xbb\u038c\xbd\u038e\u038f\u0390\u0391\u0392\u0393\u0394\u0395\u0396\u0397\u0398\u0399\u039a\u039b\u039c\u039d\u039e\u039f\u03a0\u03a1\ufffe\u03a3\u03a4\u03a5\u03a6\u03a7\u03a8\u03a9\u03aa\u03ab\u03ac\u03ad\u03ae\u03af\u03b0\u03b1\u03b2\u03b3\u03b4\u03b5\u03b6\u03b7\u03b8\u03b9\u03ba\u03bb\u03bc\u03bd\u03be\u03bf\u03c0\u03c1\u03c2\u03c3\u03c4\u03c5\u03c6\u03c7\u03c8\u03c9\u03ca\u03cb\u03cc\u03cd\u03ce\ufffe", // 1253 - Greek
	U"\u20ac\ufffe\u201a\u0192\u201e\u2026\u2020\u2021\u02c6\u2030\u0160\u2039\u0152\ufffe\ufffe\ufffe\ufffe\u2018\u2019\u201c\u201d\u2022\u2013\u2014\u02dc\u2122\u0161\u203a\u0153\ufffe\ufffe\u0178\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\u011e\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\u0130\u015e\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\u011f\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\u0131\u015f\xff", // 1254 - Turkish
	U"\u20ac\ufffe\u201a\u0192\u201e\u2026\u2020\u2021\u02c6\u2030\ufffe\u2039\ufffe\ufffe\ufffe\ufffe\ufffe\u2018\u2019\u201c\u201d\u2022\u2013\u2014\u02dc\u2122\ufffe\u203a\ufffe\ufffe\ufffe\ufffe\xa0\xa1\xa2\xa3\u20aa\xa5\xa6\xa7\xa8\xa9\xd7\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xf7\xbb\xbc\xbd\xbe\xbf\u05b0\u05b1\u05b2\u05b3\u05b4\u05b5\u05b6\u05b7\u05b8\u05b9\ufffe\u05bb\u05bc\u05bd\u05be\u05bf\u05c0\u05c1\u05c2\u05c3\u05f0\u05f1\u05f2\u05f3\u05f4\ufffe\ufffe\ufffe\ufffe\ufffe\ufffe\ufffe\u05d0\u05d1\u05d2\u05d3\u05d4\u05d5\u05d6\u05d7\u05d8\u05d9\u05da\u05db\u05dc\u05dd\u05de\u05df\u05e0\u05e1\u05e2\u05e3\u05e4\u05e5\u05e6\u05e7\u05e8\u05e9\u05ea\ufffe\ufffe\u200e\u200f\ufffe", // 1255 - Hebrew
	U"\u20ac\u067e\u201a\u0192\u201e\u2026\u2020\u2021\u02c6\u2030\u0679\u2039\u0152\u0686\u0698\u0688\u06af\u2018\u2019\u201c\u201d\u2022\u2013\u2014\u06a9\u2122\u0691\u203a\u0153\u200c\u200d\u06ba\xa0\u060c\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\u06be\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\u061b\xbb\xbc\xbd\xbe\u061f\u06c1\u0621\u0622\u0623\u0624\u0625\u0626\u0627\u0628\u0629\u062a\u062b\u062c\u062d\u062e\u062f\u0630\u0631\u0632\u0633\u0634\u0635\u0636\xd7\u0637\u0638\u0639\u063a\u0640\u0641\u0642\u0643\xe0\u0644\xe2\u0645\u0646\u0647\u0648\xe7\xe8\xe9\xea\xeb\u0649\u064a\xee\xef\u064b\u064c\u064d\u064e\xf4\u064f\u0650\xf7\u0651\xf9\u0652\xfb\xfc\u200e\u200f\u06d2", // 1256 - Arabic
	U"\u20ac\ufffe\u201a\ufffe\u201e\u2026\u2020\u2021\ufffe\u2030\ufffe\u2039\ufffe\xa8\u02c7\xb8\ufffe\u2018\u2019\u201c\u201d\u2022\u2013\u2014\ufffe\u2122\ufffe\u203a\ufffe\xaf\u02db\ufffe\xa0\ufffe\xa2\xa3\xa4\ufffe\xa6\xa7\xd8\xa9\u0156\xab\xac\xad\xae\xc6\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xf8\xb9\u0157\xbb\xbc\xbd\xbe\xe6\u0104\u012e\u0100\u0106\xc4\xc5\u0118\u0112\u010c\xc9\u0179\u0116\u0122\u0136\u012a\u013b\u0160\u0143\u0145\xd3\u014c\xd5\xd6\xd7\u0172\u0141\u015a\u016a\xdc\u017b\u017d\xdf\u0105\u012f\u0101\u0107\xe4\xe5\u0119\u0113\u010d\xe9\u017a\u0117\u0123\u0137\u012b\u013c\u0161\u0144\u0146\xf3\u014d\xf5\xf6\xf7\u0173\u0142\u015b\u016b\xfc\u017c\u017e\u02d9", // 1257 - Baltic
	U"\u20ac\ufffe\u201a\u0192\u201e\u2026\u2020\u2021\u02c6\u2030\ufffe\u2039\u0152\ufffe\ufffe\ufffe\ufffe\u2018\u2019\u201c\u201d\u2022\u2013\u2014\u02dc\u2122\ufffe\u203a\u0153\ufffe\ufffe\u0178\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\u0102\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\u0300\xcd\xce\xcf\u0110\xd1\u0309\xd3\xd4\u01a0\xd6\xd7\xd8\xd9\xda\xdb\xdc\u01af\u0303\xdf\xe0\xe1\xe2\u0103\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\u0301\xed\xee\xef\u0111\xf1\u0323\xf3\xf4\u01a1\xf6\xf7\xf8\xf9\xfa\xfb\xfc\u01b0\u20ab\xff", // 1258 - Vietnamese
};

/*
Error FontFile::load_bitmap_font(const String &p_path) {
	reset_state();

	antialiasing = TextServer::FONT_ANTIALIASING_NONE;
	mipmaps = false;
	msdf = false;
	force_autohinter = false;
	allow_system_fallback = true;
	hinting = TextServer::HINTING_NONE;
	oversampling = 1.0f;

	Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(f.is_null(), ERR_CANT_CREATE, vformat(RTR("Cannot open font from file: %s."), p_path));

	int base_size = 16;
	int height = 0;
	int ascent = 0;
	int outline = 0;
	BitField<TextServer::FontStyle> st_flags = 0;
	String font_name;

	bool packed = false;
	uint8_t ch[4] = { 0, 0, 0, 0 }; // RGBA
	int first_gl_ch = -1;
	int first_ol_ch = -1;
	int first_cm_ch = -1;

	unsigned char magic[4];
	f->get_buffer((unsigned char *)&magic, 4);
	if (magic[0] == 'B' && magic[1] == 'M' && magic[2] == 'F') {
		// Binary BMFont file.
		ERR_FAIL_COND_V_MSG(magic[3] != 3, ERR_CANT_CREATE, vformat(RTR("Version %d of BMFont is not supported (should be 3)."), (int)magic[3]));

		uint8_t block_type = f->get_8();
		uint32_t block_size = f->get_32();
		bool unicode = false;
		uint8_t encoding = 9;
		while (!f->eof_reached()) {
			uint64_t off = f->get_position();
			switch (block_type) {
				case 1: { // info
					ERR_FAIL_COND_V_MSG(block_size < 15, ERR_CANT_CREATE, RTR("Invalid BMFont info block size."));
					base_size = f->get_16();
					if (base_size <= 0) {
						base_size = 16;
					}
					uint8_t flags = f->get_8();
					if (flags & (1 << 3)) {
						st_flags.set_flag(TextServer::FONT_BOLD);
					}
					if (flags & (1 << 2)) {
						st_flags.set_flag(TextServer::FONT_ITALIC);
					}
					unicode = (flags & 0x02);
					uint8_t encoding_id = f->get_8(); // non-unicode charset
					if (!unicode) {
						switch (encoding_id) {
							case 0x00: {
								encoding = 2;
							} break;
							case 0xB2: {
								encoding = 6;
							} break;
							case 0xBA: {
								encoding = 7;
							} break;
							case 0xEE: {
								encoding = 0;
							} break;
							case 0xA1: {
								encoding = 3;
							} break;
							case 0xB1: {
								encoding = 5;
							} break;
							case 0xCC: {
								encoding = 1;
							} break;
							case 0xA2: {
								encoding = 4;
							} break;
							case 0xA3: {
								encoding = 8;
							} break;
							default: {
								WARN_PRINT(vformat("Unknown BMFont OEM encoding %x, parsing as Unicode (should be 0x00 - Latin 1, 0xB2 - Arabic, 0xBA - Baltic, 0xEE - Latin 2, 0xA1 - Greek, 0xB1 - Hebrew, 0xCC - Cyrillic, 0xA2 - Turkish, 0xA3 - Vietnamese).", encoding_id));
							} break;
						};
					}
					f->get_16(); // stretch_h, skip
					f->get_8(); // aa, skip
					f->get_32(); // padding, skip
					f->get_16(); // spacing, skip
					outline = f->get_8();
					// font name
					PackedByteArray name_data;
					name_data.resize(block_size - 14);
					f->get_buffer(name_data.ptrw(), block_size - 14);
					font_name = String::utf8((const char *)name_data.ptr(), block_size - 14);
					set_fixed_size(base_size);
				} break;
				case 2:  { // common
					ERR_FAIL_COND_V_MSG(block_size != 15, ERR_CANT_CREATE, RTR("Invalid BMFont common block size."));
					height = f->get_16();
					ascent = f->get_16();
					f->get_32(); // scale, skip
					f->get_16(); // pages, skip
					uint8_t flags = f->get_8();
					packed = (flags & 0x01);
					ch[3] = f->get_8();
					ch[0] = f->get_8();
					ch[1] = f->get_8();
					ch[2] = f->get_8();
					for (int i = 0; i < 4; i++) {
						if (ch[i] == 0 && first_gl_ch == -1) {
							first_gl_ch = i;
						}
						if (ch[i] == 1 && first_ol_ch == -1) {
							first_ol_ch = i;
							if (outline == 0) {
								outline = 1;
							}
						}
						if (ch[i] == 2 && first_cm_ch == -1) {
							first_cm_ch = i;
						}
					}
				} break;
				case 3: { // pages
					int page = 0;
					CharString cs;
					char32_t c = f->get_8();
					while (!f->eof_reached() && f->get_position() <= off + block_size) {
						if (c == '\0') {
							String base_dir = p_path.get_base_dir();
							String file = base_dir.path_join(String::utf8(cs.ptr(), cs.length()));
							if (RenderingServer::get_singleton() != nullptr) {
								Ref<Image> img;
								img.instantiate();
								Error err = ImageLoader::load_image(file, img);
								ERR_FAIL_COND_V_MSG(err != OK, ERR_FILE_CANT_READ, vformat(RTR("Can't load font texture: %s."), file));

								if (packed) {
									if (ch[3] == 0) { // 4 x 8 bit monochrome, no outline
										outline = 0;
										ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
										_convert_packed_8bit(img, page, base_size);
									} else if ((ch[3] == 2) && (outline > 0)) { // 4 x 4 bit monochrome, gl + outline
										ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
										_convert_packed_4bit(img, page, base_size);
									} else {
										ERR_FAIL_V_MSG(ERR_CANT_CREATE, RTR("Unsupported BMFont texture format."));
									}
								} else {
									if ((ch[0] == 0) && (ch[1] == 0) && (ch[2] == 0) && (ch[3] == 0)) { // RGBA8 color, no outline
										outline = 0;
										ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
										set_texture_image(0, Vector2i(base_size, 0), page, img);
									} else if ((ch[0] == 2) && (ch[1] == 2) && (ch[2] == 2) && (ch[3] == 2) && (outline > 0)) { // RGBA4 color, gl + outline
										ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
										_convert_rgba_4bit(img, page, base_size);
									} else if ((first_gl_ch >= 0) && (first_ol_ch >= 0) && (outline > 0)) { // 1 x 8 bit monochrome, gl + outline
										ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8 && img->get_format() != Image::FORMAT_L8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
										_convert_mono_8bit(img, page, first_gl_ch, base_size, 0);
										_convert_mono_8bit(img, page, first_ol_ch, base_size, 1);
									} else if ((first_cm_ch >= 0) && (outline > 0)) { // 1 x 4 bit monochrome, gl + outline
										ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8 && img->get_format() != Image::FORMAT_L8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
										_convert_mono_4bit(img, page, first_cm_ch, base_size, 1);
									} else if (first_gl_ch >= 0) { // 1 x 8 bit monochrome, no outline
										outline = 0;
										ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8 && img->get_format() != Image::FORMAT_L8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
										_convert_mono_8bit(img, page, first_gl_ch, base_size, 0);
									} else {
										ERR_FAIL_V_MSG(ERR_CANT_CREATE, RTR("Unsupported BMFont texture format."));
									}
								}
							}
							page++;
							cs = "";
						} else {
							cs += c;
						}
						c = f->get_8();
					}
				} break;
				case 4: { // chars
					int char_count = block_size / 20;
					for (int i = 0; i < char_count; i++) {
						Vector2 advance;
						Vector2 size;
						Vector2 offset;
						Rect2 uv_rect;

						char32_t idx = f->get_32();
						if (!unicode && encoding < 9) {
							if (idx >= 0x80 && idx <= 0xFF) {
								idx = _oem_to_unicode[encoding][idx - 0x80];
							} else if (idx > 0xFF) {
								WARN_PRINT(vformat("Invalid BMFont OEM character %x (should be 0x00-0xFF).", idx));
								idx = 0x00;
							}
						}
						uv_rect.position.x = (int16_t)f->get_16();
						uv_rect.position.y = (int16_t)f->get_16();
						uv_rect.size.width = (int16_t)f->get_16();
						size.width = uv_rect.size.width;
						uv_rect.size.height = (int16_t)f->get_16();
						size.height = uv_rect.size.height;
						offset.x = (int16_t)f->get_16();
						offset.y = (int16_t)f->get_16() - ascent;
						advance.x = (int16_t)f->get_16();
						if (advance.x < 0) {
							advance.x = size.width + 1;
						}

						int texture_idx = f->get_8();
						uint8_t channel = f->get_8();

						int ch_off = 0;
						if (packed) {
							switch (channel) {
								case 1:
									ch_off = 2;
									break; // B
								case 2:
									ch_off = 1;
									break; // G
								case 4:
									ch_off = 0;
									break; // R
								case 8:
									ch_off = 3;
									break; // A
								default:
									ch_off = 0;
									break;
							}
						}
						set_glyph_advance(0, base_size, idx, advance);
						set_glyph_offset(0, Vector2i(base_size, 0), idx, offset);
						set_glyph_size(0, Vector2i(base_size, 0), idx, size);
						set_glyph_uv_rect(0, Vector2i(base_size, 0), idx, uv_rect);
						set_glyph_texture_idx(0, Vector2i(base_size, 0), idx, texture_idx * (packed ? 4 : 1) + ch_off);
						if (outline > 0) {
							set_glyph_offset(0, Vector2i(base_size, 1), idx, offset);
							set_glyph_size(0, Vector2i(base_size, 1), idx, size);
							set_glyph_uv_rect(0, Vector2i(base_size, 1), idx, uv_rect);
							set_glyph_texture_idx(0, Vector2i(base_size, 1), idx, texture_idx * (packed ? 4 : 1) + ch_off);
						}
					}
				} break;
				case 5: { // kerning
					int pair_count = block_size / 10;
					for (int i = 0; i < pair_count; i++) {
						Vector2i kpk;
						kpk.x = f->get_32();
						kpk.y = f->get_32();
						if (!unicode && encoding < 9) {
							if (kpk.x >= 0x80 && kpk.x <= 0xFF) {
								kpk.x = _oem_to_unicode[encoding][kpk.x - 0x80];
							} else if (kpk.x > 0xFF) {
								WARN_PRINT(vformat("Invalid BMFont OEM character %x (should be 0x00-0xFF).", kpk.x));
								kpk.x = 0x00;
							}
							if (kpk.y >= 0x80 && kpk.y <= 0xFF) {
								kpk.y = _oem_to_unicode[encoding][kpk.y - 0x80];
							} else if (kpk.y > 0xFF) {
								WARN_PRINT(vformat("Invalid BMFont OEM character %x (should be 0x00-0xFF).", kpk.y));
								kpk.y = 0x00;
							}
						}
						set_kerning(0, base_size, kpk, Vector2((int16_t)f->get_16(), 0));
					}
				} break;
				default: {
					ERR_FAIL_V_MSG(ERR_CANT_CREATE, RTR("Invalid BMFont block type."));
				} break;
			}
			f->seek(off + block_size);
			block_type = f->get_8();
			block_size = f->get_32();
		}

	} else {
		// Text BMFont file.
		f->seek(0);
		bool unicode = false;
		uint8_t encoding = 9;
		while (true) {
			String line = f->get_line();

			int delimiter = line.find(" ");
			String type = line.substr(0, delimiter);
			int pos = delimiter + 1;
			HashMap<String, String> keys;

			while (pos < line.size() && line[pos] == ' ') {
				pos++;
			}

			while (pos < line.size()) {
				int eq = line.find("=", pos);
				if (eq == -1) {
					break;
				}
				String key = line.substr(pos, eq - pos);
				int end = -1;
				String value;
				if (line[eq + 1] == '"') {
					end = line.find("\"", eq + 2);
					if (end == -1) {
						break;
					}
					value = line.substr(eq + 2, end - 1 - eq - 1);
					pos = end + 1;
				} else {
					end = line.find(" ", eq + 1);
					if (end == -1) {
						end = line.size();
					}
					value = line.substr(eq + 1, end - eq);
					pos = end;
				}

				while (pos < line.size() && line[pos] == ' ') {
					pos++;
				}

				keys[key] = value;
			}

			if (type == "info") {
				if (keys.has("size")) {
					base_size = keys["size"].to_int();
				}
				if (keys.has("outline")) {
					outline = keys["outline"].to_int();
				}
				if (keys.has("bold")) {
					if (keys["bold"].to_int()) {
						st_flags.set_flag(TextServer::FONT_BOLD);
					}
				}
				if (keys.has("italic")) {
					if (keys["italic"].to_int()) {
						st_flags.set_flag(TextServer::FONT_ITALIC);
					}
				}
				if (keys.has("face")) {
					font_name = keys["face"];
				}
				if (keys.has("unicode")) {
					unicode = keys["unicode"].to_int();
				}
				if (!unicode) {
					if (keys.has("charset")) {
						String encoding_name = keys["charset"].to_upper();
						if (encoding_name == "" || encoding_name == "ASCII" || encoding_name == "ANSI") {
							encoding = 2;
						} else if (encoding_name == "ARABIC") {
							encoding = 6;
						} else if (encoding_name == "BALTIC") {
							encoding = 7;
						} else if (encoding_name == "EASTEUROPE") {
							encoding = 0;
						} else if (encoding_name == "GREEK") {
							encoding = 3;
						} else if (encoding_name == "HEBREW") {
							encoding = 5;
						} else if (encoding_name == "RUSSIAN") {
							encoding = 1;
						} else if (encoding_name == "TURKISH") {
							encoding = 4;
						} else if (encoding_name == "VIETNAMESE") {
							encoding = 8;
						} else {
							WARN_PRINT(vformat("Unknown BMFont OEM encoding %s, parsing as Unicode (should be ANSI, ASCII, ARABIC, BALTIC, EASTEUROPE, GREEK, HEBREW, RUSSIAN, TURKISH or VIETNAMESE).", encoding_name));
						}
					} else {
						encoding = 2;
					}
				}
				set_fixed_size(base_size);
			} else if (type == "common") {
				if (keys.has("lineHeight")) {
					height = keys["lineHeight"].to_int();
				}
				if (keys.has("base")) {
					ascent = keys["base"].to_int();
				}
				if (keys.has("packed")) {
					packed = (keys["packed"].to_int() == 1);
				}
				if (keys.has("alphaChnl")) {
					ch[3] = keys["alphaChnl"].to_int();
				}
				if (keys.has("redChnl")) {
					ch[0] = keys["redChnl"].to_int();
				}
				if (keys.has("greenChnl")) {
					ch[1] = keys["greenChnl"].to_int();
				}
				if (keys.has("blueChnl")) {
					ch[2] = keys["blueChnl"].to_int();
				}
				for (int i = 0; i < 4; i++) {
					if (ch[i] == 0 && first_gl_ch == -1) {
						first_gl_ch = i;
					}
					if (ch[i] == 1 && first_ol_ch == -1) {
						first_ol_ch = i;
						if (outline == 0) {
							outline = 1;
						}
					}
					if (ch[i] == 2 && first_cm_ch == -1) {
						first_cm_ch = i;
					}
				}
			} else if (type == "page") {
				int page = 0;
				if (keys.has("id")) {
					page = keys["id"].to_int();
				}
				if (keys.has("file")) {
					String base_dir = p_path.get_base_dir();
					String file = base_dir.path_join(keys["file"]);
					if (RenderingServer::get_singleton() != nullptr) {
						Ref<Image> img;
						img.instantiate();
						Error err = ImageLoader::load_image(file, img);
						ERR_FAIL_COND_V_MSG(err != OK, ERR_FILE_CANT_READ, vformat(RTR("Can't load font texture: %s."), file));
						if (packed) {
							if (ch[3] == 0) { // 4 x 8 bit monochrome, no outline
								outline = 0;
								ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
								_convert_packed_8bit(img, page, base_size);
							} else if ((ch[3] == 2) && (outline > 0)) { // 4 x 4 bit monochrome, gl + outline
								ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
								_convert_packed_4bit(img, page, base_size);
							} else {
								ERR_FAIL_V_MSG(ERR_CANT_CREATE, RTR("Unsupported BMFont texture format."));
							}
						} else {
							if ((ch[3] == 0) && (ch[0] == 4) && (ch[1] == 4) && (ch[2] == 4) && img->get_format() == Image::FORMAT_RGBA8) { // might be RGBA8 color, no outline (color part of the image should be sold white, but some apps designed for Godot 3 generate color fonts with this config)
								outline = 0;
								set_texture_image(0, Vector2i(base_size, 0), page, img);
							} else if ((ch[0] == 0) && (ch[1] == 0) && (ch[2] == 0) && (ch[3] == 0)) { // RGBA8 color, no outline
								outline = 0;
								ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
								set_texture_image(0, Vector2i(base_size, 0), page, img);
							} else if ((ch[0] == 2) && (ch[1] == 2) && (ch[2] == 2) && (ch[3] == 2) && (outline > 0)) { // RGBA4 color, gl + outline
								ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
								_convert_rgba_4bit(img, page, base_size);
							} else if ((first_gl_ch >= 0) && (first_ol_ch >= 0) && (outline > 0)) { // 1 x 8 bit monochrome, gl + outline
								ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8 && img->get_format() != Image::FORMAT_L8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
								_convert_mono_8bit(img, page, first_gl_ch, base_size, 0);
								_convert_mono_8bit(img, page, first_ol_ch, base_size, 1);
							} else if ((first_cm_ch >= 0) && (outline > 0)) { // 1 x 4 bit monochrome, gl + outline
								ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8 && img->get_format() != Image::FORMAT_L8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
								_convert_mono_4bit(img, page, first_cm_ch, base_size, 1);
							} else if (first_gl_ch >= 0) { // 1 x 8 bit monochrome, no outline
								outline = 0;
								ERR_FAIL_COND_V_MSG(img->get_format() != Image::FORMAT_RGBA8 && img->get_format() != Image::FORMAT_L8, ERR_FILE_CANT_READ, RTR("Unsupported BMFont texture format."));
								_convert_mono_8bit(img, page, first_gl_ch, base_size, 0);
							} else {
								ERR_FAIL_V_MSG(ERR_CANT_CREATE, RTR("Unsupported BMFont texture format."));
							}
						}
					}
				}
			} else if (type == "char") {
				char32_t idx = 0;
				Vector2 advance;
				Vector2 size;
				Vector2 offset;
				Rect2 uv_rect;
				int texture_idx = -1;
				uint8_t channel = 15;

				if (keys.has("id")) {
					idx = keys["id"].to_int();
					if (!unicode && encoding < 9) {
						if (idx >= 0x80 && idx <= 0xFF) {
							idx = _oem_to_unicode[encoding][idx - 0x80];
						} else if (idx > 0xFF) {
							WARN_PRINT(vformat("Invalid BMFont OEM character %x (should be 0x00-0xFF).", idx));
							idx = 0x00;
						}
					}
				}
				if (keys.has("x")) {
					uv_rect.position.x = keys["x"].to_int();
				}
				if (keys.has("y")) {
					uv_rect.position.y = keys["y"].to_int();
				}
				if (keys.has("width")) {
					uv_rect.size.width = keys["width"].to_int();
					size.width = keys["width"].to_int();
				}
				if (keys.has("height")) {
					uv_rect.size.height = keys["height"].to_int();
					size.height = keys["height"].to_int();
				}
				if (keys.has("xoffset")) {
					offset.x = keys["xoffset"].to_int();
				}
				if (keys.has("yoffset")) {
					offset.y = keys["yoffset"].to_int() - ascent;
				}
				if (keys.has("page")) {
					texture_idx = keys["page"].to_int();
				}
				if (keys.has("xadvance")) {
					advance.x = keys["xadvance"].to_int();
				}
				if (advance.x < 0) {
					advance.x = size.width + 1;
				}
				if (keys.has("chnl")) {
					channel = keys["chnl"].to_int();
				}

				int ch_off = 0;
				if (packed) {
					switch (channel) {
						case 1:
							ch_off = 2;
							break; // B
						case 2:
							ch_off = 1;
							break; // G
						case 4:
							ch_off = 0;
							break; // R
						case 8:
							ch_off = 3;
							break; // A
						default:
							ch_off = 0;
							break;
					}
				}
				set_glyph_advance(0, base_size, idx, advance);
				set_glyph_offset(0, Vector2i(base_size, 0), idx, offset);
				set_glyph_size(0, Vector2i(base_size, 0), idx, size);
				set_glyph_uv_rect(0, Vector2i(base_size, 0), idx, uv_rect);
				set_glyph_texture_idx(0, Vector2i(base_size, 0), idx, texture_idx * (packed ? 4 : 1) + ch_off);
				if (outline > 0) {
					set_glyph_offset(0, Vector2i(base_size, 1), idx, offset);
					set_glyph_size(0, Vector2i(base_size, 1), idx, size);
					set_glyph_uv_rect(0, Vector2i(base_size, 1), idx, uv_rect);
					set_glyph_texture_idx(0, Vector2i(base_size, 1), idx, texture_idx * (packed ? 4 : 1) + ch_off);
				}
			} else if (type == "kerning") {
				Vector2i kpk;
				if (keys.has("first")) {
					kpk.x = keys["first"].to_int();
				}
				if (keys.has("second")) {
					kpk.y = keys["second"].to_int();
				}
				if (!unicode && encoding < 9) {
					if (kpk.x >= 0x80 && kpk.x <= 0xFF) {
						kpk.x = _oem_to_unicode[encoding][kpk.x - 0x80];
					} else if (kpk.x > 0xFF) {
						WARN_PRINT(vformat("Invalid BMFont OEM character %x (should be 0x00-0xFF).", kpk.x));
						kpk.x = 0x00;
					}
					if (kpk.y >= 0x80 && kpk.y <= 0xFF) {
						kpk.y = _oem_to_unicode[encoding][kpk.y - 0x80];
					} else if (kpk.y > 0xFF) {
						WARN_PRINT(vformat("Invalid BMFont OEM character %x (should be 0x00-0xFF).", kpk.x));
						kpk.y = 0x00;
					}
				}
				if (keys.has("amount")) {
					set_kerning(0, base_size, kpk, Vector2(keys["amount"].to_int(), 0));
				}
			}

			if (f->eof_reached()) {
				break;
			}
		}
	}

	set_font_name(font_name);
	set_font_style(st_flags);
	if (st_flags & TextServer::FONT_BOLD) {
		set_font_weight(700);
	}
	set_cache_ascent(0, base_size, ascent);
	set_cache_descent(0, base_size, height - ascent);

	return OK;
}

Error FontFile::load_dynamic_font(const String &p_path) {
	reset_state();

	Vector<uint8_t> font_data = FileAccess::get_file_as_bytes(p_path);
	set_data(font_data);

	return OK;
}
*/

void FontFile::set_data_ptr(const uint8_t *p_data, size_t p_size) {
	data.clear();
	data_ptr = p_data;
	data_size = p_size;

	for (int i = 0; i < cache.size(); i++) {
		if (cache[i].is_valid()) {
			TS->font_set_data_ptr(cache[i], data_ptr, data_size);
		}
	}
}

void FontFile::set_data(const PackedByteArray &p_data) {
	data = p_data;
	data_ptr = data.ptr();
	data_size = data.size();

	for (int i = 0; i < cache.size(); i++) {
		if (cache[i].is_valid()) {
			TS->font_set_data_ptr(cache[i], data_ptr, data_size);
		}
	}
}

PackedByteArray FontFile::get_data() const {
	if (unlikely((size_t)data.size() != data_size)) {
		PackedByteArray *data_w = const_cast<PackedByteArray *>(&data);
		data_w->resize(data_size);
		memcpy(data_w->ptrw(), data_ptr, data_size);
	}
	return data;
}

void FontFile::set_font_name(const String &p_name) {
	_ensure_rid(0);
	TS->font_set_name(cache[0], p_name);
}

void FontFile::set_font_style_name(const String &p_name) {
	_ensure_rid(0);
	TS->font_set_style_name(cache[0], p_name);
}

void FontFile::set_font_style(BitField<TextServer::FontStyle> p_style) {
	_ensure_rid(0);
	TS->font_set_style(cache[0], p_style);
}

void FontFile::set_font_weight(int p_weight) {
	_ensure_rid(0);
	TS->font_set_weight(cache[0], p_weight);
}

void FontFile::set_font_stretch(int p_stretch) {
	_ensure_rid(0);
	TS->font_set_stretch(cache[0], p_stretch);
}

void FontFile::set_antialiasing(TextServer::FontAntialiasing p_antialiasing) {
	if (antialiasing != p_antialiasing) {
		antialiasing = p_antialiasing;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_antialiasing(cache[i], antialiasing);
		}
		emit_changed();
	}
}

TextServer::FontAntialiasing FontFile::get_antialiasing() const {
	return antialiasing;
}

void FontFile::set_generate_mipmaps(bool p_generate_mipmaps) {
	if (mipmaps != p_generate_mipmaps) {
		mipmaps = p_generate_mipmaps;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_generate_mipmaps(cache[i], mipmaps);
		}
		emit_changed();
	}
}

bool FontFile::get_generate_mipmaps() const {
	return mipmaps;
}

void FontFile::set_multichannel_signed_distance_field(bool p_msdf) {
	if (msdf != p_msdf) {
		msdf = p_msdf;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_multichannel_signed_distance_field(cache[i], msdf);
		}
		emit_changed();
	}
}

bool FontFile::is_multichannel_signed_distance_field() const {
	return msdf;
}

void FontFile::set_msdf_pixel_range(int p_msdf_pixel_range) {
	if (msdf_pixel_range != p_msdf_pixel_range) {
		msdf_pixel_range = p_msdf_pixel_range;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_msdf_pixel_range(cache[i], msdf_pixel_range);
		}
		emit_changed();
	}
}

int FontFile::get_msdf_pixel_range() const {
	return msdf_pixel_range;
}

void FontFile::set_msdf_size(int p_msdf_size) {
	if (msdf_size != p_msdf_size) {
		msdf_size = p_msdf_size;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_msdf_size(cache[i], msdf_size);
		}
		emit_changed();
	}
}

int FontFile::get_msdf_size() const {
	return msdf_size;
}

void FontFile::set_fixed_size(int p_fixed_size) {
	if (fixed_size != p_fixed_size) {
		fixed_size = p_fixed_size;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_fixed_size(cache[i], fixed_size);
		}
		emit_changed();
	}
}

int FontFile::get_fixed_size() const {
	return fixed_size;
}

void FontFile::set_allow_system_fallback(bool p_allow_system_fallback) {
	if (allow_system_fallback != p_allow_system_fallback) {
		allow_system_fallback = p_allow_system_fallback;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_allow_system_fallback(cache[i], allow_system_fallback);
		}
		emit_changed();
	}
}

bool FontFile::is_allow_system_fallback() const {
	return allow_system_fallback;
}

void FontFile::set_force_autohinter(bool p_force_autohinter) {
	if (force_autohinter != p_force_autohinter) {
		force_autohinter = p_force_autohinter;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_force_autohinter(cache[i], force_autohinter);
		}
		emit_changed();
	}
}

bool FontFile::is_force_autohinter() const {
	return force_autohinter;
}

void FontFile::set_hinting(TextServer::Hinting p_hinting) {
	if (hinting != p_hinting) {
		hinting = p_hinting;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_hinting(cache[i], hinting);
		}
		emit_changed();
	}
}

TextServer::Hinting FontFile::get_hinting() const {
	return hinting;
}

void FontFile::set_subpixel_positioning(TextServer::SubpixelPositioning p_subpixel) {
	if (subpixel_positioning != p_subpixel) {
		subpixel_positioning = p_subpixel;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_subpixel_positioning(cache[i], subpixel_positioning);
		}
		emit_changed();
	}
}

TextServer::SubpixelPositioning FontFile::get_subpixel_positioning() const {
	return subpixel_positioning;
}

void FontFile::set_oversampling(real_t p_oversampling) {
	if (oversampling != p_oversampling) {
		oversampling = p_oversampling;
		for (int i = 0; i < cache.size(); i++) {
			_ensure_rid(i);
			TS->font_set_oversampling(cache[i], oversampling);
		}
		emit_changed();
	}
}

real_t FontFile::get_oversampling() const {
	return oversampling;
}

RID FontFile::find_variation(const VariationCoordinates &p_variation_coordinates, int p_face_index, float p_strength, Transform2D p_transform) const {
	// Find existing variation cache.
	/*
	const TextServerVariants& supported_coords = get_supported_variation_list();
	for (int i = 0; i < cache.size(); i++) {
		if (cache[i].is_valid()) {
			const VariationCoordinates &cache_var = TS->font_get_variation_coordinates(cache[i]);
			bool match = true;
			match = match && (TS->font_get_face_index(cache[i]) == p_face_index);
			match = match && (TS->font_get_embolden(cache[i]) == p_strength);
			match = match && (TS->font_get_transform(cache[i]) == p_transform);
			for (const Variant *V = supported_coords.next(nullptr); V && match; V = supported_coords.next(V)) {
				const Vector3 &def = supported_coords[*V];

				real_t c_v = def.z;
				if (cache_var.has(*V)) {
					real_t val = cache_var[*V];
					c_v = CLAMP(val, def.x, def.y);
				}
				if (cache_var.has(TS->tag_to_name(*V))) {
					real_t val = cache_var[TS->tag_to_name(*V)];
					c_v = CLAMP(val, def.x, def.y);
				}

				real_t s_v = def.z;
				if (p_variation_coordinates.has(*V)) {
					real_t val = p_variation_coordinates[*V];
					s_v = CLAMP(val, def.x, def.y);
				}
				if (p_variation_coordinates.has(TS->tag_to_name(*V))) {
					real_t val = p_variation_coordinates[TS->tag_to_name(*V)];
					s_v = CLAMP(val, def.x, def.y);
				}

				match = match && (c_v == s_v);
			}
			if (match) {
				return cache[i];
			}
		}
	}

	// Create new variation cache.
	int idx = cache.size();
	_ensure_rid(idx);
	TS->font_set_variation_coordinates(cache[idx], p_variation_coordinates);
	TS->font_set_face_index(cache[idx], p_face_index);
	TS->font_set_embolden(cache[idx], p_strength);
	TS->font_set_transform(cache[idx], p_transform);
	return cache[idx];
	*/
	SKR_UNIMPLEMENTED_FUNCTION();
	return RID();
}

RID FontFile::_get_rid() const {
	_ensure_rid(0);
	return cache[0];
}

int FontFile::get_cache_count() const {
	return cache.size();
}

void FontFile::clear_cache() {
	_clear_cache();
	cache.clear();
	emit_changed();
}

void FontFile::remove_cache(int p_cache_index) {
	ERR_FAIL_INDEX(p_cache_index, cache.size());
	if (cache[p_cache_index].is_valid()) {
		TS->free_rid(cache[p_cache_index]);
	}
	cache.remove_at(p_cache_index);
	emit_changed();
}

TypedArray<Vector2i> FontFile::get_size_cache_list(int p_cache_index) const {
	ERR_FAIL_COND_V(p_cache_index < 0, TypedArray<Vector2i>());
	_ensure_rid(p_cache_index);
	return TS->font_get_size_cache_list(cache[p_cache_index]);
}

void FontFile::clear_size_cache(int p_cache_index) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_clear_size_cache(cache[p_cache_index]);
}

void FontFile::remove_size_cache(int p_cache_index, const Vector2i &p_size) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_remove_size_cache(cache[p_cache_index], p_size);
}

void FontFile::set_variation_coordinates(int p_cache_index, const VariationCoordinates& p_variation_coordinates) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_variation_coordinates(cache[p_cache_index], p_variation_coordinates);
}

VariationCoordinates FontFile::get_variation_coordinates(int p_cache_index) const {
	ERR_FAIL_COND_V(p_cache_index < 0, VariationCoordinates());
	_ensure_rid(p_cache_index);
	return TS->font_get_variation_coordinates(cache[p_cache_index]);
}

void FontFile::set_embolden(int p_cache_index, float p_strength) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_embolden(cache[p_cache_index], p_strength);
}

float FontFile::get_embolden(int p_cache_index) const {
	ERR_FAIL_COND_V(p_cache_index < 0, 0.f);
	_ensure_rid(p_cache_index);
	return TS->font_get_embolden(cache[p_cache_index]);
}

void FontFile::set_transform(int p_cache_index, Transform2D p_transform) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_transform(cache[p_cache_index], p_transform);
}

Transform2D FontFile::get_transform(int p_cache_index) const {
	ERR_FAIL_COND_V(p_cache_index < 0, Transform2D());
	_ensure_rid(p_cache_index);
	return TS->font_get_transform(cache[p_cache_index]);
}

void FontFile::set_face_index(int p_cache_index, int64_t p_index) {
	ERR_FAIL_COND(p_cache_index < 0);
	ERR_FAIL_COND(p_index < 0);
	ERR_FAIL_COND(p_index >= 0x7FFF);

	_ensure_rid(p_cache_index);
	TS->font_set_face_index(cache[p_cache_index], p_index);
}

int64_t FontFile::get_face_index(int p_cache_index) const {
	ERR_FAIL_COND_V(p_cache_index < 0, 0);
	_ensure_rid(p_cache_index);
	return TS->font_get_face_index(cache[p_cache_index]);
}

void FontFile::set_cache_ascent(int p_cache_index, int p_size, real_t p_ascent) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_ascent(cache[p_cache_index], p_size, p_ascent);
}

real_t FontFile::get_cache_ascent(int p_cache_index, int p_size) const {
	ERR_FAIL_COND_V(p_cache_index < 0, 0.f);
	_ensure_rid(p_cache_index);
	return TS->font_get_ascent(cache[p_cache_index], p_size);
}

void FontFile::set_cache_descent(int p_cache_index, int p_size, real_t p_descent) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_descent(cache[p_cache_index], p_size, p_descent);
}

real_t FontFile::get_cache_descent(int p_cache_index, int p_size) const {
	ERR_FAIL_COND_V(p_cache_index < 0, 0.f);
	_ensure_rid(p_cache_index);
	return TS->font_get_descent(cache[p_cache_index], p_size);
}

void FontFile::set_cache_underline_position(int p_cache_index, int p_size, real_t p_underline_position) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_underline_position(cache[p_cache_index], p_size, p_underline_position);
}

real_t FontFile::get_cache_underline_position(int p_cache_index, int p_size) const {
	ERR_FAIL_COND_V(p_cache_index < 0, 0.f);
	_ensure_rid(p_cache_index);
	return TS->font_get_underline_position(cache[p_cache_index], p_size);
}

void FontFile::set_cache_underline_thickness(int p_cache_index, int p_size, real_t p_underline_thickness) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_underline_thickness(cache[p_cache_index], p_size, p_underline_thickness);
}

real_t FontFile::get_cache_underline_thickness(int p_cache_index, int p_size) const {
	ERR_FAIL_COND_V(p_cache_index < 0, 0.f);
	_ensure_rid(p_cache_index);
	return TS->font_get_underline_thickness(cache[p_cache_index], p_size);
}

void FontFile::set_cache_scale(int p_cache_index, int p_size, real_t p_scale) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_scale(cache[p_cache_index], p_size, p_scale);
}

real_t FontFile::get_cache_scale(int p_cache_index, int p_size) const {
	ERR_FAIL_COND_V(p_cache_index < 0, 0.f);
	_ensure_rid(p_cache_index);
	return TS->font_get_scale(cache[p_cache_index], p_size);
}

int FontFile::get_texture_count(int p_cache_index, const Vector2i &p_size) const {
	ERR_FAIL_COND_V(p_cache_index < 0, 0);
	_ensure_rid(p_cache_index);
	return TS->font_get_texture_count(cache[p_cache_index], p_size);
}

void FontFile::clear_textures(int p_cache_index, const Vector2i &p_size) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_clear_textures(cache[p_cache_index], p_size);
}

void FontFile::remove_texture(int p_cache_index, const Vector2i &p_size, int p_texture_index) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_remove_texture(cache[p_cache_index], p_size, p_texture_index);
}

void FontFile::set_texture_image(int p_cache_index, const Vector2i &p_size, int p_texture_index, const Ref<Image> &p_image) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_texture_image(cache[p_cache_index], p_size, p_texture_index, p_image);
}

Ref<Image> FontFile::get_texture_image(int p_cache_index, const Vector2i &p_size, int p_texture_index) const {
	ERR_FAIL_COND_V(p_cache_index < 0, Ref<Image>());
	_ensure_rid(p_cache_index);
	return TS->font_get_texture_image(cache[p_cache_index], p_size, p_texture_index);
}

void FontFile::set_texture_offsets(int p_cache_index, const Vector2i &p_size, int p_texture_index, const PackedInt32Array &p_offset) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_texture_offsets(cache[p_cache_index], p_size, p_texture_index, p_offset);
}

PackedInt32Array FontFile::get_texture_offsets(int p_cache_index, const Vector2i &p_size, int p_texture_index) const {
	ERR_FAIL_COND_V(p_cache_index < 0, PackedInt32Array());
	_ensure_rid(p_cache_index);
	return TS->font_get_texture_offsets(cache[p_cache_index], p_size, p_texture_index);
}

PackedInt32Array FontFile::get_glyph_list(int p_cache_index, const Vector2i &p_size) const {
	ERR_FAIL_COND_V(p_cache_index < 0, PackedInt32Array());
	_ensure_rid(p_cache_index);
	return TS->font_get_glyph_list(cache[p_cache_index], p_size);
}

void FontFile::clear_glyphs(int p_cache_index, const Vector2i &p_size) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_clear_glyphs(cache[p_cache_index], p_size);
}

void FontFile::remove_glyph(int p_cache_index, const Vector2i &p_size, int32_t p_glyph) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_remove_glyph(cache[p_cache_index], p_size, p_glyph);
}

void FontFile::set_glyph_advance(int p_cache_index, int p_size, int32_t p_glyph, const Vector2 &p_advance) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_glyph_advance(cache[p_cache_index], p_size, p_glyph, p_advance);
}

Vector2 FontFile::get_glyph_advance(int p_cache_index, int p_size, int32_t p_glyph) const {
	ERR_FAIL_COND_V(p_cache_index < 0, Vector2());
	_ensure_rid(p_cache_index);
	return TS->font_get_glyph_advance(cache[p_cache_index], p_size, p_glyph);
}

void FontFile::set_glyph_offset(int p_cache_index, const Vector2i &p_size, int32_t p_glyph, const Vector2 &p_offset) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_glyph_offset(cache[p_cache_index], p_size, p_glyph, p_offset);
}

Vector2 FontFile::get_glyph_offset(int p_cache_index, const Vector2i &p_size, int32_t p_glyph) const {
	ERR_FAIL_COND_V(p_cache_index < 0, Vector2());
	_ensure_rid(p_cache_index);
	return TS->font_get_glyph_offset(cache[p_cache_index], p_size, p_glyph);
}

void FontFile::set_glyph_size(int p_cache_index, const Vector2i &p_size, int32_t p_glyph, const Vector2 &p_gl_size) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_glyph_size(cache[p_cache_index], p_size, p_glyph, p_gl_size);
}

Vector2 FontFile::get_glyph_size(int p_cache_index, const Vector2i &p_size, int32_t p_glyph) const {
	ERR_FAIL_COND_V(p_cache_index < 0, Vector2());
	_ensure_rid(p_cache_index);
	return TS->font_get_glyph_size(cache[p_cache_index], p_size, p_glyph);
}

void FontFile::set_glyph_uv_rect(int p_cache_index, const Vector2i &p_size, int32_t p_glyph, const Rect2 &p_uv_rect) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_glyph_uv_rect(cache[p_cache_index], p_size, p_glyph, p_uv_rect);
}

Rect2 FontFile::get_glyph_uv_rect(int p_cache_index, const Vector2i &p_size, int32_t p_glyph) const {
	ERR_FAIL_COND_V(p_cache_index < 0, Rect2());
	_ensure_rid(p_cache_index);
	return TS->font_get_glyph_uv_rect(cache[p_cache_index], p_size, p_glyph);
}

void FontFile::set_glyph_texture_idx(int p_cache_index, const Vector2i &p_size, int32_t p_glyph, int p_texture_idx) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_glyph_texture_idx(cache[p_cache_index], p_size, p_glyph, p_texture_idx);
}

int FontFile::get_glyph_texture_idx(int p_cache_index, const Vector2i &p_size, int32_t p_glyph) const {
	ERR_FAIL_COND_V(p_cache_index < 0, 0);
	_ensure_rid(p_cache_index);
	return TS->font_get_glyph_texture_idx(cache[p_cache_index], p_size, p_glyph);
}

TypedArray<Vector2i> FontFile::get_kerning_list(int p_cache_index, int p_size) const {
	ERR_FAIL_COND_V(p_cache_index < 0, TypedArray<Vector2i>());
	_ensure_rid(p_cache_index);
	return TS->font_get_kerning_list(cache[p_cache_index], p_size);
}

void FontFile::clear_kerning_map(int p_cache_index, int p_size) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_clear_kerning_map(cache[p_cache_index], p_size);
}

void FontFile::remove_kerning(int p_cache_index, int p_size, const Vector2i &p_glyph_pair) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_remove_kerning(cache[p_cache_index], p_size, p_glyph_pair);
}

void FontFile::set_kerning(int p_cache_index, int p_size, const Vector2i &p_glyph_pair, const Vector2 &p_kerning) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_set_kerning(cache[p_cache_index], p_size, p_glyph_pair, p_kerning);
}

Vector2 FontFile::get_kerning(int p_cache_index, int p_size, const Vector2i &p_glyph_pair) const {
	ERR_FAIL_COND_V(p_cache_index < 0, Vector2());
	_ensure_rid(p_cache_index);
	return TS->font_get_kerning(cache[p_cache_index], p_size, p_glyph_pair);
}

void FontFile::render_range(int p_cache_index, const Vector2i &p_size, char32_t p_start, char32_t p_end) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_render_range(cache[p_cache_index], p_size, p_start, p_end);
}

void FontFile::render_glyph(int p_cache_index, const Vector2i &p_size, int32_t p_index) {
	ERR_FAIL_COND(p_cache_index < 0);
	_ensure_rid(p_cache_index);
	TS->font_render_glyph(cache[p_cache_index], p_size, p_index);
}

void FontFile::set_language_support_override(const String &p_language, bool p_supported) {
	_ensure_rid(0);
	TS->font_set_language_support_override(cache[0], p_language, p_supported);
}

bool FontFile::get_language_support_override(const String &p_language) const {
	_ensure_rid(0);
	return TS->font_get_language_support_override(cache[0], p_language);
}

void FontFile::remove_language_support_override(const String &p_language) {
	_ensure_rid(0);
	TS->font_remove_language_support_override(cache[0], p_language);
}

Vector<String> FontFile::get_language_support_overrides() const {
	_ensure_rid(0);
	return TS->font_get_language_support_overrides(cache[0]);
}

void FontFile::set_script_support_override(const String &p_script, bool p_supported) {
	_ensure_rid(0);
	TS->font_set_script_support_override(cache[0], p_script, p_supported);
}

bool FontFile::get_script_support_override(const String &p_script) const {
	_ensure_rid(0);
	return TS->font_get_script_support_override(cache[0], p_script);
}

void FontFile::remove_script_support_override(const String &p_script) {
	_ensure_rid(0);
	TS->font_remove_script_support_override(cache[0], p_script);
}

Vector<String> FontFile::get_script_support_overrides() const {
	_ensure_rid(0);
	return TS->font_get_script_support_overrides(cache[0]);
}

void FontFile::set_opentype_feature_overrides(const TextServerFeatures& p_overrides) {
	_ensure_rid(0);
	TS->font_set_opentype_feature_overrides(cache[0], p_overrides);
}

TextServerFeatures FontFile::get_opentype_feature_overrides() const {
	_ensure_rid(0);
	return TS->font_get_opentype_feature_overrides(cache[0]);
}

int32_t FontFile::get_glyph_index(int p_size, char32_t p_char, char32_t p_variation_selector) const {
	_ensure_rid(0);
	return TS->font_get_glyph_index(cache[0], p_size, p_char, p_variation_selector);
}

FontFile::FontFile() {
}

FontFile::~FontFile() {
	_clear_cache();
}

} // namespace godot