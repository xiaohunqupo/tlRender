# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import os
import weakref

class MainWindow(ftk.MainWindow):
    def __init__(self, context, app):
        ftk.MainWindow.__init__(self, context, app, ftk.Size2I(1280, 960))

        path = os.getcwd()

        self._player = None

        self._fileBrowserModel = ftk.FileBrowserModel(context)
        self._fileBrowserModel.path = path
        
        self._fileBrowserPathWidget = ftk.FileBrowserPath(context)
        self._fileBrowserPathWidget.path = path

        self._fileBrowserView = ftk.FileBrowserView(
            context,
            ftk.FileBrowserMode.File,
            self._fileBrowserModel)

        self._viewport = tl.ui.Viewport(context)

        self._timelineWidget = tl.ui.TimelineWidget(context, app.getTimeUnitsModel())

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
        layout = ftk.VerticalLayout(context, self._splitter)
        layout.spacingRole = ftk.SizeRole._None
        self._viewport.parent = layout
        self._timelineWidget.parent = layout

        selfWeak = weakref.ref(self)
        self._fileBrowserPathWidget.setCallback(
            lambda path: selfWeak()._pathUpdate(path))
        
        self._fileBrowserView.setSelectCallback(
            lambda path: selfWeak()._selectCallback(path))

        self._pathObserver = ftk.PathObserver(
            self._fileBrowserModel.observePath(),
            lambda path: selfWeak()._pathUpdate(path))

    def tickEvent(self, parentsVisible, parentsEnabled, event):
        super().tickEvent(parentsVisible, parentsEnabled, event)
        if self._player:
            print("tick")
            self._player.tick()

    def _pathUpdate(self, path):
        self._fileBrowserModel.path = path
        self._fileBrowserPathWidget.path = path

    def _selectCallback(self, path):
        fileBrowserSystem = self.context.getSystemByName("ftk::FileBrowserSystem")
        exts = fileBrowserSystem.model.exts
        if os.path.splitext(path.get())[1] in exts:
            timeline = tl.timeline.Timeline(self.context, path)
            options = tl.timeline.PlayerOptions()
            self._player = tl.timeline.Player(self.context, timeline, options)
            self._viewport.player = self._player
            self._timelineWidget.player = self._player
