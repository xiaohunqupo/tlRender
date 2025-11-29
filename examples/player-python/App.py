# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import MainWindow
import SettingsModel

class App(ftk.App):
    def __init__(self, context, argv):
        cmdLineInput = ftk.CmdLineValueArgString("Input", "Timeline file", True)
        ftk.App.__init__(self, context, argv, "player", "Python player example", [ cmdLineInput ])

        # \bug
        self.displayScale = 2

        self._settingsModel = SettingsModel.SettingsModel(context)
        self._timeUnitsModel = tl.TimeUnitsModel(context)

        self._recentFilesModel = ftk.RecentFilesModel(context)
        self._settingsModel.get("/Files/Recent", self._recentFilesModel.recent)
        
        self._player = tl.ObservableValuePlayer(None)
        self._window = MainWindow.MainWindow(context, self)

        if cmdLineInput.hasValue:
            self.open(ftk.Path(cmdLineInput.value))

    def __del__(self):
        self._settingsModel.set("/Files/Recent", list(map(str, self._recentFilesModel.recent)))

    def getTimeUnitsModel(self):
        return self._timeUnitsModel

    def open(self, path):
        timeline = tl.Timeline(self.context, path)
        self._player.setAlways(tl.Player(self.context, timeline))
        self._recentFilesModel.addRecent(path.get())

    def close(self):
        if self._player.get():
            self._player.setAlways(None)

    def reload(self):
        if self._player.get():
            path = self._player.get().path
            timeline = tl.Timeline(self.context, path)
            self._player.setAlways(tl.Player(self.context, timeline))

    def observePlayer(self):
        return self._player

    def tick(self):
        super().tick()
        if self._player.get():
            self._player.get().tick()
