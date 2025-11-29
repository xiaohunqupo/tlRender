# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import weakref

class FileActions:

    def __init__(self, context, app, mainWindow):

        appWeak = weakref.ref(app)
        mainWindowWeak = weakref.ref(mainWindow)
        self.actions = {}
        self.actions["Open"] = ftk.Action(
            "Open",
            "FileOpen",
            ftk.Key.O,
            ftk.KeyModifier.Control,
            lambda: context.getSystemByName("ftk::FileBrowserSystem").open(mainWindowWeak(), appWeak().open))
        self.actions["Open"].tooltip = "Open an image sequence, movie, or timeline file"

        self.actions["Close"] = ftk.Action(
            "Close",
            "FileClose",
            ftk.Key.E,
            ftk.KeyModifier.Control,
            lambda: appWeak().close())
        self.actions["Close"].tooltip = "Close the current file"

        self.actions["Reload"] = ftk.Action(
            "Reload",
            "FileReload",
            ftk.Key.R,
            ftk.KeyModifier.Control,
            lambda: appWeak().reload())
        self.actions["Reload"].tooltip = "Reload the current file"

        self.actions["Exit"] = ftk.Action(
            "Exit",
            ftk.Key.Q,
            ftk.KeyModifier.Control,
            lambda: appWeak().exit())

        self.playerObserver = tl.ValueObserverPlayer(app.observePlayer(), self._actionsUpdate)

    def _actionsUpdate(self, player):
        self.actions["Close"].enabled = player != None
        self.actions["Reload"].enabled = player != None

class PlaybackActions:

    def __init__(self, context, app):
        
        self.actions = {}
        self.actions["Stop"] = ftk.Action(
            "Stop",
            "PlaybackStop",
            ftk.Key.K,
            0,
            self._stopCallback)
        self.actions["Stop"].tooltip = "Stop playback"

        self.actions["Forward"] = ftk.Action(
            "Forward",
            "PlaybackForward",
            ftk.Key.L,
            0,
            self._forwardCallback)
        self.actions["Forward"].tooltip = "Start forward playback"

        self.actions["Reverse"] = ftk.Action(
            "Reverse",
            "PlaybackReverse",
            ftk.Key.J,
            0,
            self._reverseCallback)
        self.actions["Reverse"].tooltip = "Start reverse playback"

        self.actions["Start"] = ftk.Action(
            "Start Frame",
            "FrameStart",
            ftk.Key.Up,
            0,
            self._startCallback)
        self.actions["Start"].tooltip = "Go to the start frame"

        self.actions["Prev"] = ftk.Action(
            "Previous Frame",
            "FramePrev",
            ftk.Key.Left,
            0,
            self._prevCallback)
        self.actions["Prev"].tooltip = "Go to the previous frame"

        self.actions["Next"] = ftk.Action(
            "Next Frame",
            "FrameNext",
            ftk.Key.Right,
            0,
            self._nextCallback)
        self.actions["Next"].tooltip = "Go to the next frame"

        self.actions["End"] = ftk.Action(
            "End Frame",
            "FrameEnd",
            ftk.Key.Down,
            0,
            self._endCallback)
        self.actions["End"].tooltip = "Go to the end frame"

        self.playerObserver = tl.ValueObserverPlayer(app.observePlayer(), self._actionsUpdate)

    def _stopCallback(self):
        if self._player:
            self._player.stop()

    def _forwardCallback(self):
        if self._player:
            self._player.forward()

    def _reverseCallback(self):
        if self._player:
            self._player.reverse()

    def _startCallback(self):
        if self._player:
            self._player.timeAction(tl.TimeAction.Start)

    def _prevCallback(self):
        if self._player:
            self._player.timeAction(tl.TimeAction.FramePrev)

    def _nextCallback(self):
        if self._player:
            self._player.timeAction(tl.TimeAction.FrameNext)

    def _endCallback(self):
        if self._player:
            self._player.timeAction(tl.TimeAction.End)

    def _actionsUpdate(self, player):
        self._player = player
        self.actions["Stop"].enabled = player != None
        self.actions["Forward"].enabled = player != None
        self.actions["Reverse"].enabled = player != None
        self.actions["Start"].enabled = player != None
        self.actions["Prev"].enabled = player != None
        self.actions["Next"].enabled = player != None
        self.actions["End"].enabled = player != None
