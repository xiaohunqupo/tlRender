# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import json

class SettingsModel(ftk.Settings):

    def __init__(self, context):
        docPath = ftk.getUserPath(ftk.UserPath.Documents)
        settingsPath = ftk.getSettingsPath(ftk.Path(docPath, "tlRender").get(), "player-python.json")
        ftk.Settings.__init__(self, context, settingsPath)

        cacheOptions = tl.PlayerCacheOptions()
        cacheOptions.from_json(self.getJSON("/Cache")[1])
        self._cache = tl.ObservableValuePlayerCacheOptions(cacheOptions)

    def __del__(self):
        self.setJSON("/Cache", self._cache.get().to_json())

    def observeCache(self):
        return self._cache

    def setCache(self, value):
        self._cache.setIfChanged(value)
