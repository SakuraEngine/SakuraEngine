#pragma once
#include "SkrSystem/window.h"
#include "SkrContainers/string.hpp"
#include "SkrBase/math.hpp"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
@class CocoaWindowDelegate;
@class CocoaView;
@class CocoaMetalView;
#else
// Forward declarations for C++
typedef struct objc_object NSWindow;
typedef struct objc_object CocoaWindowDelegate;
typedef struct objc_object CocoaView;
typedef struct objc_object CocoaMetalView;
typedef struct CGRect NSRect;
#endif

class CocoaEventSource;

class CocoaWindow : public skr::SystemWindow {
public:
    CocoaWindow(const skr::SystemWindowCreateInfo& info) SKR_NOEXCEPT;
    ~CocoaWindow() SKR_NOEXCEPT override;

    // Window properties
    void set_title(const skr::String& title) SKR_NOEXCEPT override;
    skr::String get_title() const SKR_NOEXCEPT override;
    
    void set_position(int32_t x, int32_t y) SKR_NOEXCEPT override;
    skr::math::int2 get_position() const SKR_NOEXCEPT override;
    
    void set_size(uint32_t width, uint32_t height) SKR_NOEXCEPT override;
    skr::math::uint2 get_size() const SKR_NOEXCEPT override;
    skr::math::uint2 get_physical_size() const SKR_NOEXCEPT override;
    float get_pixel_ratio() const SKR_NOEXCEPT override;
    
    // Window state
    void show() SKR_NOEXCEPT override;
    void hide() SKR_NOEXCEPT override;
    void minimize() SKR_NOEXCEPT override;
    void maximize() SKR_NOEXCEPT override;
    void restore() SKR_NOEXCEPT override;
    void focus() SKR_NOEXCEPT override;
    
    bool is_visible() const SKR_NOEXCEPT override;
    bool is_minimized() const SKR_NOEXCEPT override;
    bool is_maximized() const SKR_NOEXCEPT override;
    bool is_focused() const SKR_NOEXCEPT override;
    
    // Window settings
    void set_opacity(float opacity) SKR_NOEXCEPT override;
    float get_opacity() const SKR_NOEXCEPT override;
    
    // Fullscreen
    void set_fullscreen(bool fullscreen, skr::SystemMonitor* monitor = nullptr) SKR_NOEXCEPT override;
    bool is_fullscreen() const SKR_NOEXCEPT override;
    
    // Platform handles
    void* get_native_handle() const SKR_NOEXCEPT override;
    void* get_native_view() const SKR_NOEXCEPT override;
    
    // Cocoa specific
    NSWindow* get_nswindow() const SKR_NOEXCEPT { return window_; }
    CocoaView* get_cocoa_view() const SKR_NOEXCEPT { return view_; }
    void set_event_source(CocoaEventSource* source) SKR_NOEXCEPT;
    
    // Metal support - creates Metal view if not exists
    void* create_metal_view() SKR_NOEXCEPT;
    // Get the CAMetalLayer from the Metal view (creates view if needed)
    void* get_metal_layer() SKR_NOEXCEPT;

private:
    NSWindow* window_;
    CocoaWindowDelegate* delegate_;
    CocoaView* view_;
    CocoaMetalView* metal_view_; // Created on demand for rendering
    
    // Window state tracking
    bool was_maximized_before_fullscreen_;
    NSRect restore_frame_; // For fullscreen restore
};