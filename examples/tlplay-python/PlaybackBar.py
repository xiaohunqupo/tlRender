# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import PlaybackActions

class Widget(ftk.IWidget):

    def __init__(self, context, app, actions, parent = None):
        ftk.IWidget.__init__(self, context, "PlaybackBar.Widget", parent)

        self._player = None

        self._playbackToolBar = ftk.ToolBar(context)
        self._playbackToolBar.addAction(actions.actions["Reverse"])
        self._playbackToolBar.addAction(actions.actions["Stop"])
        self._playbackToolBar.addAction(actions.actions["Forward"])

        self._frameToolBar = ftk.ToolBar(context)
        self._frameToolBar.addAction(actions.actions["Start"])
        button = self._frameToolBar.addAction(actions.actions["Prev"])
        button.repeatClick = True
        button = self._frameToolBar.addAction(actions.actions["Next"])
        button.repeatClick = True
        self._frameToolBar.addAction(actions.actions["End"])

        self._currentTimeEdit = tl.ui.TimeEdit(context, app.getTimeUnitsModel())
        self._currentTimeEdit.tooltip = "Current time."

        self._durationLabel = tl.ui.TimeLabel(context, app.getTimeUnitsModel())
        self._durationLabel.tooltip = "Playback duration."

        self._speedEdit = ftk.DoubleEdit(context)
        self._speedEdit.range = ftk.RangeD(1.0, 99999.0)
        self._speedEdit.step = 1.0
        self._speedEdit.largeStep = 10.0
        self._speedEdit.tooltip = "Playback speed."

        self._timeUnitsWidget = tl.ui.TimeUnitsWidget(context, app.getTimeUnitsModel())
        self._timeUnitsWidget.tooltip = "Time units."

        self._layout = ftk.HorizontalLayout(context, self)
        self._layout.marginRole = ftk.SizeRole.MarginInside
        self._playbackToolBar.parent = self._layout
        self._frameToolBar.parent = self._layout
        self._currentTimeEdit.parent = self._layout
        self._durationLabel.parent = self._layout
        self._speedEdit.parent = self._layout
        self._timeUnitsWidget.parent = self._layout
        
        self._currentTimeEdit.setCallback(self._currentTimeCallback)

        self._speedEdit.setCallback(self._speedCallback)

        self._playerObserver = tl.timeline.ValueObserverPlayer(
            app.getDocumentModel().observePlayer(),
            self._widgetUpdate)

    def setGeometry(self, value):
        ftk.IWidget.setGeometry(self, value)
        self._layout.setGeometry(value)
    
    def sizeHintEvent(self, event):
        self.setSizeHint(self._layout.sizeHint)

    def _currentTimeCallback(self, value):
        if self._player:
            self._player.currentTime = value

    def _currentTimeUpdate(self, value):
        self._currentTimeEdit.value = value

    def _speedCallback(self, value):
        if self._player:
            self._player.speed = value

    def _speedUpdate(self, value):
        self._speedEdit.value = value

    def _widgetUpdate(self, player):
        self._player = player
        if player:
            self._durationLabel.value = player.duration
            self._currentTimeObserver = tl.timeline.ValueObserverRationalTime(
                player.observeCurrentTime,
                self._currentTimeUpdate)
            self._speedObserver = ftk.ValueObserverD(
                player.observeSpeed,
                self._speedUpdate)
        else:
            self._currentTimeEdit.value = tl.invalidTime
            self._durationLabel.value = tl.invalidTime
            self._speedEdit.value = 1.0
            self._currentTimeObserver = None
            self._speedObserver = None
        self._playbackToolBar.enabled = player != None
        self._frameToolBar.enabled = player != None
        self._currentTimeEdit.enabled = player != None
        self._durationLabel.enabled = player != None
        self._speedEdit.enabled = player != None

