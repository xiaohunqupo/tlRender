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
            self._openFile)
        self.fileMenu = ftk.Menu(context)
        self.fileMenu.addAction(self.fileOpenAction)
        self.menuBar = ftk.MenuBar(context)
        self.menuBar.addMenu("File", self.fileMenu)

        layout = ftk.VerticalLayout(context)
        self.viewport = tl.Viewport(context, layout)
        self.widget = tl.TimelineWidget(context, layout)
        self.setWidget(layout)

    def setPlayer(self, player):
        self.viewport.setPlayer(player)
        self.widget.setPlayer(player)

    def _openFile(self):
        fileBrowserSystem = context.getSystemByName("ftk::FileBrowserSystem")
        fileBrowserSystem.open(
            self,
            self._openFile2)

class App(ftk.App):
    def __init__(self, context, argv):
        ftk.App.__init__(self, context, argv, "simple", "Simple example")

        self.timeline = tl.Timeline(
            context,
            #ftk.Path("/home/darby/Desktop/ASC_StEM2_178_8K_ST2084_1000nits_Rec2020_Stereo.mov"))
            ftk.Path("/home/darby/Dev/tlRender/tlRender/etc/SampleData/TransitionOverlay.otio"))
        self.player = tl.Player(context, self.timeline)
        #self.player.playback = tl.Playback.Forward

        self.window = MainWindow(context, self)
        self.window.setPlayer(self.player)

    def tick(self):
        super().tick()
        self.player.tick()

context = ftk.Context()
tl.uiInit(context)
app = App(context, sys.argv)
if app.getExit() != 0:
    sys.exit(app.getExit())
app.run()

