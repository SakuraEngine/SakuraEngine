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

} // namespace godot