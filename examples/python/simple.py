# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl
import sys

class MainWindow(ftk.MainWindow):

    def _openFile2(self, path):
        print(path)

    def __init__(self, context, app):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1280, 960))

        self.player = None

        self.fileOpenAction = ftk.Action(
            "Open",
            "FileOpen",
            ftk.Key.O,
            ftk.KeyModifier.Control,
            lambda: context.getSystemByName("ftk::FileBrowserSystem").open(self, app.open))

        self.fileCloseAction = ftk.Action(
            "Close",
            "FileClose",
            ftk.Key.E,
            ftk.KeyModifier.Control,
            lambda: app.close())

        self.exitAction = ftk.Action(
            "Exit",
            ftk.Key.Q,
            ftk.KeyModifier.Control,
            lambda: app.exit())

        self.fileMenu = ftk.Menu(context)
        self.fileMenu.addAction(self.fileOpenAction)
        self.fileMenu.addAction(self.fileCloseAction)
        self.fileMenu.addDivider();
        self.fileMenu.addAction(self.exitAction)

        self.menuBar = ftk.MenuBar(context)
        self.menuBar.addMenu("File", self.fileMenu)

        self.viewport = tl.Viewport(context)

        self.playbackButtons = tl.PlaybackButtons(context)

        self.currentTimeEdit = tl.TimeEdit(context, app.getTimeUnitsModel())
        #print(self.currentTimeEdit.value)
        #self.currentTimeEdit.value = otio.opentime.RationalTime(0.0, 24.0)

        self.widget = tl.TimelineWidget(context)

        self.splitter = ftk.Splitter(context, ftk.Orientation.Vertical)
        self.splitter.split = .7
        self.setWidget(self.splitter)
        self.viewport.parent = self.splitter
        vLayout = ftk.VerticalLayout(context, self.splitter)
        vLayout.spacingRole = ftk.SizeRole._None
        hLayout = ftk.HorizontalLayout(context, vLayout)
        hLayout.marginRole = ftk.SizeRole.MarginInside
        hLayout.spacingRole = ftk.SizeRole.SpacingSmall
        self.playbackButtons.parent = hLayout
        self.currentTimeEdit.parent = hLayout
        ftk.Divider(context, ftk.Orientation.Vertical, vLayout)
        self.widget.parent = vLayout

        self._widgetUpdate()

        self.playbackButtons.setCallback(self._playbackCallback)

    def setPlayer(self, player):
        self.player = player
        self.viewport.setPlayer(player)
        self.widget.setPlayer(player)
        if player:
            self.playbackObserver = tl.ValueObserverPlayback(
                player.observePlayback,
                self._playbackCallback2)
            #self.currentTimeObserver = tl.ValueObserverRationalTime(
            #    player.observeCurrentTime,
            #    self._currentTimeCallback)
        else:
            self.playbackObserver = None
            #self.currentTimeObserver = None
        self._widgetUpdate()

    def _playbackCallback(self, value):
        if self.player:
            self.player.playback = value

    def _playbackCallback2(self, value):
        self.playbackButtons.playback = value

    def _currentTimeCallback(self, value):
        self.currentTimeEdit.value = value

    def _widgetUpdate(self):
        enabled = False
        if self.player:
            enabled = True
        self.playbackButtons.enabled = enabled

class App(ftk.App):
    def __init__(self, context, argv):
        ftk.App.__init__(self, context, argv, "simple", "Simple example")

        # \bug
        self.displayScale = 2

        self.timeUnitsModel = tl.TimeUnitsModel(context)
        self.player = None
        self.window = MainWindow(context, self)

    def getTimeUnitsModel(self):
        return self.timeUnitsModel

    def open(self, path):
        self.timeline = tl.Timeline(context, path)
        self.player = tl.Player(context, self.timeline)
        #self.player.playback = tl.Playback.Forward
        self.window.setPlayer(self.player)

    def close(self):
        if self.player:
            self.timeline = None
            self.player = None
            self.window.setPlayer(None)

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

