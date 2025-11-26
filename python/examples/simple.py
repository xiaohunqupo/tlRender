# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import ftkPy as ftk
import tlRenderPy as tl
import sys

class MainWindow(ftk.MainWindow):

    def _openFile2(self, path):
        print(path)

    def __init__(self, context, app):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1280, 960))

        self.fileOpenAction = ftk.Action(
            "Open",
            "FileOpen",
            ftk.Key.O,
            ftk.KeyModifier.Control,
            lambda: context.getSystemByName("ftk::FileBrowserSystem").open(self, app.open))

        self.exitAction = ftk.Action(
            "Exit",
            ftk.Key.Q,
            ftk.KeyModifier.Control,
            lambda: app.exit())

        self.fileMenu = ftk.Menu(context)
        self.fileMenu.addAction(self.fileOpenAction)
        self.fileMenu.addDivider();
        self.fileMenu.addAction(self.exitAction)

        self.menuBar = ftk.MenuBar(context)
        self.menuBar.addMenu("File", self.fileMenu)

        layout = ftk.VerticalLayout(context)
        self.setWidget(layout)
        self.viewport = tl.Viewport(context, layout)
        hLayout = ftk.HorizontalLayout(context, layout)
        self.playbackButtons = tl.PlaybackButtons(context, hLayout)
        self.widget = tl.TimelineWidget(context, layout)

    def setPlayer(self, player):
        self.viewport.setPlayer(player)
        self.widget.setPlayer(player)

class App(ftk.App):
    def __init__(self, context, argv):
        ftk.App.__init__(self, context, argv, "simple", "Simple example")

        # \bug
        self.displayScale = 2

        self.player = None
        self.window = MainWindow(context, self)

    def open(self, path):
        self.timeline = tl.Timeline(context, path)
        self.player = tl.Player(context, self.timeline)
        #self.player.playback = tl.Playback.Forward
        self.window.setPlayer(self.player)

    def tick(self):
        super().tick()
        if self.player:
            self.player.tick()

context = ftk.Context()
tl.uiInit(context)
app = App(context, sys.argv)
if app.getExit() != 0:
    sys.exit(app.getExit())
app.run()

