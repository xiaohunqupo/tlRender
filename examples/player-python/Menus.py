# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

def createFileMenu(context, actions):
    menu = ftk.Menu(context)
    menu.addAction(actions.actions["Open"])
    menu.addAction(actions.actions["Close"])
    menu.addDivider();
    menu.addAction(actions.actions["Exit"])
    return menu

def createPlaybackMenu(context, actions):
    menu = ftk.Menu(context)
    menu.addAction(actions.actions["Stop"])
    menu.addAction(actions.actions["Forward"])
    menu.addAction(actions.actions["Reverse"])
    menu.addDivider();
    menu.addAction(actions.actions["Start"])
    menu.addAction(actions.actions["Prev"])
    menu.addAction(actions.actions["Next"])
    menu.addAction(actions.actions["End"])
    return menu

