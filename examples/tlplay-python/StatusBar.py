# SPDX-License-Identifier: BSD-3-Clause
# Copyright Contributors to the feather-tk project.

import opentimelineio as otio
import ftkPy as ftk
import tlRenderPy as tl

class Widget(ftk.IWidget):

    def __init__(self, context, app, actions, parent = None):
        ftk.IWidget.__init__(self, context, "StatusBar.Widget", parent)

        self._errorLabel = ftk.Label(context)
        self._errorLabel.hStretch = ftk.Stretch.Expanding
        self._errorLabel.marginRole = ftk.SizeRole.MarginInside

        self._infoLabel = ftk.Label(context)
        self._infoLabel.marginRole = ftk.SizeRole.MarginInside

        self._layout = ftk.HorizontalLayout(context, self)
        self._layout.spacingRole = ftk.SizeRole.SpacingSmall
        self._errorLabel.parent = self._layout
        ftk.Divider(context, ftk.Orientation.Horizontal, self._layout)
        self._infoLabel.parent = self._layout

        self._playerObserver = tl.timeline.ValueObserverPlayer(
            app.getDocumentModel().observePlayer(),
            self._playerUpdate)

    def setGeometry(self, value):
        ftk.IWidget.setGeometry(self, value)
        self._layout.setGeometry(value)
    
    def sizeHintEvent(self, event):
        self.setSizeHint(self._layout.sizeHint)

    def _playerUpdate(self, player):
        text = []
        tooltip = []
        if player:
            path = player.path
            text.append(path.fileName)
            ioInfo = player.ioInfo
            if ioInfo.video:
                video = ioInfo.video[0]
                text.append("{}x{}:{:.2f}".format(
                    video.size.w,
                    video.size.h,
                    video.aspect))
            tooltip.append(path.get())
        self._infoLabel.text = " ".join(text)
        self._infoLabel.tooltip = "\n".join(tooltip)
