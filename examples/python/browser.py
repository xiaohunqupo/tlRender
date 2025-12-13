# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

# This example combines a file browser with a timeline viewport and widget.

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
        fileBrowserOptions = ftk.FileBrowserOptions()
        fileBrowserOptions.dirList.seqExts = tl.timeline.getExts(context, tl.io.FileType.Seq)
        self._fileBrowserModel.options = fileBrowserOptions
        self._fileBrowserModel.path = path
        self._fileBrowserView = ftk.FileBrowserView(
            context,
            ftk.FileBrowserMode.File,
            self._fileBrowserModel)

        # Create the viewport.
        self._viewport = tl.ui.Viewport(context)
        
        # Create the playback buttons.
        self._playButton = ftk.ToolButton(context)
        self._playButton.icon = "PlaybackForward"
        self._playButton.checkable = True
        self._playButton.tooltip = "Toggle playback."

        self._startButton = ftk.ToolButton(context)
        self._startButton.icon = "FrameStart"
        self._startButton.tooltip = "Go to the start frame."

        self._prevButton = ftk.ToolButton(context)
        self._prevButton.icon = "FramePrev"
        self._prevButton.repeatClick = True
        self._prevButton.tooltip = "Go to the previous frame."

        self._nextButton = ftk.ToolButton(context)
        self._nextButton.icon = "FrameNext"
        self._nextButton.repeatClick = True
        self._nextButton.tooltip = "Go to the next frame."

        self._endButton = ftk.ToolButton(context)
        self._endButton.icon = "FrameEnd"
        self._endButton.tooltip = "Go to the end frame."

        # Create the timeline.
        self._timelineWidget = tl.ui.TimelineWidget(context)
        self._timelineWidget.backgroundColor = ftk.ColorRole.Red
        self._timelineWidget.vStretch = ftk.Stretch.Expanding
        timelineDisplayOptions = tl.ui.DisplayOptions()
        timelineDisplayOptions.minimize = False
        timelineDisplayOptions.thumbnailHeight = 300
        timelineDisplayOptions.waveformWidth = 50
        timelineDisplayOptions.waveformHeight = 150
        self._timelineWidget.displayOptions = timelineDisplayOptions

        # Layout the widgets.
        self._layout = ftk.VerticalLayout(context)
        self._layout.spacingRole = ftk.SizeRole._None
        self.widget = self._layout

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
        self._startButton.parent = hLayout
        self._prevButton.parent = hLayout
        self._nextButton.parent = hLayout
        self._endButton.parent = hLayout
        ftk.Divider(context, ftk.Orientation.Vertical, layout)
        self._timelineWidget.parent = layout

        # Update the widget.
        self._widgetUpdate()

        # Setup callbacks.
        selfWeak = weakref.ref(self)
        self._fileBrowserPathWidget.setCallback(
            lambda path: selfWeak()._pathUpdate(path))

        self._fileBrowserView.setSelectCallback(
            lambda path: selfWeak()._selectCallback(path))

        self._playButton.setCheckedCallback(
            lambda value: selfWeak()._playbackCallback(value))
        self._startButton.setClickedCallback(
            lambda: selfWeak()._player.timeAction(tl.timeline.TimeAction.Start))
        self._prevButton.setClickedCallback(
            lambda: selfWeak()._player.timeAction(tl.timeline.TimeAction.FramePrev))
        self._nextButton.setClickedCallback(
            lambda: selfWeak()._player.timeAction(tl.timeline.TimeAction.FrameNext))
        self._endButton.setClickedCallback(
            lambda: selfWeak()._player.timeAction(tl.timeline.TimeAction.End))

        self._pathObserver = ftk.PathObserver(
            self._fileBrowserModel.observePath,
            lambda path: selfWeak()._pathUpdate(path))

    def tickEvent(self, parentsVisible, parentsEnabled, event):
        super().tickEvent(parentsVisible, parentsEnabled, event)
        if self._player:
            self._player.tick()

    def _selectCallback(self, path):
        
        self._player = None
        self._viewport.player = None
        self._playButton.checked = False
        self._timelineWidget.player = None
        self._playbackObserver = None

        # Check for a valid file extension.
        ext = os.path.splitext(path.get())[1]
        if ext and ext.lower() in self._exts:
        
            # Create the timeline and player.
            #
            # \todo Add exception handling.
            timeline = tl.timeline.Timeline(self.context, path)
            self._player = tl.timeline.Player(self.context, timeline)
            self._viewport.player = self._player
            self._timelineWidget.player = self._player

            selfWeak = weakref.ref(self)
            self._playbackObserver = tl.timeline.PlaybackObserver(
                self._player.observePlayback,
                lambda value: selfWeak()._playbackUpdate(value))

        self._widgetUpdate()

    def _playbackCallback(self, value):
        if self._player:
            if value:
                self._player.playback = tl.timeline.Playback.Forward
            else:
                self._player.playback = tl.timeline.Playback.Stop

    def _pathUpdate(self, path):
        self._fileBrowserModel.path = path
        self._fileBrowserPathWidget.path = path

    def _playbackUpdate(self, value):
        if value == tl.timeline.Playback.Forward:
            self._playButton.checked = True
        else:
            self._playButton.checked = False

    def _widgetUpdate(self):
        self._playButton.enabled = self._player != None
        self._startButton.enabled = self._player != None
        self._prevButton.enabled = self._player != None
        self._nextButton.enabled = self._player != None
        self._endButton.enabled = self._player != None

# Create the application.
context = ftk.Context()
tl.ui.init(context)
app = ftk.App(context, sys.argv, "browser-python", "Python browser example.")
if app.exitValue != 0:
    sys.exit(app.exitValue)

# Create the main window.
window = MainWindow(context, app)

app.run()

# Clean up.
window = None
app = None
