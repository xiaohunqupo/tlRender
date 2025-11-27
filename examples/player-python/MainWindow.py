# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import Actions
import Menus
import ToolBars
import Widgets

class MainWindow(ftk.MainWindow):

    def __init__(self, context, app):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1920, 1080))

        self._fileActions = Actions.FileActions(context, app, self)
        self._playbackActions = Actions.PlaybackActions(context, app, self)

        self.menuBar = ftk.MenuBar(context)
        self.menuBar.addMenu("File", Menus.createFileMenu(context, self._fileActions))
        self.menuBar.addMenu("Playback", Menus.createPlaybackMenu(context, self._playbackActions))

        self._fileToolBar = ToolBars.createFileToolBar(context, self._fileActions)

        self.viewport = tl.Viewport(context)

        self.playbackBar = Widgets.PlaybackBar(context, app, self._playbackActions)

        self.timelineWidget = tl.TimelineWidget(context, app.getTimeUnitsModel())

        self.layout = ftk.VerticalLayout(context)
        self.layout.spacingRole = ftk.SizeRole._None
        self.setWidget(self.layout)
        self._fileToolBar.parent = self.layout
        self.splitter = ftk.Splitter(context, ftk.Orientation.Vertical, self.layout)
        self.splitter.split = .7
        self.viewport.parent = self.splitter
        vLayout = ftk.VerticalLayout(context, self.splitter)
        vLayout.spacingRole = ftk.SizeRole._None
        self.playbackBar.parent = vLayout
        ftk.Divider(context, ftk.Orientation.Vertical, vLayout)
        self.timelineWidget.parent = vLayout

        self.playerObserver = tl.ValueObserverPlayer(app.observePlayer(), self._widgetUpdate)

    def _widgetUpdate(self, player):
        self.viewport.setPlayer(player)
        self.timelineWidget.setPlayer(player)
