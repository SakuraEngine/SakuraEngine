#pragma once
#include "SkrGuiRenderer/gdi_renderer.hpp"
#include "misc/make_zeroed.hpp"
#include "misc/defer.hpp"
#include "async/thread_job.hpp"

namespace skr::gui
{
struct ImageTexFutureLauncher : public skr::FutureLauncher<bool> {
};
} // namespace skr::gui