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
        self._playbackActions = Actions.PlaybackActions(context, app)

        self._menuBar = ftk.MenuBar(context)
        self._menuBar.addMenu("File", Menus.createFileMenu(context, self._fileActions))
        self._menuBar.addMenu("Playback", Menus.createPlaybackMenu(context, self._playbackActions))

        self._fileToolBar = ToolBars.createFileToolBar(context, self._fileActions)

        self._viewport = tl.Viewport(context)

        self._playbackBar = Widgets.PlaybackBar(context, app, self._playbackActions)

        self._timelineWidget = tl.TimelineWidget(context, app.getTimeUnitsModel())

        self._layout = ftk.VerticalLayout(context)
        self._layout.spacingRole = ftk.SizeRole._None
        self.setWidget(self._layout)
        self._fileToolBar.parent = self._layout
        self._splitter = ftk.Splitter(context, ftk.Orientation.Vertical, self._layout)
        self._splitter.split = .7
        self._viewport.parent = self._splitter
        vLayout = ftk.VerticalLayout(context, self._splitter)
        vLayout.spacingRole = ftk.SizeRole._None
        self._playbackBar.parent = vLayout
        ftk.Divider(context, ftk.Orientation.Vertical, vLayout)
        self._timelineWidget.parent = vLayout

        self.playerObserver = tl.ValueObserverPlayer(
            app.observePlayer(),
            self._widgetUpdate)

    def _widgetUpdate(self, player):
        self._viewport.setPlayer(player)
        self._timelineWidget.setPlayer(player)
