# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import weakref

class Actions:

    def __init__(self, context, app):
        
        self._playback = tl.timeline.Playback.Forward

        self.actions = {}
        self.actions["Stop"] = ftk.Action(
            "Stop",
            "PlaybackStop",
            ftk.Key.K,
            0,
            self._stopCallback)
        self.actions["Stop"].tooltip = "Stop playback."

        self.actions["Forward"] = ftk.Action(
            "Forward",
            "PlaybackForward",
            ftk.Key.L,
            0,
            self._forwardCallback)
        self.actions["Forward"].tooltip = "Start forward playback."

        self.actions["Reverse"] = ftk.Action(
            "Reverse",
            "PlaybackReverse",
            ftk.Key.J,
            0,
            self._reverseCallback)
        self.actions["Reverse"].tooltip = "Start reverse playback."

        self.actions["TogglePlayback"] = ftk.Action(
            "Toggle Playback",
            ftk.Key.Space,
            0,
            self._togglePlaybackCallback)
        self.actions["TogglePlayback"].tooltip = "Toggle playback direction."

        self.actions["Start"] = ftk.Action(
            "Start Frame",
            "FrameStart",
            ftk.Key.Up,
            0,
            self._startCallback)
        self.actions["Start"].tooltip = "Go to the start frame."

        self.actions["Prev"] = ftk.Action(
            "Previous Frame",
            "FramePrev",
            ftk.Key.Left,
            0,
            self._prevCallback)
        self.actions["Prev"].tooltip = "Go to the previous frame."

        self.actions["Next"] = ftk.Action(
            "Next Frame",
            "FrameNext",
            ftk.Key.Right,
            0,
            self._nextCallback)
        self.actions["Next"].tooltip = "Go to the next frame."

        self.actions["End"] = ftk.Action(
            "End Frame",
            "FrameEnd",
            ftk.Key.Down,
            0,
            self._endCallback)
        self.actions["End"].tooltip = "Go to the end frame."

        self.actions["SetInPoint"] = ftk.Action(
            "Set In Point",
            ftk.Key.I,
            0,
            self._setInPointCallback)
        self.actions["SetInPoint"].tooltip = "Set the in point to the current frame."

        self.actions["ResetInPoint"] = ftk.Action(
            "Reset In Point",
            ftk.Key.I,
            ftk.KeyModifier.Shift,
            self._resetInPointCallback)
        self.actions["ResetInPoint"].tooltip = "Reset the in point to the start frame."

        self.actions["SetOutPoint"] = ftk.Action(
            "Set Out Point",
            ftk.Key.O,
            0,
            self._setOutPointCallback)
        self.actions["SetOutPoint"].tooltip = "Set the out point to the current frame."

        self.actions["ResetOutPoint"] = ftk.Action(
            "Reset Out Point",
            ftk.Key.O,
            ftk.KeyModifier.Shift,
            self._resetOutPointCallback)
        self.actions["ResetOutPoint"].tooltip = "Reset the out point to the end frame."

        self._playerObserver = tl.timeline.PlayerObserver(
            app.getDocumentModel().observePlayer(),
            self._playerUpdate)

    def _stopCallback(self):
        if self._player:
            self._player.stop()

    def _forwardCallback(self):
        if self._player:
            self._player.forward()

    def _reverseCallback(self):
        if self._player:
            self._player.reverse()

    def _togglePlaybackCallback(self):
        if self._player:
            if self._player.isStopped:
                self._player.playback = self._playback
            else:
                self._player.stop()

    def _startCallback(self):
        if self._player:
            self._player.timeAction(tl.timeline.TimeAction.Start)

    def _prevCallback(self):
        if self._player:
            self._player.timeAction(tl.timeline.TimeAction.FramePrev)

    def _nextCallback(self):
        if self._player:
            self._player.timeAction(tl.timeline.TimeAction.FrameNext)

    def _endCallback(self):
        if self._player:
            self._player.timeAction(tl.timeline.TimeAction.End)

    def _setInPointCallback(self):
        if self._player:
            self._player.setInPoint()

    def _resetInPointCallback(self):
        if self._player:
            self._player.resetInPoint()

    def _setOutPointCallback(self):
        if self._player:
            self._player.setOutPoint()

    def _resetOutPointCallback(self):
        if self._player:
            self._player.resetOutPoint()

    def _playerUpdate(self, player):
        
        self._player = player
        
        if player:
            self._playbackObserver = tl.timeline.PlaybackObserver(
                player.observePlayback,
                self._playbackUpdate)
        else:
            self._playbackObserver = None
        
        self.actions["Stop"].enabled = player != None
        self.actions["Forward"].enabled = player != None
        self.actions["Reverse"].enabled = player != None
        self.actions["TogglePlayback"].enabled = player != None
        self.actions["Start"].enabled = player != None
        self.actions["Prev"].enabled = player != None
        self.actions["Next"].enabled = player != None
        self.actions["End"].enabled = player != None
        self.actions["SetInPoint"].enabled = player != None
        self.actions["ResetInPoint"].enabled = player != None
        self.actions["SetOutPoint"].enabled = player != None
        self.actions["ResetOutPoint"].enabled = player != None

    def _playbackUpdate(self, playback):

        if playback != tl.timeline.Playback.Stop:
            self._playback = playback

        self.actions["Stop"].checked = tl.timeline.Playback.Stop == playback
        self.actions["Forward"].checked = tl.timeline.Playback.Forward == playback
        self.actions["Reverse"].checked = tl.timeline.Playback.Reverse == playback
