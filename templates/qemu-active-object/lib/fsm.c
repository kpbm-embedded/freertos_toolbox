#include "fsm.h"

fsm_state_t fsm_initial(void) { return ST_CLOSED; }

fsm_action_t fsm_step(fsm_state_t *s, fsm_event_t ev)
{
  switch (*s) {
  case ST_CLOSED:  if (ev == EV_BUTTON) { *s = ST_OPENING; return ACT_MOTOR_OPEN; } break;
  case ST_OPENING: if (ev == EV_LIMIT_OPEN) { *s = ST_OPEN; return ACT_MOTOR_STOP_START_TIMER; } break;
  case ST_OPEN:    if (ev == EV_TIMEOUT || ev == EV_BUTTON) { *s = ST_CLOSING; return ACT_MOTOR_CLOSE; } break;
  case ST_CLOSING:
    if (ev == EV_LIMIT_CLOSED) { *s = ST_CLOSED; return ACT_MOTOR_STOP; }
    if (ev == EV_OBSTACLE || ev == EV_BUTTON) { *s = ST_OPENING; return ACT_MOTOR_OPEN; }
    break;
  }
  return ACT_NONE;
}

const char *fsm_state_name(fsm_state_t s)
{
  static const char *n[] = { "CLOSED", "OPENING", "OPEN", "CLOSING" };
  return n[s];
}
