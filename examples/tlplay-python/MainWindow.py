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

import weakref

class MainWindow(ftk.MainWindow):

    def __init__(self, context, app):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1280, 960))

        # Get settings.
        settingsToggle = False
        splitter = 0.8
        splitter2 = 0.8
        self._settingsModel = app.getSettingsModel()
        settings = self._settingsModel.getBool("/MainWindow/SettingsVisible")
        if settings[0]:
            settingsToggle = settings[1]
        settings = self._settingsModel.getDouble("/MainWindow/Splitter")
        if settings[0]:
            splitter = settings[1]
        settings = self._settingsModel.getDouble("/MainWindow/Splitter2")
        if settings[0]:
            splitter2 = settings[1]

        # Create observables.
        self.settingsToggle = ftk.ObservableBool(settingsToggle)

        # Create widgets and actions.
        self._viewport = tl.ui.Viewport(context)

        self._timelineWidget = tl.ui.TimelineWidget(context, app.getTimeUnitsModel())

        self._fileActions = FileActions.Actions(context, app, self)
        self._playbackActions = PlaybackActions.Actions(context, app)
        self._viewActions = ViewActions.Actions(context, app, self)
        self._windowActions = WindowActions.Actions(context, app, self)

        self._menuBar = ftk.MenuBar(context)
        self._menuBar.addMenu("File", Menus.File(context, app, self._fileActions))
        self._menuBar.addMenu("Playback", Menus.Playback(context, app, self._playbackActions))
        self._menuBar.addMenu("View", Menus.View(context, app, self._viewActions))
        self._menuBar.addMenu("Window", Menus.Window(context, app, self._windowActions))
        self.menuBar = self._menuBar

        self._fileToolBar = ToolBars.File(context, self._fileActions)
        self._viewToolBar = ToolBars.View(context, self._viewActions)
        self._windowToolBar = ToolBars.Window(context, self._windowActions)

        self._playbackBar = PlaybackBar.Widget(context, app, self._playbackActions)

        self._statusBar = StatusBar.Widget(context, app, self)

        self._settingsWidget = SettingsWidget.Widget(context, app)
        self._settingsWidget.setVisible(settingsToggle)

        # Layout widgets.
        self._layout = ftk.VerticalLayout(context)
        self._layout.spacingRole = ftk.SizeRole._None
        self.setWidget(self._layout)
        hLayout = ftk.HorizontalLayout(context, self._layout)
        hLayout.spacingRole = ftk.SizeRole.SpacingSmall
        self._fileToolBar.parent = hLayout
        ftk.Divider(context, ftk.Orientation.Horizontal, hLayout)
        self._viewToolBar.parent = hLayout
        ftk.Divider(context, ftk.Orientation.Horizontal, hLayout)
        self._windowToolBar.parent = hLayout
        ftk.Divider(context, ftk.Orientation.Vertical, self._layout)
        self._splitter = ftk.Splitter(context, ftk.Orientation.Vertical, self._layout)
        self._splitter.split = splitter
        self._splitter2 = ftk.Splitter(context, ftk.Orientation.Horizontal, self._splitter)
        self._splitter2.split = splitter2
        self._viewport.parent = self._splitter2
        self._settingsWidget.parent = self._splitter2
        vLayout = ftk.VerticalLayout(context, self._splitter)
        vLayout.spacingRole = ftk.SizeRole._None
        self._playbackBar.parent = vLayout
        ftk.Divider(context, ftk.Orientation.Vertical, vLayout)
        self._timelineWidget.parent = vLayout
        ftk.Divider(context, ftk.Orientation.Vertical, self._layout)
        self._statusBar.parent = self._layout

        # Create observers.
        selfWeak = weakref.ref(self)
        self.playerObserver = tl.timeline.PlayerObserver(
            app.getDocumentModel().observePlayer(),
            lambda value: selfWeak()._widgetUpdate(value))

    def __del__(self):
        self._settingsModel.setBool(
            "/MainWindow/SettingsVisible",
            self.settingsToggle.get())
        self._settingsModel.setDouble("/MainWindow/Splitter", self._splitter.split)
        self._settingsModel.setDouble("/MainWindow/Splitter2", self._splitter2.split)

    def getViewport(self):
        return self._viewport

    def setSettingsVisible(self, value):
        if self.settingsToggle.setIfChanged(value):
            self._settingsWidget.setVisible(value)

    def _widgetUpdate(self, player):
        self._viewport.player = player
        self._timelineWidget.player = player
