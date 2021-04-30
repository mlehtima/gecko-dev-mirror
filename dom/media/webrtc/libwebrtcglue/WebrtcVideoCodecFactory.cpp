/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-*/
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "WebrtcVideoCodecFactory.h"

#include "GmpVideoCodec.h"
#include "MediaDataCodec.h"
#include "VideoConduit.h"
#include "mozilla/StaticPrefs_media.h"

// libwebrtc includes
#include "api/rtp_headers.h"
#include "api/video_codecs/video_codec.h"
#include "api/video_codecs/video_encoder_software_fallback_wrapper.h"
#include "media/engine/encoder_simulcast_proxy.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"

namespace mozilla {

std::unique_ptr<webrtc::VideoDecoder>
WebrtcVideoDecoderFactory::CreateVideoDecoder(
    const webrtc::SdpVideoFormat& aFormat) {
  std::unique_ptr<webrtc::VideoDecoder> decoder;
  auto type = webrtc::PayloadStringToCodecType(aFormat.name);
  // Attempt to create a decoder using MediaDataDecoder.
  decoder.reset(MediaDataCodec::CreateDecoder(type));
  if (decoder) {
    return decoder;
  }

  // Attempt to create a GMP decoder.
  {
    nsCString tag;

    switch (type) {
    case webrtc::VideoCodecType::kVideoCodecH264:
      tag = "h264"_ns;
      break;
    case webrtc::VideoCodecType::kVideoCodecVP8:
      tag = "vp8"_ns;
      break;
    case webrtc::VideoCodecType::kVideoCodecVP9:
      tag = "vp9"_ns;
      break;
    default:
      return nullptr;
    }

    if (HaveGMPFor(nsLiteralCString(GMP_API_VIDEO_DECODER), { tag })) {
      auto gmpDecoder = WrapUnique(GmpVideoCodec::CreateDecoder(mPCHandle, type));
      mCreatedGmpPluginEvent.Forward(*gmpDecoder->InitPluginEvent());
      mReleasedGmpPluginEvent.Forward(*gmpDecoder->ReleasePluginEvent());
      decoder.reset(gmpDecoder.release());
      return decoder;
    }
  }

  switch (type) {
    case webrtc::VideoCodecType::kVideoCodecH264: {
      // No support for software h264.
      return nullptr;
    }

    // Use libvpx decoders as fallbacks.
    case webrtc::VideoCodecType::kVideoCodecVP8:
      if (!decoder) {
        decoder = webrtc::VP8Decoder::Create();
      }
      break;
    case webrtc::VideoCodecType::kVideoCodecVP9:
      decoder = webrtc::VP9Decoder::Create();
      break;

    default:
      break;
  }

  return decoder;
}

std::unique_ptr<webrtc::VideoEncoder>
WebrtcVideoEncoderFactory::CreateVideoEncoder(
    const webrtc::SdpVideoFormat& aFormat) {
  if (!mInternalFactory->Supports(aFormat)) {
    return nullptr;
  }
  auto type = webrtc::PayloadStringToCodecType(aFormat.name);
  switch (type) {
    case webrtc::VideoCodecType::kVideoCodecVP8:
      // XXX We might be able to use the simulcast proxy for more codecs, but
      // that requires testing.
      return std::make_unique<webrtc::EncoderSimulcastProxy>(
          mInternalFactory.get(), aFormat);
    default:
      return mInternalFactory->CreateVideoEncoder(aFormat);
  }
}

bool WebrtcVideoEncoderFactory::InternalFactory::Supports(
    const webrtc::SdpVideoFormat& aFormat) {
  switch (webrtc::PayloadStringToCodecType(aFormat.name)) {
    case webrtc::VideoCodecType::kVideoCodecVP8:
    case webrtc::VideoCodecType::kVideoCodecVP9:
    case webrtc::VideoCodecType::kVideoCodecH264:
      return true;
    default:
      return false;
  }
}

std::unique_ptr<webrtc::VideoEncoder>
WebrtcVideoEncoderFactory::InternalFactory::CreateVideoEncoder(
    const webrtc::SdpVideoFormat& aFormat) {
  MOZ_ASSERT(Supports(aFormat));

  std::unique_ptr<webrtc::VideoEncoder> platformEncoder;
  auto type = webrtc::PayloadStringToCodecType(aFormat.name);
  platformEncoder.reset(MediaDataCodec::CreateEncoder(aFormat));
  const bool fallback = StaticPrefs::media_webrtc_software_encoder_fallback();
  if (!fallback && platformEncoder) {
    return platformEncoder;
  }

  std::unique_ptr<webrtc::VideoEncoder> encoder;
  // Attempt to create a GMP encoder.
  {
    nsCString tag;

    switch (type) {
    case webrtc::VideoCodecType::kVideoCodecH264:
      tag = "h264"_ns;
      break;
    case webrtc::VideoCodecType::kVideoCodecVP8:
      tag = "vp8"_ns;
      break;
    case webrtc::VideoCodecType::kVideoCodecVP9:
      tag = "vp9"_ns;
      break;
    default:
      return nullptr;
    }

    if (HaveGMPFor(nsLiteralCString(GMP_API_VIDEO_ENCODER), { tag })) {
      auto gmpEncoder = WrapUnique(GmpVideoCodec::CreateEncoder(mPCHandle, type));
      mCreatedGmpPluginEvent.Forward(*gmpEncoder->InitPluginEvent());
      mReleasedGmpPluginEvent.Forward(*gmpEncoder->ReleasePluginEvent());
      encoder.reset(gmpEncoder.release());
      return encoder;
    }
  }

  switch (type) {
    case webrtc::VideoCodecType::kVideoCodecH264: {
      // No support for software h264.
      return nullptr;
    }
    // libvpx fallbacks.
    case webrtc::VideoCodecType::kVideoCodecVP8:
      if (!encoder) {
        encoder = webrtc::VP8Encoder::Create();
      }
      break;
    case webrtc::VideoCodecType::kVideoCodecVP9:
      encoder = webrtc::VP9Encoder::Create();
      break;

    default:
      break;
  }
  if (fallback && encoder && platformEncoder) {
    return webrtc::CreateVideoEncoderSoftwareFallbackWrapper(
        std::move(encoder), std::move(platformEncoder), false);
  }
  if (platformEncoder) {
    return platformEncoder;
  }
  return encoder;
}

}  // namespace mozilla
