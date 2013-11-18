#include "Aithon.h"

#define TICKS_PER_SECOND 1000000

Mutex buzzer;
Thread *asyncMusicThread;
volatile MusicNote *asyncNotes;
volatile int asyncNumNotes;

static PWMConfig pwmcfg2 = {
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
	0
};

void playSound(uint16_t frequency, uint16_t duration)
{
	if (duration <= 20) return;
	duration -= 20;
	pwmDisableChannel(&PWMD2, 1);
	pwmcfg2.period = TICKS_PER_SECOND/frequency;
	pwmStart(&PWMD2, &pwmcfg2);
	pwmEnableChannel(&PWMD2, 1, PWM_FRACTION_TO_WIDTH(&PWMD2, 100, 25));
	chThdSleepMilliseconds(duration);
	pwmStop(&PWMD2);
	chThdSleepMilliseconds(20);
}

void aiMusicPlayNote(MusicNote note)
{
   chMtxLock(&buzzer);
   playSound(note.frequency, note.duration);
   chMtxUnlock();
}

void aiMusicPlayNotes(MusicNote *notes, int numNotes)
{
   chMtxLock(&buzzer);
	int i;
	for (i = 0; i < numNotes; i++)
	{
      playSound(notes[i].frequency, notes[i].duration);
	}
   chMtxUnlock();
}

static WORKING_AREA(waMusicThread, 128);
static msg_t MusicThread(void *arg)
{
   (void)arg;
   while (1)
   {
      chMsgRelease(chMsgWait(), 1);
      chMtxLock(&buzzer);
      int i;
      for (i = 0; i < asyncNumNotes; i++)
      {
         playSound(asyncNotes[i].frequency, asyncNotes[i].duration);
      }
      chMtxUnlock();
   }
   return 0;
}

void aiMusicPlayNotesAsync(MusicNote *notes, int numNotes)
{
   // something else is already playing
   if (!chMtxTryLock(&buzzer))
      return;
      
   asyncNotes = notes;
   asyncNumNotes = numNotes;
   chMsgSend(asyncMusicThread, 1);
   chMtxUnlock();
}

void aiMusicInit(void)
{
   chMtxInit(&buzzer);
   asyncMusicThread = chThdCreateStatic(waMusicThread, sizeof(waMusicThread), HIGHPRIO, MusicThread, NULL);
}