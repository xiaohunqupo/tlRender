# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import FileActions
import Menus
import PlaybackActions
import PlaybackBar
import SettingsWidget
import StatusBar
import ToolBars
import ViewActions
import WindowActions

class MainWindow(ftk.MainWindow):

    def __init__(self, context, app):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1920, 1080))
        
        self.settingsToggle = ftk.ObservableValueBool(False)

        self._fileActions = FileActions.Actions(context, app, self)
        self._playbackActions = PlaybackActions.Actions(context, app)
        self._viewActions = ViewActions.Actions(context, app)
        self._windowActions = WindowActions.Actions(context, app, self)

        self._menuBar = ftk.MenuBar(context)
        self._menuBar.addMenu("File", Menus.File(context, app, self._fileActions))
        self._menuBar.addMenu("Playback", Menus.Playback(context, app, self._playbackActions))
        self._menuBar.addMenu("View", Menus.View(context, app, self._viewActions))
        self._menuBar.addMenu("Window", Menus.Window(context, app, self._windowActions))
        self.menuBar = self._menuBar

        self._fileToolBar = ToolBars.File(context, self._fileActions)
        self._windowToolBar = ToolBars.Window(context, self._windowActions)

        self._viewport = tl.ui.Viewport(context)

        self._playbackBar = PlaybackBar.Widget(context, app, self._playbackActions)

        self._timelineWidget = tl.ui.TimelineWidget(context, app.getTimeUnitsModel())

        self._statusBar = StatusBar.Widget(context, app, self)

        self._settingsWidget = SettingsWidget.Widget(context, app)
        self._settingsWidget.hide()

        self._layout = ftk.VerticalLayout(context)
        self._layout.spacingRole = ftk.SizeRole._None
        self.setWidget(self._layout)
        hLayout = ftk.HorizontalLayout(context, self._layout)
        hLayout.spacingRole = ftk.SizeRole.SpacingSmall
        self._fileToolBar.parent = hLayout
        ftk.Divider(context, ftk.Orientation.Horizontal, hLayout)
        self._windowToolBar.parent = hLayout
        ftk.Divider(context, ftk.Orientation.Vertical, self._layout)
        self._splitter = ftk.Splitter(context, ftk.Orientation.Vertical, self._layout)
        self._splitter.split = .8
        self._splitter2 = ftk.Splitter(context, ftk.Orientation.Horizontal, self._splitter)
        self._splitter2.split = .8
        self._viewport.parent = self._splitter2
        self._settingsWidget.parent = self._splitter2
        vLayout = ftk.VerticalLayout(context, self._splitter)
        vLayout.spacingRole = ftk.SizeRole._None
        self._playbackBar.parent = vLayout
        ftk.Divider(context, ftk.Orientation.Vertical, vLayout)
        self._timelineWidget.parent = vLayout
        ftk.Divider(context, ftk.Orientation.Vertical, self._layout)
        self._statusBar.parent = self._layout

        self.playerObserver = tl.timeline.ValueObserverPlayer(
            app.getDocumentModel().observePlayer(),
            self._widgetUpdate)

    def setSettings(self, value):
        if self.settingsToggle.setIfChanged(value):
            self._settingsWidget.setVisible(value)

    def _widgetUpdate(self, player):
        self._viewport.setPlayer(player)
        self._timelineWidget.setPlayer(player)
