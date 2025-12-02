# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import sys
import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import os
import pathlib
import weakref

class MainWindow(ftk.MainWindow):
    def __init__(self, context, app):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1280, 960))

        self._player = None
        
        # Get the valid file extensions.
        self._exts = tl.timeline.getExts(context)
        
        # Create the file browser path widget.
        self._fileBrowserPathWidget = ftk.FileBrowserPath(context)
        path = os.getcwd()
        self._fileBrowserPathWidget.path = path

        # Create the file browser view.
        self._fileBrowserModel = ftk.FileBrowserModel(context)
        self._fileBrowserModel.path = path
        self._fileBrowserView = ftk.FileBrowserView(
            context,
            ftk.FileBrowserMode.File,
            self._fileBrowserModel)

        # Create the viewport.
        self._viewport = tl.ui.Viewport(context)
        
        # Create the playback button.
        self._playButton = ftk.ToolButton(context)
        self._playButton.icon = "PlaybackForward"
        self._playButton.checkable = True
        
        # Create the timeline.
        self._timelineWidget = tl.ui.TimelineWidget(context)
        self._timelineWidget.vStretch = ftk.Stretch.Expanding
        timelineDisplayOptions = tl.ui.DisplayOptions()
        timelineDisplayOptions.minimize = False
        timelineDisplayOptions.thumbnailHeight = 300
        timelineDisplayOptions.waveformHeight = 150
        self._timelineWidget.displayOptions = timelineDisplayOptions

        # Layout the widgets.
        self._layout = ftk.VerticalLayout(context)
        self._layout.spacingRole = ftk.SizeRole._None
        self.setWidget(self._layout)

        layout = ftk.VerticalLayout(context, self._layout)
        layout.marginRole = ftk.SizeRole.MarginInside
        self._fileBrowserPathWidget.parent = layout
        ftk.Divider(context, ftk.Orientation.Vertical, self._layout)

        self._splitter = ftk.Splitter(context, ftk.Orientation.Horizontal, self._layout)
        self._splitter.split = 0.3
        scrollWidget = ftk.ScrollWidget(context, ftk.ScrollType.Both, self._splitter)
        scrollWidget.vStretch = ftk.Stretch.Expanding
        scrollWidget.border = False
        scrollWidget.widget = self._fileBrowserView

        self._splitter2 = ftk.Splitter(context, ftk.Orientation.Vertical, self._splitter)
        self._splitter2.split = 0.6
        self._viewport.parent = self._splitter2
        layout = ftk.VerticalLayout(context, self._splitter2)
        layout.spacingRole = ftk.SizeRole._None
        hLayout = ftk.HorizontalLayout(context, layout)
        hLayout.marginRole = ftk.SizeRole.MarginInside
        hLayout.spacingRole = ftk.SizeRole.SpacingTool
        self._playButton.parent = hLayout
        ftk.Divider(context, ftk.Orientation.Vertical, layout)
        self._timelineWidget.parent = layout

        # Setup callbacks.
        selfWeak = weakref.ref(self)
        self._fileBrowserPathWidget.setCallback(
            lambda path: selfWeak()._pathUpdate(path))

        self._fileBrowserView.setSelectCallback(
            lambda path: selfWeak()._selectCallback(path))

        self._playButton.setCheckedCallback(
            lambda value: selfWeak()._playbackCallback(value))

        self._pathObserver = ftk.PathObserver(
            self._fileBrowserModel.observePath(),
            lambda path: selfWeak()._pathUpdate(path))

    def tickEvent(self, parentsVisible, parentsEnabled, event):
        super().tickEvent(parentsVisible, parentsEnabled, event)
        if self._player:
            self._player.tick()

    def _pathUpdate(self, path):
        self._fileBrowserModel.path = path
        self._fileBrowserPathWidget.path = path

    def _selectCallback(self, path):
        
        # Check for a valid file extension.
        if os.path.splitext(path.get())[1] in self._exts:
        
            # Create the timeline and player.
            #
            # \todo Add exception handling.
            timeline = tl.timeline.Timeline(self.context, path)
            options = tl.timeline.PlayerOptions()
            self._player = tl.timeline.Player(self.context, timeline, options)
            self._viewport.player = self._player
            self._playButton.checked = False
            self._timelineWidget.player = self._player

            selfWeak = weakref.ref(self)
            self._playbackObserver = tl.timeline.PlaybackObserver(
                self._player.observePlayback,
                lambda value: selfWeak()._playbackUpdate(value))

    def _playbackCallback(self, value):
        if self._player:
            if value:
                self._player.playback = tl.timeline.Playback.Forward
            else:
                self._player.playback = tl.timeline.Playback.Stop

    def _playbackUpdate(self, value):
        if value == tl.timeline.Playback.Forward:
            self._playButton.checked = True
        else:
            self._playButton.checked = False

# Create the application.
context = ftk.Context()
tl.ui.init(context)
app = ftk.App(context, sys.argv, "browser-python", "Python browser example.")
if app.exitValue != 0:
    sys.exit(app.exitValue)

# Create the main window.
window = MainWindow(context, app)

# \bug Set the display scale manually.
app.displayScale = 2

app.run()

# Clean up.
window = None
app = None
