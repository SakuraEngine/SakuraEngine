#pragma once

namespace skr::gui
{
// basic
struct Key;
template <typename T>
struct DiagnosticsProperty;
struct Diagnosticable;
struct DiagnosticableTree;
struct DiagnosticableTreeNode;

// widget
struct Widget;
struct StatelessWidget;
struct StatefulWidget;
struct RenderObjectWidget;
struct SingleChildRenderObjectWidget;
struct MultiChildRenderObjectWidget;
struct LeafRenderObjectWidget;
struct ProxyWidget;
struct InheritedWidget;

// element
struct Element;
struct StatelessElement;
struct StatefulElement;
struct RenderObjectElement;
struct ProxyElement;
struct InheritedElement;
struct ComponentElement;
struct RenderWindowElement;
struct RenderNativeWindowElement;

// render object
struct RenderObject;
struct RenderBox;
struct RenderWindow;
struct RenderNativeWindow;

// layer
struct Layer;
struct GeometryLayer;
struct ContainerLayer;
struct OffsetLayer;
struct WindowLayer;
struct NativeWindowLayer;

// binding
struct PaintingContext;
struct BuildOwner;
struct IBuildContext;
struct FocusManager;

// backend
struct ICanvas;
struct WindowContext;
struct INativeWindow;
struct INativeDevice;

// input
struct Event;
struct PointerEvent;
struct HitTestResult;
struct HitTestEntry;

// misc
struct State;
struct Notification;
struct Slot;

} // namespace skr::gui