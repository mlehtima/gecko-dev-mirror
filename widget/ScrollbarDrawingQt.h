/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_widget_ScrollbarDrawingQt_h
#define mozilla_widget_ScrollbarDrawingQt_h

#include "ScrollbarDrawing.h"

namespace mozilla::widget {

class ScrollbarDrawingQt final : public ScrollbarDrawing {
 public:
  ScrollbarDrawingQt() : ScrollbarDrawing(Kind::Qt) {}
  virtual ~ScrollbarDrawingQt() = default;

  LayoutDeviceIntSize GetMinimumWidgetSize(nsPresContext*,
                                           StyleAppearance aAppearance,
                                           nsIFrame* aFrame) override;

  ScrollbarSizes GetScrollbarSizes(nsPresContext*, StyleScrollbarWidth,
                                   Overlay) override;

  template <typename PaintBackendData>
  void DoPaintScrollbarThumb(PaintBackendData&, const LayoutDeviceRect& aRect,
                             ScrollbarKind, nsIFrame* aFrame,
                             const ComputedStyle& aStyle,
                             const EventStates& aElementState,
                             const EventStates& aDocumentState,
                             const DPIRatio&);
  bool PaintScrollbarThumb(DrawTarget&, const LayoutDeviceRect&, ScrollbarKind,
                           nsIFrame*, const ComputedStyle&,
                           const EventStates& aElementState,
                           const EventStates& aDocumentState, const Colors&,
                           const DPIRatio&) override;
  bool PaintScrollbarThumb(WebRenderBackendData&, const LayoutDeviceRect&,
                           ScrollbarKind, nsIFrame*, const ComputedStyle&,
                           const EventStates& aElementState,
                           const EventStates& aDocumentState, const Colors&,
                           const DPIRatio&) override;

  void RecomputeScrollbarParams() override;
};

}  // namespace mozilla::widget

#endif
