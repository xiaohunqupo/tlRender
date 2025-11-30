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
        cmdLineInput = ftk.CmdLineValueArgString("Input", "Timeline file", True)
        ftk.App.__init__(self, context, argv, "player", "Python player example", [ cmdLineInput ])

        # \bug
        self.displayScale = 2

        self._settingsModel = SettingsModel.SettingsModel(context)

        self._timeUnitsModel = tl.TimeUnitsModel(context)

        self._recentFilesModel = ftk.RecentFilesModel(context)
        self._recentFilesModel.recent = self._settingsModel.getStringList("/Files/Recent")[1]
        
        self._documentModel = DocumentModel.Model(self.context, self)
        
        self._window = MainWindow.MainWindow(context, self)

        if cmdLineInput.hasValue:
            self.open(ftk.Path(cmdLineInput.value))

    def __del__(self):
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
