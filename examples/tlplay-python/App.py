# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import DocumentModel
import MainWindow
import SettingsModel

import pathlib
import weakref

class App(ftk.App):
    """
    The application creates the models and main window.
    """
    def __init__(self, context, argv):
        
        # Command line arguments.
        cmdLineInput = ftk.CmdLineValueArgString("Input", "Input file", True)
        
        ftk.App.__init__(
            self,
            context,
            argv,
            "tlplay-python",
            "Python player example",
            [ cmdLineInput ])

        # \bug Set the display scale manually.
        self.displayScale = 2

        # Create the settings model.
        self._settingsModel = SettingsModel.Model(context)

        # Create the time units model.
        self._timeUnitsModel = tl.timeline.TimeUnitsModel(context)
        settings = self._settingsModel.getString("/TimeUnits")
        if settings[0]:
            self._timeUnitsModel.timeUnits = tl.timeline.timeUnitsFromString(settings[1])[1]

        # Create the recent files model.
        self._recentFilesModel = ftk.RecentFilesModel(context)
        self._recentFilesModel.recent = self._settingsModel.getStringList("/Files/Recent")[1]
        fileBrowserSystem = context.getSystemByName("ftk::FileBrowserSystem")
        fileBrowserSystem.recentFilesModel = self._recentFilesModel

        # Create the document model.
        self._documentModel = DocumentModel.Model(self.context, self)
        
        # Create the main window.
        self._window = MainWindow.MainWindow(context, self)
        
        # Create an observer to update the recent files.
        selfWeak = weakref.ref(self)
        self._playerObserver = tl.timeline.PlayerObserver(
            self._documentModel.observePlayer(),
            lambda player: selfWeak()._playerUpdate(player))

        # Open command line inputs.
        if cmdLineInput.hasValue:
            self._documentModel.open(ftk.Path(cmdLineInput.value))

    def __del__(self):
        
        # Save time units settings.
        self._settingsModel.setString("/TimeUnits",
            tl.timeline.timeUnitsToString(self._timeUnitsModel.timeUnits))

        # Save recent files settings.
        self._settingsModel.setStringList("/Files/Recent", self._recentFilesModel.recent)

    def getSettingsModel(self):
        """
        Get the settings model.
        """
        return self._settingsModel

    def getTimeUnitsModel(self):
        """
        Get the time units model.
        """
        return self._timeUnitsModel

    def getRecentFilesModel(self):
        """
        Get the recent files model.
        """
        return self._recentFilesModel

    def getDocumentModel(self):
        """
        Get the document model.
        """
        return self._documentModel

    def tick(self):
        super().tick()
        self._documentModel.tick()

    def _playerUpdate(self, player):
        if player:
            self._recentFilesModel.addRecent(player.path.get())
