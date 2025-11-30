# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import weakref

class Model:
    def __init__(self, context, app):
    
        self._app = weakref.ref(app)

        self._player = tl.ObservableValuePlayer(None)

        self._cacheObserver = tl.ValueObserverPlayerCacheOptions(
            app.getSettingsModel().observeCache(), self._cacheUpdate)

    def open(self, path):
        timeline = tl.Timeline(self._app().context, path)
        options = tl.PlayerOptions()
        options.cache = self._app().getSettingsModel().observeCache().get()
        self._player.setAlways(tl.Player(self._app().context, timeline, options))

    def close(self):
        if self._player.get():
            self._player.setAlways(None)

    def reload(self, path):
        if self._player.get():
            path = self._player.get().path
            timeline = tl.Timeline(self._app().context, path)
            options = tl.PlayerOptions()
            options.cache = self._settingsModel.observeCache().get()
            self._player.setAlways(tl.Player(self.context, timeline, options))

    def observePlayer(self):
        return self._player

    def tick(self):
        if self._player.get():
            self._player.get().tick()
    
    def _cacheUpdate(self, value):
        if self._player.get():
            self._player.get().cacheOptions = value
        
