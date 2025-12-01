# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import DocumentModel
import MainWindow
import SettingsModel

import pathlib

class App(ftk.App):
    def __init__(self, context, argv):
        cmdLineInput = ftk.CmdLineValueArgString("Input", "Input file", True)
        ftk.App.__init__(self, context, argv, "tlplay-python", "Python player example", [ cmdLineInput ])

        # \bug
        self.displayScale = 2

        # Create the settings model.
        self._settingsModel = SettingsModel.Model(context)

        # Restore file browser system settings.
        fileBrowserSystem = context.getSystemByName("ftk::FileBrowserSystem")
        settings = self._settingsModel.getJSON("/FileBrowser")
        if settings[0]:
            fileBrowserSystem.model.options.from_json(settings[1])
        settings = self._settingsModel.getBool("/NativeFileDialog")
        if settings[0]:
            fileBrowserSystem.nativeFileDialog = settings[1]

        # Create and restore the time units model.
        self._timeUnitsModel = tl.timeline.TimeUnitsModel(context)
        settings = self._settingsModel.getString("/TimeUnits")
        if settings[0]:
            self._timeUnitsModel.timeUnits = tl.timeline.timeUnitsFromString(settings[1])[1]

        # Create and restore the recent files model.
        self._recentFilesModel = ftk.RecentFilesModel(context)
        self._recentFilesModel.recent = self._settingsModel.getStringList("/Files/Recent")[1]
        fileBrowserSystem.recentFilesModel = self._recentFilesModel

        # Create the document model.
        self._documentModel = DocumentModel.Model(self.context, self)
        
        # Create the main window.
        self._window = MainWindow.MainWindow(context, self)

        # Open command line inputs.
        if cmdLineInput.hasValue:
            self.open(ftk.Path(cmdLineInput.value))

    def __del__(self):

        # Save file browser settings.
        fileBrowserSystem = self.context.getSystemByName("ftk::FileBrowserSystem")
        self._settingsModel.setJSON("/FileBrowser", fileBrowserSystem.model.options.to_json())
        self._settingsModel.setBool("/NativeFileDialog", fileBrowserSystem.nativeFileDialog)
        
        # Save time units settings.
        self._settingsModel.setString("/TimeUnits",
            tl.timeline.timeUnitsToString(self._timeUnitsModel.timeUnits))

        # Save recent files settings.
        self._settingsModel.setStringList("/Files/Recent", self._recentFilesModel.recent)

    def getSettingsModel(self):
        return self._settingsModel

    def getTimeUnitsModel(self):
        return self._timeUnitsModel

    def getRecentFilesModel(self):
        return self._recentFilesModel

    def getDocumentModel(self):
        return self._documentModel

    def open(self, path):
        self._documentModel.open(path)
        self._recentFilesModel.addRecent(path.get())

    def close(self):
        self._documentModel.close()

    def reload(self):
        self._documentModel.reload()

    def tick(self):
        super().tick()
        self._documentModel.tick()
