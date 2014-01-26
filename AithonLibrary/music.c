#include "Aithon.h"

#define TICKS_PER_SECOND 1000000

Mutex _buzzerMtx;
Thread *_asyncMusicThread;
volatile MusicNote *_asyncNotes;
volatile int _asyncNumNotes;

static PWMConfig _pwmcfg2 = {
	TICKS_PER_SECOND,
	1, // this will be set later to the correct period
	NULL,
	{
		{PWM_OUTPUT_DISABLED, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
	},
	0,
	0,
   0
};

void _play_note(uint16_t frequency, uint16_t duration)
{
	if (duration <= 20) return;
	duration -= 20;
	pwmDisableChannel(&PWMD2, 1);
	_pwmcfg2.period = TICKS_PER_SECOND/frequency;
	pwmStart(&PWMD2, &_pwmcfg2);
	pwmEnableChannel(&PWMD2, 1, PWM_FRACTION_TO_WIDTH(&PWMD2, 100, 25));
	chThdSleepMilliseconds(duration);
	pwmStop(&PWMD2);
	chThdSleepMilliseconds(20);
}

static WORKING_AREA(waMusicThread, 128);
static msg_t MusicThread(void *arg)
{
   (void)arg;
   while (1)
   {
      chMsgRelease(chMsgWait(), 1);
      chMtxLock(&_buzzerMtx);
      int i;
      for (i = 0; i < _asyncNumNotes; i++)
      {
         _play_note(_asyncNotes[i].frequency, _asyncNotes[i].duration);
      }
      chMtxUnlock();
   }
   return 0;
}

void _music_init(void)
{
   chMtxInit(&_buzzerMtx);
   _asyncMusicThread = chThdCreateStatic(waMusicThread, sizeof(waMusicThread), HIGHPRIO, MusicThread, NULL);
}

void music_playNote(MusicNote note)
{
   chMtxLock(&_buzzerMtx);
   _play_note(note.frequency, note.duration);
   chMtxUnlock();
}

void music_playNotes(MusicNote *notes, int numNotes)
{
   chMtxLock(&_buzzerMtx);
	int i;
	for (i = 0; i < numNotes; i++)
	{
      _play_note(notes[i].frequency, notes[i].duration);
	}
   chMtxUnlock();
}

void music_playNotesAsync(MusicNote *notes, int numNotes)
{
   // something else is already playing
   if (!chMtxTryLock(&_buzzerMtx))
      return;
      
   _asyncNotes = notes;
   _asyncNumNotes = numNotes;
   chMsgSend(_asyncMusicThread, 1);
   chMtxUnlock();
}