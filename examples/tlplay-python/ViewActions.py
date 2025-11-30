# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import weakref

class Actions:

    def __init__(self, context, app):

        appWeak = weakref.ref(app)
        self.actions = {}
