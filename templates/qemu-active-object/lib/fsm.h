/* Door controller state machine: pure C, no RTOS, tested on the host (tests/). */
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
typedef enum { ST_CLOSED, ST_OPENING, ST_OPEN, ST_CLOSING } fsm_state_t;
typedef enum { EV_BUTTON, EV_LIMIT_OPEN, EV_LIMIT_CLOSED, EV_TIMEOUT, EV_OBSTACLE } fsm_event_t;
typedef enum { ACT_NONE, ACT_MOTOR_OPEN, ACT_MOTOR_CLOSE, ACT_MOTOR_STOP_START_TIMER, ACT_MOTOR_STOP } fsm_action_t;
fsm_state_t fsm_initial(void);
fsm_action_t fsm_step(fsm_state_t *state, fsm_event_t ev);   /* updates *state, returns what to do */
const char *fsm_state_name(fsm_state_t s);
#ifdef __cplusplus
}
#endif
