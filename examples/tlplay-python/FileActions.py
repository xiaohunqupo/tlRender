# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import weakref

class Actions:

    def __init__(self, context, app, mainWindow):

        appWeak = weakref.ref(app)
        mainWindowWeak = weakref.ref(mainWindow)
        self.actions = {}
        self.actions["Open"] = ftk.Action(
            "Open",
            "FileOpen",
            ftk.Key.O,
            ftk.commandKeyModifier,
            lambda: context.getSystemByName("ftk::FileBrowserSystem").open(mainWindowWeak(), appWeak().open))
        self.actions["Open"].tooltip = "Open an image sequence, movie, or timeline file."

        self.actions["Close"] = ftk.Action(
            "Close",
            "FileClose",
            ftk.Key.E,
            ftk.commandKeyModifier,
            lambda: appWeak().close())
        self.actions["Close"].tooltip = "Close the current file."

        self.actions["Reload"] = ftk.Action(
            "Reload",
            "FileReload",
            ftk.Key.R,
            ftk.commandKeyModifier,
            lambda: appWeak().reload())
        self.actions["Reload"].tooltip = "Reload the current file."

        self.actions["Exit"] = ftk.Action(
            "Exit",
            ftk.Key.Q,
            ftk.commandKeyModifier,
            lambda: appWeak().exit())

        self._playerObserver = tl.timeline.ValueObserverPlayer(
            app.getDocumentModel().observePlayer(),
            self._playerUpdate)

    def _playerUpdate(self, player):
        self.actions["Close"].enabled = player != None
        self.actions["Reload"].enabled = player != None
