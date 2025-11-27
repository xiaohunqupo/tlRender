# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

def createFileToolBar(context, actions):
    toolBar = ftk.ToolBar(context)
    toolBar.addAction(actions.actions["Open"])
    toolBar.addAction(actions.actions["Close"])
    return toolBar

