// Copyright 2013-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for import/export progress feedback and cancellation.
 */
#ifndef SKETCHUPPLUGINPROGRESSCALLBACK_H_
#define SKETCHUPPLUGINPROGRESSCALLBACK_H_

#include <string>

/**
@struct SketchUpPluginProgressCallback
@brief  Interface to provide import/export progress feedback and cancellation.

All well behaved SketchUp importer/exporter plugins should support progress
feedback and cancellation through this interface. SketchUp will pop up a
progress/cancel dialog prior to calling the appropriate Convert method on the
importer/exporter plugin and supply a concrete implementation of this interface.
The plugin should supply feedback to the user by estimating how far along the
conversion is (via SetPercentDone or SetStepSize and Step), provide some textual
progress (via SetProgressMessage) and test for cancellation by calling
HasBeenCancelled.

For example, the main loop of an importer plugin may look like this:

\code{.cpp}
progress_callback->SetProgressMessage("Importing 50 things!");
progress_callback->SetStepSize(1.0);
for (int steps = 0; steps < 50; ++steps) {
  progress_callback->Step();
  // Do something which builds an skp model
  if (progress_callback->HasBeenCancelled()) {
    // Clean up and return
    return false;
  }
}
progress_callback->SetProgressMessage("We're half way there!");
// Do some more larger steps
progress_callback->SetPercentDone(75.0);
progress_callback->SetProgressMessage("Finishing up!");
// And some more
progress_callback->SetPercentDone(100.0);
// Success!
return true;
\endcode

*/
class SketchUpPluginProgressCallback {
 public:
  /**
  @brief  Call this to test if the user has pressed the cancel button.  Your
          code should clean up and return promptly after detecting this.
  @return Returns true if the user has pressed cancel.
  */
  virtual bool HasBeenCancelled() = 0;

  /**
  @brief  Call this to set the percent complete on the dialog.
  @param[in] percent The percent done from 0 to 100.
   */
  virtual void SetPercentDone(double percent) = 0;

  /**
  @brief  You may also set a step size and increment it using the Step method.
  @param[in] percent The percent to increment with each call to Step.
   */
  virtual void SetStepSize(double percent) = 0;

  /**
  @brief  Increments the progress dialog by the step size set using
          SetStepSize.
  */
  virtual void Step() = 0;

  /**
  @brief  Supplies a UTF-8 message which will be presented on the progress
          dialog.
  @param utf8_message The message.
  */
  virtual void SetProgressMessage(const std::string& utf8_message) = 0;
};

#endif  // SKETCHUPPLUGINPROGRESSCALLBACK_H_
