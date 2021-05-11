/* -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; tab-width: 2; -*- */
/* vim: set sw=2 ts=8 et tw=80 : */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ScrollbarDrawingQt.h"

#include "mozilla/gfx/Helpers.h"
#include "nsLayoutUtils.h"
#include "nsNativeTheme.h"

using namespace mozilla::gfx;
namespace mozilla::widget {

static const CSSIntCoord SCROLL_BAR_SIZE = 17;


LayoutDeviceIntSize ScrollbarDrawingQt::GetMinimumWidgetSize(
    nsPresContext* aPresContext, StyleAppearance aAppearance,
    nsIFrame* aFrame) {
  MOZ_ASSERT(nsNativeTheme::IsWidgetScrollbarPart(aAppearance));
  auto sizes = ScrollbarDrawing::GetScrollbarSizes(aPresContext, aFrame);
  MOZ_ASSERT(sizes.mHorizontal == sizes.mVertical);
  LayoutDeviceIntSize size{sizes.mHorizontal, sizes.mVertical};
  if (aAppearance == StyleAppearance::ScrollbarHorizontal ||
      aAppearance == StyleAppearance::ScrollbarVertical ||
      aAppearance == StyleAppearance::ScrollbarthumbHorizontal ||
      aAppearance == StyleAppearance::ScrollbarthumbVertical) {
    CSSCoord thumbSize(
        StaticPrefs::widget_non_native_theme_gtk_scrollbar_thumb_cross_size());
    const bool isVertical =
        aAppearance == StyleAppearance::ScrollbarVertical ||
        aAppearance == StyleAppearance::ScrollbarthumbVertical;
    auto dpi = GetDPIRatioForScrollbarPart(aPresContext);
    if (isVertical) {
      size.height = thumbSize * dpi;
    } else {
      size.width = thumbSize * dpi;
    }
  }
  return size;
}

auto ScrollbarDrawingQt::GetScrollbarSizes(nsPresContext* aPresContext,
                                              StyleScrollbarWidth aWidth,
                                              Overlay)
    -> ScrollbarSizes {
  int32_t size = aPresContext->CSSPixelsToDevPixels(SCROLL_BAR_SIZE);
  return {size, size};
}

template <typename PaintBackendData>
void ScrollbarDrawingQt::DoPaintScrollbarThumb(
    PaintBackendData& aPaintData, const LayoutDeviceRect& aRect,
    ScrollbarKind aScrollbarKind, nsIFrame* aFrame, const ComputedStyle& aStyle,
    const EventStates& aElementState, const EventStates& aDocumentState,
    const DPIRatio& aDpiRatio) {
  ScrollbarParams params =
      ComputeScrollbarParams(aFrame, aStyle, aScrollbarKind);
  auto thumb = GetThumbRect(aRect, params, aDpiRatio.scale);
  LayoutDeviceCoord radius =
      (params.isHorizontal ? thumb.mRect.Height() : thumb.mRect.Width()) / 2.0f;
  ThemeDrawing::PaintRoundedRectWithRadius(
      aPaintData, thumb.mRect, thumb.mRect,
      sRGBColor::FromABGR(thumb.mFillColor), sRGBColor::White(0.0f), 0.0f,
      radius / aDpiRatio, aDpiRatio);
  if (!thumb.mStrokeColor) {
    return;
  }

  // Paint the stroke if needed.
  auto strokeRect = thumb.mRect;
  strokeRect.Inflate(thumb.mStrokeOutset + thumb.mStrokeWidth);
  radius =
      (params.isHorizontal ? strokeRect.Height() : strokeRect.Width()) / 2.0f;
  ThemeDrawing::PaintRoundedRectWithRadius(
      aPaintData, strokeRect, sRGBColor::White(0.0f),
      sRGBColor::FromABGR(thumb.mStrokeColor), thumb.mStrokeWidth,
      radius / aDpiRatio, aDpiRatio);
}

bool ScrollbarDrawingQt::PaintScrollbarThumb(
    DrawTarget& aDt, const LayoutDeviceRect& aRect,
    ScrollbarKind aScrollbarKind, nsIFrame* aFrame, const ComputedStyle& aStyle,
    const EventStates& aElementState, const EventStates& aDocumentState,
    const Colors&, const DPIRatio& aDpiRatio) {
  // TODO: Maybe respect the UseSystemColors setting?
  DoPaintScrollbarThumb(aDt, aRect, aScrollbarKind, aFrame, aStyle,
                        aElementState, aDocumentState, aDpiRatio);
  return true;
}

bool ScrollbarDrawingQt::PaintScrollbarThumb(
    WebRenderBackendData& aWrData, const LayoutDeviceRect& aRect,
    ScrollbarKind aScrollbarKind, nsIFrame* aFrame, const ComputedStyle& aStyle,
    const EventStates& aElementState, const EventStates& aDocumentState,
    const Colors&, const DPIRatio& aDpiRatio) {
  // TODO: Maybe respect the UseSystemColors setting?
  DoPaintScrollbarThumb(aWrData, aRect, aScrollbarKind, aFrame, aStyle,
                        aElementState, aDocumentState, aDpiRatio);
  return true;
}

void ScrollbarDrawingQt::RecomputeScrollbarParams() {
  uint32_t defaultSize = SCROLL_BAR_SIZE;
  uint32_t overrideSize =
      StaticPrefs::widget_non_native_theme_scrollbar_size_override();
  if (overrideSize > 0) {
    defaultSize = overrideSize;
  }
  mHorizontalScrollbarHeight = mVerticalScrollbarWidth = defaultSize;
}

}  // namespace mozilla::widget
