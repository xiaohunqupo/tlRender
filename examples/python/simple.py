# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

# This example creates a simple application that displays a timeline.
#
# The timeline can be specified on the command line like:
# * python simple.py timeline.otio
# * python simple.py timeline.otioz
# * python simple.py render.mov
# * python simple.py render.#.exr
#
# The timeline can be zoomed with the mouse wheel or "-" and "=" keys. The
# timeline can be panned with control + mouse or the scroll bars.

import sys
import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import os

class MainWindow(ftk.MainWindow):
    def __init__(self, context, path):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1280, 960))

        # Create the timeline and timeline player.
        #
        # \todo Add exception handling.
        timeline = tl.timeline.Timeline(context, path)
        self._player = tl.timeline.Player(context, timeline)

        # Create the timeline widget.
        self._timelineWidget = tl.ui.TimelineWidget(context)
        self._timelineWidget.backgroundColor = ftk.ColorRole.Red
        self._timelineWidget.vStretch = ftk.Stretch.Expanding
        
        # Set timeline widget options.
        timelineDisplayOptions = tl.ui.DisplayOptions()
        timelineDisplayOptions.minimize = False
        timelineDisplayOptions.thumbnailHeight = 600
        timelineDisplayOptions.waveformHeight = 300
        self._timelineWidget.displayOptions = timelineDisplayOptions
        
        # Set the timeline player.
        self._timelineWidget.player = self._player
        
        # Set the timeline widget as the central widget in the window.
        self.widget = self._timelineWidget

    def tickEvent(self, parentsVisible, parentsEnabled, event):
        super().tickEvent(parentsVisible, parentsEnabled, event)
        
        # Tick the timeline player.
        if self._player:
            self._player.tick()

# Create the application.
context = ftk.Context()
tl.ui.init(context)
cmdLineInput = ftk.CmdLineValueArgString("Input", "Input file")
app = ftk.App(context, sys.argv, "timeline", "Python timeline example.", [ cmdLineInput ])
if app.exitValue != 0:
    sys.exit(app.exitValue)

# Create the main window.
window = MainWindow(context, ftk.Path(cmdLineInput.value))

# \bug Set the display scale manually.
app.displayScale = 2

# Run the application.
app.run()

# Clean up.
window = None
app = None
