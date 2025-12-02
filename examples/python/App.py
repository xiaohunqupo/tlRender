# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import MainWindow

import pathlib
import weakref

class App(ftk.App):
    def __init__(self, context, argv):
        
        ftk.App.__init__(
            self,
            context,
            argv,
            "browser-python",
            "Python browser example")

    def getTimeUnitsModel(self):
        return self._timeUnitsModel

    def tick(self):
        super().tick()
        #self._documentModel.tick()

    def run(self):

        # \bug Set the display scale manually.
        self.displayScale = 2

        # Create the time units model.
        self._timeUnitsModel = tl.timeline.TimeUnitsModel(self.context)
        
        # Initialize the file browser.
        fileBrowserSystem = self.context.getSystemByName("ftk::FileBrowserSystem")
        fileBrowserSystem.model.exts = tl.timeline.getExts(self.context)

        # Create the main window.
        self._window = MainWindow.MainWindow(self.context, self)

        super().run()
