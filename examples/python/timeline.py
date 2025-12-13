# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

# This example displays a timeline.
#
# The timeline can be specified on the command line like:
# * simple.py timeline.otio
# * simple.py timeline.otioz
# * simple.py render.mov
# * simple.py render.#.exr
#
# The timeline can be zoomed with the mouse wheel, or "-" and "=" keys.
#
# The timeline can be panned with control + mouse, or the scroll bars.

import sys
import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import os

# Create the application.
context = ftk.Context()
tl.ui.init(context)
cmdLineInput = ftk.CmdLineValueArgString("Input", "Input file")
app = ftk.App(context, sys.argv, "timeline", "Python timeline example.", [ cmdLineInput ])
if app.exitValue != 0:
    sys.exit(app.exitValue)

# Create the timeline and timeline player.
#
# \todo Add exception handling.
timeline = tl.timeline.Timeline(context, ftk.Path(cmdLineInput.value))
player = tl.timeline.Player(context, timeline)

# Create the timeline widget.
timelineWidget = tl.ui.TimelineWidget(context)
timelineWidget.backgroundColor = ftk.ColorRole.Red
timelineWidget.vStretch = ftk.Stretch.Expanding
timelineWidget.player = player

# Set timeline widget options.
timelineDisplayOptions = tl.ui.DisplayOptions()
timelineDisplayOptions.minimize = False
timelineDisplayOptions.thumbnailHeight = 600
timelineDisplayOptions.waveformHeight = 300
timelineWidget.displayOptions = timelineDisplayOptions

# Create the main window.
window = ftk.MainWindow(context, app)
window.widget = timelineWidget

# Run the application.
app.run()

# Clean up.
window = None
app = None
