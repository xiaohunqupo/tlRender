# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import weakref

class Model:
    """
    This model handles opening and closing files.
    """
    def __init__(self, context, app):
    
        self._app = weakref.ref(app)

        self._player = tl.timeline.ObservablePlayer(None)
        
        selfWeak = weakref.ref(self)
        self._cacheObserver = tl.timeline.PlayerCacheOptionsObserver(
            app.getSettingsModel().observeCache(),
            lambda value: selfWeak()._cacheUpdate(value))

    def open(self, path):
        """
        Open a timeline, image sequence, or media file.

        \todo Add exception handling.
        """
        timeline = tl.timeline.Timeline(self._app().context, path)
        options = tl.timeline.PlayerOptions()
        options.cache = self._app().getSettingsModel().observeCache().get()
        self._player.setAlways(tl.timeline.Player(self._app().context, timeline, options))

    def close(self):
        """
        Close the current document.
        """
        if self._player.get():
            self._player.setAlways(None)

    def reload(self, path):
        """
        Reload the current document.

        \todo Add exception handling.
        """
        if self._player.get():
            path = self._player.get().path
            timeline = tl.timeline.Timeline(self._app().context, path)
            options = tl.timeline.PlayerOptions()
            options.cache = self._settingsModel.observeCache().get()
            self._player.setAlways(tl.timeline.Player(self.context, timeline, options))

    def observePlayer(self):
        """
        Observe the current timeline player.
        """
        return self._player
    
    def _cacheUpdate(self, value):
        if self._player.get():
            self._player.get().cacheOptions = value

