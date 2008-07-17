/*
 * PROJECT:     ReactOS Sound System "MME Buddy" Library
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        lib/sound/mmebuddy/wave/streamcontrol.c
 *
 * PURPOSE:     Controls the wave streaming thread.
 *
 * PROGRAMMERS: Andrew Greenwood (silverblade@reactos.org)
*/

#include <windows.h>
#include <mmsystem.h>

#include <ntddk.h>
#include <ntddsnd.h>

#include <mmebuddy.h>
#include "wave.h"

MMRESULT
InitWaveStreamData(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance)
{
    PWAVE_STREAM_INFO StreamInfo;

    VALIDATE_MMSYS_PARAMETER( IsValidSoundDeviceInstance(SoundDeviceInstance) );

    StreamInfo = &SoundDeviceInstance->Streaming.Wave;

    /*StreamInfo->State = WAVE_DD_IDLE;*/

    StreamInfo->BufferQueueHead = NULL;
    StreamInfo->BufferQueueTail = NULL;
    StreamInfo->CurrentBuffer = NULL;

    StreamInfo->BuffersOutstanding = 0;

    return MMSYSERR_NOERROR;
}

MMRESULT
QueueWaveDeviceBuffer(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance,
    IN  PWAVEHDR BufferHeader)
{
    VALIDATE_MMSYS_PARAMETER( IsValidSoundDeviceInstance(SoundDeviceInstance) );
    VALIDATE_MMSYS_PARAMETER( BufferHeader );
    VALIDATE_MMSYS_PARAMETER( BufferHeader->lpData );
    VALIDATE_MMSYS_PARAMETER( BufferHeader->dwBufferLength > 0 );

    /* Make sure the buffer flags are sane */
    BufferHeader->dwFlags &= WHDR_INQUEUE | WHDR_DONE | WHDR_PREPARED |
                             WHDR_BEGINLOOP | WHDR_ENDLOOP;

    if ( ! ( BufferHeader->dwFlags & WHDR_PREPARED ) )
        return WAVERR_UNPREPARED;

    if ( ( BufferHeader->dwFlags & WHDR_INQUEUE ) )
        return WAVERR_STILLPLAYING;

    /* Clear the "done" flag */
    BufferHeader->dwFlags &= ~WHDR_DONE;

    /* ...and at present there's nothing after this buffer, so we do this: */
    BufferHeader->lpNext = NULL;

    return CallUsingSoundThread(SoundDeviceInstance,
                                QueueWaveBuffer_Request,
                                BufferHeader);
}

MMRESULT
GetWaveDeviceState(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance,
    OUT PULONG State)
{
    MMRESULT Result;
    PSOUND_DEVICE SoundDevice;
    PMMFUNCTION_TABLE Functions;

    VALIDATE_MMSYS_PARAMETER( IsValidSoundDeviceInstance(SoundDeviceInstance) );
    VALIDATE_MMSYS_PARAMETER( State );

    // *State = SoundDeviceInstance->Streaming.Wave.State;

    Result = GetSoundDeviceFromInstance(SoundDeviceInstance,
                                        &SoundDevice);
    ASSERT(Result == MMSYSERR_NOERROR);

    Result = GetSoundDeviceFunctionTable(SoundDevice,
                                         &Functions);
    ASSERT(Result == MMSYSERR_NOERROR);

    return Functions->GetWaveDeviceState(SoundDeviceInstance,
                                         State);

}

MMRESULT
DefaultGetWaveDeviceState(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance,
    OUT PULONG State)
{
    MMRESULT Result;

    TRACE_ENTRY();

    ASSERT(SoundDeviceInstance);
    ASSERT(State);

    Result = RetrieveFromSoundDevice(SoundDeviceInstance,
                                     IOCTL_WAVE_GET_STATE,
                                     State,
                                     sizeof(ULONG),
                                     NULL);

    ASSERT( Result == MMSYSERR_NOERROR );

    Result = TranslateInternalMmResult(Result);
    TRACE_EXIT(Result);

    return Result;
}

MMRESULT
PauseWaveDevice(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance)
{
    MMRESULT Result;
    PSOUND_DEVICE SoundDevice;
    PMMFUNCTION_TABLE Functions;

    VALIDATE_MMSYS_PARAMETER( IsValidSoundDeviceInstance(SoundDeviceInstance) );

    Result = GetSoundDeviceFromInstance(SoundDeviceInstance,
                                        &SoundDevice);
    ASSERT(Result == MMSYSERR_NOERROR);

    Result = GetSoundDeviceFunctionTable(SoundDevice,
                                         &Functions);
    ASSERT(Result == MMSYSERR_NOERROR);

    return Functions->PauseWaveDevice(SoundDeviceInstance);
}

MMRESULT
DefaultPauseWaveDevice(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance)
{
    ULONG RequestedState = WAVE_DD_STOP;
    MMRESULT Result;

    TRACE_ENTRY();

    Result = SendToSoundDevice(SoundDeviceInstance,
                               IOCTL_WAVE_SET_STATE,
                               &RequestedState,
                               sizeof(RequestedState),
                               NULL);

    ASSERT( Result == MMSYSERR_NOERROR );

    Result = TranslateInternalMmResult(Result);

    TRACE_EXIT(Result);
    return Result;
}

MMRESULT
RestartWaveDevice(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance)
{
    MMRESULT Result;
    PSOUND_DEVICE SoundDevice;
    PMMFUNCTION_TABLE Functions;

    VALIDATE_MMSYS_PARAMETER( IsValidSoundDeviceInstance(SoundDeviceInstance) );

    Result = GetSoundDeviceFromInstance(SoundDeviceInstance,
                                        &SoundDevice);
    ASSERT(Result == MMSYSERR_NOERROR);

    Result = GetSoundDeviceFunctionTable(SoundDevice,
                                         &Functions);
    ASSERT(Result == MMSYSERR_NOERROR);

    return Functions->RestartWaveDevice(SoundDeviceInstance);
}

MMRESULT
DefaultRestartWaveDevice(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance)
{
    ULONG RequestedState = WAVE_DD_PLAY;
    MMRESULT Result;

    TRACE_ENTRY();

    Result = SendToSoundDevice(SoundDeviceInstance,
                               IOCTL_WAVE_SET_STATE,
                               &RequestedState,
                               sizeof(RequestedState),
                               NULL);

    ASSERT( Result == MMSYSERR_NOERROR );

    Result = TranslateInternalMmResult(Result);
    TRACE_EXIT(Result);
    return Result;
}

/* FIXME - This needs to be done a better way */
MMRESULT
ResetWaveDevice(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance)
{
    MMRESULT Result;
    PSOUND_DEVICE SoundDevice;
    PMMFUNCTION_TABLE Functions;

    VALIDATE_MMSYS_PARAMETER( IsValidSoundDeviceInstance(SoundDeviceInstance) );

    Result = GetSoundDeviceFromInstance(SoundDeviceInstance,
                                        &SoundDevice);
    ASSERT(Result == MMSYSERR_NOERROR);

    Result = GetSoundDeviceFunctionTable(SoundDevice,
                                         &Functions);
    ASSERT(Result == MMSYSERR_NOERROR);

    /* ugly HACK to stop sound playback... FIXME */
    SoundDeviceInstance->Streaming.Wave.CurrentBuffer = NULL;

    /* TODO: Return all audio buffers to the client, marking as DONE */

    return Functions->ResetWaveDevice(SoundDeviceInstance);
}

MMRESULT
DefaultResetWaveDevice(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance)
{
    ULONG RequestedState = WAVE_DD_RESET;
    MMRESULT Result;

    TRACE_ENTRY();

    Result = SendToSoundDevice(SoundDeviceInstance,
                               IOCTL_WAVE_SET_STATE,
                               &RequestedState,
                               sizeof(RequestedState),
                               NULL);

    ASSERT( Result == MMSYSERR_NOERROR );

    Result = TranslateInternalMmResult(Result);
    TRACE_EXIT(Result);
    return Result;
}

MMRESULT
BreakWaveDeviceLoop(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance)
{
    MMRESULT Result;
    PSOUND_DEVICE SoundDevice;
    PMMFUNCTION_TABLE Functions;

    VALIDATE_MMSYS_PARAMETER( IsValidSoundDeviceInstance(SoundDeviceInstance) );

    Result = GetSoundDeviceFromInstance(SoundDeviceInstance,
                                        &SoundDevice);
    ASSERT(Result == MMSYSERR_NOERROR);

    Result = GetSoundDeviceFunctionTable(SoundDevice,
                                         &Functions);
    ASSERT(Result == MMSYSERR_NOERROR);

    return Functions->BreakWaveDeviceLoop(SoundDeviceInstance);
}

MMRESULT
DefaultBreakWaveDeviceLoop(
    IN  PSOUND_DEVICE_INSTANCE SoundDeviceInstance)
{
    ASSERT( SoundDeviceInstance );

    /*
        This will cause the loop to end. Note that the LoopHead member is
        left intact otherwise the streaming routine may be processing the
        start of the loop and think it's starting a new loop.
    */
    SoundDeviceInstance->Streaming.Wave.LoopsRemaining = 0;

    return MMSYSERR_NOERROR;
}
