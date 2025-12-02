# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

import weakref

class Actions:
    """
    This class provides file actions.
    """
    def __init__(self, context, app, mainWindow):

        appWeak = weakref.ref(app)
        mainWindowWeak = weakref.ref(mainWindow)
        self.actions = {}
        self.actions["Open"] = ftk.Action(
            "Open",
            "FileOpen",
            ftk.Key.O,
            ftk.commandKeyModifier,
            lambda: context.getSystemByName("ftk::FileBrowserSystem").open(
                mainWindowWeak(),
                appWeak().getDocumentModel().open))
        self.actions["Open"].tooltip = "Open an image sequence, movie, or timeline file."

        self.actions["Close"] = ftk.Action(
            "Close",
            "FileClose",
            ftk.Key.E,
            ftk.commandKeyModifier,
            lambda: appWeak().getDocumentModel().close())
        self.actions["Close"].tooltip = "Close the current file."

        self.actions["Reload"] = ftk.Action(
            "Reload",
            "FileReload",
            ftk.Key.R,
            ftk.commandKeyModifier,
            lambda: appWeak().getDocumentModel().reload())
        self.actions["Reload"].tooltip = "Reload the current file."

        self.actions["Exit"] = ftk.Action(
            "Exit",
            ftk.Key.Q,
            ftk.commandKeyModifier,
            lambda: appWeak().exit())

        selfWeak = weakref.ref(self)
        self._playerObserver = tl.timeline.PlayerObserver(
            app.getDocumentModel().observePlayer(),
            lambda player: selfWeak()._playerUpdate(player))

    def _playerUpdate(self, player):
        self.actions["Close"].enabled = player != None
        self.actions["Reload"].enabled = player != None
