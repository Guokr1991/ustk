/****************************************************************************
 *
 * This file is part of the ustk software.
 * Copyright (C) 2016 - 2017 by Inria. All rights reserved.
 *
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ustk with software that can not be combined with the GNU
 * GPL, please contact Inria about acquiring a ViSP Professional
 * Edition License.
 *
 * This software was developed at:
 * Inria Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 *
 * If you have questions regarding the use of this file, please contact
 * Inria at ustk@inria.fr
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors:
 * Marc Pouliquen
 *
 *****************************************************************************/

/**
 * @file usNetworkGrabberRF3D.h
 * @brief Grabber used to grab RF volumes from ultrasonix station, using a tcp connection.
 */

#ifndef __usNetworkGrabberRF3D_h_
#define __usNetworkGrabberRF3D_h_

#include <visp3/ustk_core/usConfig.h>

#if defined(USTK_HAVE_QT5) || defined(USTK_HAVE_VTK_QT)

#include <vector>

#include <visp3/ustk_core/usImageRF2D.h>
#include <visp3/ustk_core/usImageRF3D.h>
#include <visp3/ustk_core/usMHDSequenceWriter.h>
#include <visp3/ustk_grabber/usFrameGrabbedInfo.h>
#include <visp3/ustk_grabber/usNetworkGrabber.h>
#include <visp3/ustk_grabber/usVolumeGrabbedInfo.h>

/**
 * @class usNetworkGrabberRF3D
 * @brief Specific class to grab RF volumes from the ultrasound station on the network.
 * @ingroup module_ustk_grabber
 *
 * The following figure details the network communication process and summarizes the steps to follow to acquire
 * ultrasound images:
 * \image html img-usNetworkGrabber.png
 *
 * This grabber manages a buffer system to avoid multiple copy of the volumes.
 * The acquire() method returns you a pointer on a new volume, you can acess and modify the volume (it is thread-safe).
 * Acquire() can be blocking, the behaviour depends on how often you call it:
 * - If you call acquire() faster than the volumes are arriving on the network, it is blocking to wait next volume
 * coming.
 * - If you call it slower you will loose volumes, but you will get the last volume available.
 */
class VISP_EXPORT usNetworkGrabberRF3D : public usNetworkGrabber
{
  Q_OBJECT
public:
  explicit usNetworkGrabberRF3D(usNetworkGrabber *parent = 0);
  ~usNetworkGrabberRF3D();

  void activateRecording(std::string path);

  usVolumeGrabbedInfo<usImageRF3D<short int> > *acquire();

  void dataArrived();

  bool isFirstFrameAvailable() { return m_firstFrameAvailable; }

  void setVolumeField(usVolumeField volumeField);

  void stopRecording();

signals:
  void newVolumeAvailable();

protected:
  void includeFrameInVolume();

private:
  // grabbed image (we have to "turn" it if it is a pre-scan frame):
  // the rows and cols have to be interverted, it is not a valid image to use.
  usFrameGrabbedInfo<usImageRF2D<short int> > m_grabbedImage;

  // to keep saved motor settings from one frame to next one
  usMotorSettings m_motorSettings;

  // Output images: we have to invert (i <-> j) in the image grabbed
  std::vector<usVolumeGrabbedInfo<usImageRF3D<short int> > *> m_outputBuffer;
  bool m_firstFrameAvailable;
  bool m_firstVolumeAvailable;

  // to manage the recording process
  bool m_recordingOn;
  usMHDSequenceWriter m_sequenceWriter;
  uint64_t m_firstImageTimestamp;

  // volume selection
  usNetworkGrabber::usVolumeField m_volumeField;
};

#endif // QT4 || QT5
#endif // __usNetworkGrabberRF3D_h_
