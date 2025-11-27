# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import sys
import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import MainWindow

class App(ftk.App):
    def __init__(self, context, argv):
        ftk.App.__init__(self, context, argv, "player", "Python player example")

        # \bug
        self.displayScale = 2

        self._timeUnitsModel = tl.TimeUnitsModel(context)
        self._player = tl.ObservableValuePlayer(None)
        self._window = MainWindow.MainWindow(context, self)

    def getTimeUnitsModel(self):
        return self._timeUnitsModel

    def open(self, path):
        timeline = tl.Timeline(context, path)
        self._player.setAlways(tl.Player(context, timeline))

    def close(self):
        if self._player.get():
            self._player.setAlways( None)

    def observePlayer(self):
        return self._player

    def tick(self):
        super().tick()
        if self._player.get():
            self._player.get().tick()

context = ftk.Context()
tl.uiInit(context)
app = App(context, sys.argv)
if app.getExit() != 0:
    sys.exit(app.getExit())
app.run()

