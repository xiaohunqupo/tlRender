# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import ftkPy as ftk
import tlRenderPy as tl
import sys

#class App(ftk.App):
#    def __init__(self, context, argv):
#        ftk.App.__init__(self, context, argv, "simple", "Simple example")
#
#        self.window = tl.Window(context, self, "simple", ftk.Size2I(1280, 960))

# Create the context and application.
context = ftk.Context()
tl.uiInit(context)
app = ftk.App(context, sys.argv, "simple", "Simple example")
if app.getExit() != 0:
    sys.exit(app.getExit())

# Create a timeline player.
#timeline = tl.Timeline(context, ftk.Path("/home/darby/Dev/tlRender/tlRender/etc/SampleData/SingleClip.otio"))
#player = tl.Player(context, timeline)
#player.playback = tl.Playback.Forward

# Create widgets.
window = tl.Window(context, app, "simple", ftk.Size2I(1280, 960))
#window = ftk.Window(context, app, "simple", ftk.Size2I(1280, 960))
#layout = ftk.VerticalLayout(context, window)
#viewport = tl.Viewport(context, layout)
#viewport.setPlayer(player)
#widget = tl.TimelineWidget(context, layout)
#widget.setPlayer(player)

# Run the application.
app.run()

