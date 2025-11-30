# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import weakref

class CacheWidget(ftk.IWidget):

    def __init__(self, context, app, parent = None):
        ftk.IWidget.__init__(self, context, "CacheWidget", parent)
        
        self._app = weakref.ref(app)
        
        self._videoEdit = ftk.FloatEdit(context)
        self._videoEdit.range = ftk.RangeF(0.0, 128.0);
        self._videoEdit.step = 1.0
        self._videoEdit.largeStep = 10.0
        
        self._audioEdit = ftk.FloatEdit(context)
        self._audioEdit.range = ftk.RangeF(0.0, 128.0);
        self._audioEdit.step = 1.0
        self._audioEdit.largeStep = 10.0

        self._readBehindEdit = ftk.FloatEdit(context)
        self._readBehindEdit.range = ftk.RangeF(0.0, 2.0)

        self._layout = ftk.FormLayout(context, self)
        self._layout.spacingRole = ftk.SizeRole.SpacingSmall
        self._layout.addRow("Video cache (GB):", self._videoEdit)
        self._layout.addRow("Audio cache (GB):", self._audioEdit)
        self._layout.addRow("Read behind (seconds):", self._readBehindEdit)

        self._videoEdit.setCallback(self._videoCallback)
        self._audioEdit.setCallback(self._audioCallback)
        self._readBehindEdit.setCallback(self._readBehindCallback)

        self._cacheObserver = tl.timeline.ValueObserverPlayerCacheOptions(
            app.getSettingsModel().observeCache(), self._cacheUpdate)

    def setGeometry(self, value):
        ftk.IWidget.setGeometry(self, value)
        self._layout.setGeometry(value)
    
    def sizeHintEvent(self, event):
        self.setSizeHint(self._layout.sizeHint)

    def _videoCallback(self, value):
        if self._app:
            cache = self._app().getSettingsModel().observeCache().get()
            cache.videoGB = value
            self._app().getSettingsModel().setCache(cache)

    def _audioCallback(self, value):
        if self._app:
            cache = self._app().getSettingsModel().observeCache().get()
            cache.audioGB = value
            self._app().getSettingsModel().setCache(cache)

    def _readBehindCallback(self, value):
        if self._app:
            cache = self._app().getSettingsModel().observeCache().get()
            cache.readBehind = value
            self._app().getSettingsModel().setCache(cache)

    def _cacheUpdate(self, value):
        self._videoEdit.value = value.videoGB;
        self._audioEdit.value = value.audioGB;
        self._readBehindEdit.value = value.readBehind;

class Widget(ftk.IWidget):

    def __init__(self, context, app, parent = None):
        ftk.IWidget.__init__(self, context, "SettingsWidget", parent)
        
        layout = ftk.VerticalLayout(context)
        layout.marginRole = ftk.SizeRole.MarginSmall
        layout.spacingRole = ftk.SizeRole.SpacingSmall

        cacheGroupBox = ftk.GroupBox(context, "Cache", layout)
        self._cacheWidget = CacheWidget(context, app, cacheGroupBox)

        self._scrollWidget = ftk.ScrollWidget(context, ftk.ScrollType.Both, self)
        self._scrollWidget.border = False
        self._scrollWidget.widget = layout

    def setGeometry(self, value):
        ftk.IWidget.setGeometry(self, value)
        self._scrollWidget.setGeometry(value)
    
    def sizeHintEvent(self, event):
        self.setSizeHint(self._scrollWidget.sizeHint)
