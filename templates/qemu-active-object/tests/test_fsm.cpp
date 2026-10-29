#include <gtest/gtest.h>
#include "fsm.h"

TEST(Fsm, OpensOnButton) {
  fsm_state_t s = fsm_initial();
  EXPECT_EQ(fsm_step(&s, EV_BUTTON), ACT_MOTOR_OPEN);
  EXPECT_EQ(s, ST_OPENING);
}

TEST(Fsm, FullCycle) {
  fsm_state_t s = fsm_initial();
  fsm_step(&s, EV_BUTTON);
  EXPECT_EQ(fsm_step(&s, EV_LIMIT_OPEN), ACT_MOTOR_STOP_START_TIMER);
  EXPECT_EQ(fsm_step(&s, EV_TIMEOUT), ACT_MOTOR_CLOSE);
  EXPECT_EQ(fsm_step(&s, EV_LIMIT_CLOSED), ACT_MOTOR_STOP);
  EXPECT_EQ(s, ST_CLOSED);
}

TEST(Fsm, ObstacleReopens) {
  fsm_state_t s = ST_CLOSING;
  EXPECT_EQ(fsm_step(&s, EV_OBSTACLE), ACT_MOTOR_OPEN);
  EXPECT_EQ(s, ST_OPENING);
}

TEST(Fsm, IgnoresIrrelevantEvents) {
  fsm_state_t s = ST_CLOSED;
  EXPECT_EQ(fsm_step(&s, EV_TIMEOUT), ACT_NONE);
  EXPECT_EQ(s, ST_CLOSED);
}
