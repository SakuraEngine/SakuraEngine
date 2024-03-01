// Copyright 2012-2020 Trimble Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SketchUpModelExporterInterface.
 */
#ifndef MODELEXPORTERPLUGIN_H_
#define MODELEXPORTERPLUGIN_H_

#include <map>
#include <string>

#include <SketchUpAPI/import_export/pluginprogresscallback.h>

/**
@struct SketchUpModelExporterInterface
@brief  A valid SketchUp "exporter" plugin must support the following interface.
        See the SDK samples for an example.
*/
class SketchUpModelExporterInterface {
 public:
  /**
  @brief  Returns a unique ASCII (non-localized) identifier for the exporter.
          For example, all SketchUp supplied exporters use the following
          convention for the id name: com.sketchup.exporters.extension. This id
          is used to identify the current default exporter and saved between
          invocations of SketchUp.
  @return
  - A unique string identifier for the plugin
  */
  virtual std::string GetIdentifier() const = 0;

  /**
  @brief The number of extensions supported by the exporter.  Must be > 0.
  @return
  - The number of file extensions supported by this plugin
  */
  virtual int GetFileExtensionCount() const = 0;

  /**
  @brief  Each extension, assumed to be ASCII and case independent with no
          leading dot.
  @param[in] index The index of the extension
  @return
  - The file extension retrieved
  */
  virtual std::string GetFileExtension(int index) const = 0;

  /**
  @brief Return a brief description of the exporter for the file drop down box
         which may be extension specific. For example you may return "AutoCAD
         DWG Files" corresponding to the dwg extension and "AutoCAD DXF Files"
         corresponding to the dxf extension. Each extension and description pair
         will be entered into the exporter drop down list as a separate entry.
  @param[in] index The index of the extension
  @return
  - The description retrieved
  */
  virtual std::string GetDescription(int index) const = 0;


  /**
  @brief Indicates whether the exporter supports an options dialog.
  @return
  - true if the exporter supports an options dialog. If so, the "Options" button
    will be enabled when your exporter is chosen, and selecting that button will
    call the exporter's ShowOptionsDialog method.
  */
  virtual bool SupportsOptions() const {
    return false;
  }

  /**
  @brief  Displays a modal dialog which controls options supported by the
          exporter. Implementing this is required if SupportOptions is true.
          Options should be saved someplace which persists between this method
          and ConvertFromSkp as well as between sessions.
  @param[in] model_has_selection Indicates if the model has a current selection
               set. May be used in concert with ExportSelectionSetOnly to allow
               the exporter to export just the selected portion of the model.
               For example, you may enable a check box which controls this and
               return the state of that check box from ExportSelectionSetOnly.
  */
  virtual void ShowOptionsDialog(bool model_has_selection) {
  }

  /**
  @brief  Indicates whether the plugin supports exporting just the selection.
  @return
  - true if the exporter supports an option to just export the selection AND the
    user has set that to true. If true, SketchUp will save the selected portion
    of the model and pass that into the ConvertFromSkp method.
  */
  virtual bool ExportSelectionSetOnly() {
    return false;
  }

  /**
  @brief Indicates whether the plugin supports the progress callback.
  @return
  - true if the exporter supports the progress callback.  If true, the plugin
    should periodically update the status with messages and percent complete
    as well as check if the user pressed the cancel button.  If false, SketchUp
    will provide limited feedback on the progress of the export step.
  */
  virtual bool SupportsProgress() const {
    return true;
  }

  /**
  @brief  Performs the conversion from a temporary skp file to the target output
          file using options set during the ShowOptionsDialog method.
  @param[in] input_skp Either a saved version of the current model or the
                       current selection (if ExportSelectionSetOnly returns
                       true). The method should read this file and convert to
                       the output file. The file name is specified in UTF-8 and
                       may require conversion to a platform-dependent format
                       (i.e. UTF-16) for proper support of international file
                       names.
  @param[in] output    The requested output file in UTF-8.
  @param[in] progress  The exporter should support this progress interface. See
                       that for details. The method should be tolerant to a NULL
                       progress callback.
  @param[in] reserved  Reserved for internal use.
  @return
  - true on success
  - false on failure or cancellation
  */
  virtual bool ConvertFromSkp(
      const std::string& input_skp, const std::string& output,
      SketchUpPluginProgressCallback* progress, void* reserved) = 0;
  /**
  @brief Displays a modal dialog showing an optional summary of the export
         process.  The default does nothing.  Note that Ruby scripting often
         disables this dialog.
  */
  virtual void ShowSummaryDialog() {
  }
};

////////////////////////////////////////////////////////////////////////////////

#ifdef __OBJC__
  #import <Foundation/Foundation.h>
/** Each Mac plugin should support this simple protocol.
 */
@protocol SketchUpModelExporterPlugin<NSObject>
/**
@brief Returns an auto-released instance of the exporter.
*/
+ (id<SketchUpModelExporterPlugin>)exporter;

/**
@brief  This returns our c++ interface from the obj-c wrapper.
*/
- (SketchUpModelExporterInterface*)sketchUpModelExporterInterface;
@end

#else
/**
 * Each Windows DLL should export a function which returns the C++ interface.
 */
typedef SketchUpModelExporterInterface* (*GetSketchUpModelExporterInterfaceFunc)(void);
#endif
#endif  // MODELEXPORTERPLUGIN_H_
