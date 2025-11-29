# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

class SettingsModel(ftk.Settings):

    def __init__(self, context):
        docPath = ftk.getUserPath(ftk.UserPath.Documents)
        settingsPath = ftk.getSettingsPath(ftk.Path(docPath, "tlRender").get(), "player-python.json")
        ftk.Settings.__init__(self, context, settingsPath)
