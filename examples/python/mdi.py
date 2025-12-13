# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

# This example displays multiple timelines in a MDI widget.

import sys
import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import os

class MainWindow(ftk.MainWindow):
    def __init__(self, context):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1280, 960))

        # Create the MDI canvas.
        self._mdiCanvas = ftk.MDICanvas(context)
        self.widget = self._mdiCanvas

    def dropEvent(self, event):
        event.accept = True
        if isinstance(event.data, ftk.DragDropTextData):
            for text in event.data.text:

                # \todo Add exception handling.
                timeline = tl.timeline.Timeline(self.context, text)
                player = tl.timeline.Player(self.context, timeline)

                splitter = ftk.Splitter(context, ftk.Orientation.Vertical)

                viewport = tl.ui.Viewport(self.context, splitter)
                viewport.player = player

                timelineWidget = tl.ui.TimelineWidget(self.context, splitter)
                timelineDisplayOptions = tl.ui.DisplayOptions()
                timelineDisplayOptions.minimize = False
                timelineDisplayOptions.thumbnailHeight = 300
                timelineDisplayOptions.waveformHeight = 150
                timelineWidget.displayOptions = timelineDisplayOptions
                timelineWidget.vStretch = ftk.Stretch.Expanding
                timelineWidget.player = player
                
                self._mdiCanvas.addWidget(text, event.pos, splitter)

# Create the application.
context = ftk.Context()
tl.ui.init(context)
app = ftk.App(context, sys.argv, "mdi", "Python MDI example.")
if app.exitValue != 0:
    sys.exit(app.exitValue)

# Create the main window.
window = MainWindow(context)

# Run the application.
app.run()

# Clean up.
window = None
app = None

