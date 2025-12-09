# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import sys
import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import os
import pathlib
import weakref

class MainWindow(ftk.MainWindow):
    def __init__(self, context, app):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1280, 960))

        self._player = None
        
        # Create the file menu.
        fileMenu = self.menuBar.getMenu("File")
        fileMenu.clear()

        selfWeak = weakref.ref(self)
        self._fileOpenAction = ftk.Action(
            "Open",
            "FileOpen",
            ftk.Key.O,
            ftk.commandKeyModifier,
            lambda: context.getSystemByName("ftk::FileBrowserSystem").open(
                selfWeak(),
                selfWeak().open))
        fileMenu.addAction(self._fileOpenAction)

        self._fileCloseAction = ftk.Action(
            "Close",
            "FileClose",
            ftk.Key.E,
            ftk.commandKeyModifier,
            lambda: selfWeak().close())
        fileMenu.addAction(self._fileCloseAction)

        appWeak = weakref.ref(app)
        self._fileExitAction = ftk.Action(
            "Exit",
            ftk.Key.Q,
            ftk.commandKeyModifier,
            lambda: appWeak().exit())
        fileMenu.addDivider()
        fileMenu.addAction(self._fileExitAction)

        # Create the timeline.
        self._timelineWidget = tl.ui.TimelineWidget(context)
        self._timelineWidget.backgroundColor = ftk.ColorRole.Red
        self._timelineWidget.vStretch = ftk.Stretch.Expanding
        timelineDisplayOptions = tl.ui.DisplayOptions()
        timelineDisplayOptions.minimize = False
        timelineDisplayOptions.thumbnailHeight = 600
        timelineDisplayOptions.waveformHeight = 300
        self._timelineWidget.displayOptions = timelineDisplayOptions
        self.widget = self._timelineWidget
        
        self._widgetUpdate()

    def open(self, path):
        # Create the timeline and player.
        #
        # \todo Add exception handling.
        timelineOptions = tl.timeline.Options()
        timeline = tl.timeline.Timeline(self.context, path, timelineOptions)
        playerOptions = tl.timeline.PlayerOptions()
        self._player = tl.timeline.Player(self.context, timeline, playerOptions)
        self._timelineWidget.player = self._player
        
        self._widgetUpdate()

    def close(self):
        self._player = None
        self._timelineWidget.player = None
        self._widgetUpdate()

    def tickEvent(self, parentsVisible, parentsEnabled, event):
        super().tickEvent(parentsVisible, parentsEnabled, event)
        if self._player:
            self._player.tick()

    def _widgetUpdate(self):
        self._fileCloseAction.enabled = self._player != None

# Create the application.
context = ftk.Context()
tl.ui.init(context)
cmdLineInput = ftk.CmdLineValueArgString("Input", "Input file", True)
app = ftk.App(context, sys.argv, "timeline", "Python timeline example.", [ cmdLineInput ])
if app.exitValue != 0:
    sys.exit(app.exitValue)

# Create the main window.
window = MainWindow(context, app)
if cmdLineInput.hasValue:
    window.open(ftk.Path(cmdLineInput.value))

# \bug Set the display scale manually.
app.displayScale = 2

app.run()

# Clean up.
window = None
app = None
