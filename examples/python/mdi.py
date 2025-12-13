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
        
        self._players = []

        # Create the MDI canvas.
        self._mdiCanvas = ftk.MDICanvas(context)
        self.widget = self._mdiCanvas

    def tickEvent(self, parentsVisible, parentsEnabled, event):
        super().tickEvent(parentsVisible, parentsEnabled, event)
        
        # Tick the timeline players.
        for player in self._players:
            player.tick()

    def dropEvent(self, event):
        event.accept = True
        if isinstance(event.data, ftk.DragDropTextData):
            for text in event.data.text:

                # \todo Add exception handling.
                timeline = tl.timeline.Timeline(self.context, text)
                player = tl.timeline.Player(self.context, timeline)
                self._players.append(player)

                timelineWidget = tl.ui.TimelineWidget(self.context)
                timelineWidget.player = player
                self._mdiCanvas.addWidget(text, event.pos, timelineWidget)

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

