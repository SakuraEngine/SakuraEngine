#include "attributes.hxx"
#include <skr/std.hxx>

struct VertexIn {
	[[INSTANCE_ID]] uint instance_id;
	[[MESHLET_ID]] uint meshlet_id;
};
[[TASK_SHADER]] bool IsVisible(
	VertexIn vertex_in) {
    // Draw every mesh
	return true;
}