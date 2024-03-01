// Copyright 2012-2020  Trimble, Inc. All Rights Reserved.

/**
 * @file
 * @brief Interfaces for SketchUpModelImporterInterface.
 */
#ifndef MODELIMPORTERPLUGIN_H_
#define MODELIMPORTERPLUGIN_H_

#include <string>

#include <SketchUpAPI/import_export/pluginprogresscallback.h>

/** @brief Return type of SketchUpOptionsDialogResponse::ShowOptionsDialog(). */
enum SketchUpOptionsDialogResponse {
  IMPORTER_OPTIONS_UNAVAILABLE = 0,  ///< Importer options not available.
  IMPORTER_OPTIONS_CANCELLED = 1,    ///< Importer options cancelled.
  IMPORTER_OPTIONS_ACCEPTED = 2      ///< Importer options accepted.
};

/** Return type of GetImporterBehavior(). */
enum SketchUpModelImporterBehavior {
  IMPORT_MODEL_AT_ORIGIN = 0,     ///< Preserves coordinates of the imported
                                  ///< geometry.
  IMPORT_MODEL_AS_COMPONENT = 1,  ///< Model gets imported as a selected
                                  ///< component with the move tool activated.
  ///< New Sketchup 2017 import behaviors.
  IMPORT_MODEL_PRESERVE_ORIGIN = IMPORT_MODEL_AT_ORIGIN,  ///< Same as
                                                          ///< IMPORT_MODEL_AT_ORIGIN.
  IMPORT_MODEL_RESET_ORIGIN_TO_BBOX_MIN = 2               ///< Translates the coordinates of
                                                          ///< the imported geometry such
                                                          ///< that the minimum point of the
                                                          ///< bounding box is at the origin.
};

/**
@struct SketchUpModelImporterInterface
@brief  A valid SketchUp "importer" plugin must support the following interface.
        See the SDK samples for an example.
*/
class SketchUpModelImporterInterface {
 public:
  /**
  @brief Returns a unique ASCII (non-localized) identifier for the importer.
         For example, all SketchUp supplied importers use the following
         convention for the id name:  com.sketchup.importers.extension.
         This id is used to identify the current default importer and
         saved between invocations of SketchUp.
  @return
  - A unique string identifier for the plugin
  */
  virtual std::string GetIdentifier() const = 0;

  /**
  @brief The number of extensions supported by the importer.  Must be > 0.
  @return
  - The number of file extensions supported by this plugin
  */
  virtual int GetFileExtensionCount() const = 0;

  /**
  @brief Each extension, assumed to be ASCII and case independent with no
         leading dot.
  @param[in] index The index of the extension
  @return
  - The file extension retrieved
   */
  virtual std::string GetFileExtension(int index) const = 0;

  /**
  @brief Return a brief description of the importer for the file drop down
         box. For example, "AutoCAD DWG Files". SketchUp uses this description
         together with all the extensions to form a single entry in the import
         dialog drop down list: e.g., "AutoCAD DWG Files (*.dwg,*.dxf)"
  @return
  - The description retrieved
  */
  virtual std::string GetDescription() const = 0;

  /**
  @brief Indicates whether the importer supports an options dialog.
  @return
  - true if the importer supports an options dialog. If so, the "Options" button
    will be enabled when your importer is chosen, and selecting that button will
    call the importer's ShowOptionsDialog method.
  */
  virtual bool SupportsOptions() const {
    return false;
  }

  /**
  @brief  Displays a modal dialog which controls options supported by the
          importer. Implementing this is required if SupportOptions is true.
          Options should be saved someplace which persists between this method
          and ConvertToSkp as well as between sessions.
  @warning *** Breaking Change: The behavior of this method was changed in
          SketchUp 2019.2, API 7.1 on the Windows API and SketchUp 2020, API 8.0
          on the MacOS API. In previous releases, the method returned void. Now
          it will return \ref SketchUpOptionsDialogResponse.
  @return SketchUpOptionsDialogResponse
          IMPORTER_OPTIONS_UNAVAILABLE if unable to display options,
          IMPORTER_OPTIONS_CANCELLED if options were cancelled,
          IMPORTER_OPTIONS_ACCEPTED if options were accepted

  */
  virtual SketchUpOptionsDialogResponse ShowOptionsDialog() {
    return IMPORTER_OPTIONS_UNAVAILABLE;
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
  @brief  This performs the conversion from the selected input file to the
          given temporary output file using options set during the
          ShowOptionsDialog method.  The output file is then inserted into
          the current model using rules defined by the GetImporterBehavior
          method.
  @param[in] input                     The user selected input file. The file
                                       name is specified in UTF-8 and may
                                       require conversion to a platform
                                       dependent format (i.e. UTF-16) for proper
                                       support of international file names.
  @param[in] output_skp                A temporary output file in UTF-8.
  @param[in] progress                  The importer should support this progress
                                       interface. See that for details. The
                                       method should be tolerant to a NULL
                                       progress callback.
  @param reserved                      Reserved for internal use.
  @return
  - true on success
  - false on failure or cancellation
  */
  virtual bool ConvertToSkp(
      const std::string& input, const std::string& output_skp,
      SketchUpPluginProgressCallback* progress, void* reserved) = 0;
  /**
  @brief Displays a modal dialog showing an optional summary of the import
         process.  The default does nothing.  Note that Ruby scripting often
         disables this dialog.
  */
  virtual void ShowSummaryDialog() {
  }
  /**
  @brief  Defines the method SketchUp uses when placing the imported model.
  */
  virtual SketchUpModelImporterBehavior GetImporterBehavior() const {
    return IMPORT_MODEL_PRESERVE_ORIGIN;
  }

};

#ifdef __OBJC__
/** Each Mac plugin should support this simple protocol.
 */
@protocol SketchUpModelImporterPlugin<NSObject>
/**
@brief Returns an auto-released instance of the importer.
*/
+ (id<SketchUpModelImporterPlugin>)importer;

/**
@brief  This returns our c++ interface from the obj-c wrapper.
*/
// This grabs our c++ interface from the obj-c wrapper.
- (SketchUpModelImporterInterface*)sketchUpModelImporterInterface;
@end

#else
/** Each Windows DLL should export a function which returns the c++ interface.
 */
typedef SketchUpModelImporterInterface* (*GetSketchUpModelImporterInterfaceFunc)(void);
#endif
#endif  // MODELIMPORTERPLUGIN_H_
