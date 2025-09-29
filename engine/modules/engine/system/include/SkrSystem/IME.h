#pragma once
#include "SkrSystem/window.h"
#include "SkrBase/math.hpp"
#include "SkrContainersDef/string.hpp"
#include "SkrContainersDef/vector.hpp"
#include <functional>

namespace skr
{

// IME input types - similar to SDL but more comprehensive
enum class EIMEInputType : uint8_t
{
    Text,                   // Normal text input
    Password,               // Password input (hidden)
    Email,                  // Email address
    Username,               // Username/account name
    URL,                    // Web URL
    Search,                 // Search field
    Telephone,              // Phone number
    Number,                 // Numeric input
    NumberPassword,         // Numeric PIN (hidden)
    Date,                   // Date input
    Time,                   // Time input
    Multiline              // Multi-line text
};

// Text capitalization modes
enum class EIMECapitalization : uint8_t
{
    None,                   // No auto-capitalization
    Sentences,              // Capitalize first letter of sentences
    Words,                  // Capitalize first letter of words
    Characters              // Capitalize all characters
};


// Text input area for IME positioning
struct IMETextInputArea
{
    int2 position;          // Position relative to window
    uint2 size;             // Size of the input area
    int32_t cursor_height;  // Cursor height for better IME positioning
};

// Candidate list layout preference
enum class EIMECandidateLayout : uint8_t
{
    Horizontal,             // Horizontal candidate list
    Vertical               // Vertical candidate list
};

// IME composition info
struct IMECompositionInfo
{
    skr::String text;                   // Current composition text
    int32_t cursor_pos;                 // Cursor position within composition
    int32_t selection_start;            // Selection start (-1 if none)
    int32_t selection_length;           // Selection length (-1 if none)
};

// IME candidate info
struct IMECandidateInfo
{
    skr::Vector<skr::String> candidates;  // List of candidates
    int32_t selected_index;               // Currently selected candidate
    int32_t page_start;                   // First candidate index in current page
    int32_t page_size;                    // Number of candidates per page
    int32_t total_candidates;             // Total number of candidates
    EIMECandidateLayout preferred_layout; // Preferred layout
};

// IME event callbacks
struct IMEEventCallbacks
{
    // Text input received (committed text)
    std::function<void(const skr::String& text)> on_text_input;
    
    // Composition updated (uncommitted text)
    std::function<void(const IMECompositionInfo& info)> on_composition_update;
    
    // Composition started
    std::function<void()> on_composition_start;
    
    // Composition ended (committed or cancelled)
    std::function<void(bool committed)> on_composition_end;
    
    // Candidate list updated
    std::function<void(const IMECandidateInfo& info)> on_candidates_update;
    
    // Input area changed (for repositioning IME window)
    std::function<void(const IMETextInputArea& area)> on_input_area_change;
};

// IME interface
struct SKR_SYSTEM_API IME
{
    virtual ~IME() = default;
    
    // Start text input for a window
    virtual void start_text_input(SystemWindow* window) = 0;
    
    // Start text input with properties
    virtual void start_text_input_ex(SystemWindow* window, 
                                    EIMEInputType input_type,
                                    EIMECapitalization capitalization = EIMECapitalization::Sentences,
                                    bool autocorrect = true,
                                    bool multiline = false) = 0;
    
    // Stop text input
    virtual void stop_text_input() = 0;
    
    // Check if text input is active
    virtual bool is_text_input_active() const = 0;
    
    // Get the window currently receiving text input
    virtual SystemWindow* get_text_input_window() const = 0;
    
    // Set text input area (for IME window positioning)
    virtual void set_text_input_area(const IMETextInputArea& area) = 0;
    
    // Get current text input area
    virtual IMETextInputArea get_text_input_area() const = 0;
    
    // Set event callbacks
    virtual void set_event_callbacks(const IMEEventCallbacks& callbacks) = 0;
    
    // Platform-specific features
    virtual bool is_screen_keyboard_supported() const = 0;
    
    // Get current composition info (if any)
    virtual IMECompositionInfo get_composition_info() const = 0;
    
    // Get current candidates info (if any)
    virtual IMECandidateInfo get_candidates_info() const = 0;
    
    // Clear current composition
    virtual void clear_composition() = 0;
    
    // Commit current composition
    virtual void commit_composition() = 0;
    
    // Clipboard operations
    virtual bool has_clipboard_text() const = 0;
    virtual skr::String get_clipboard_text() const = 0;
    virtual void set_clipboard_text(const skr::String& text) = 0;
    
    // Factory method
    static IME* Create(const char* backend = nullptr);
    static void Destroy(IME* ime);
};

} // namespace skr