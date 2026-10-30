#ifndef MEETINGBOX_TIMER_DEMO_HPP
#define MEETINGBOX_TIMER_DEMO_HPP

// Paint the TimeBox countdown on the LED faces and run it from InterruptTimer.
// Audio, hotword, and ODAS are not started.
//
//   bazel run //src:timer_demo -- --demo=timer --no-audio --headless --gif=/tmp/meeting_timer.gif
//   bazel run //src:timer_demo -- --seconds=60
int run_timer_demo(int argc, char **argv);

#endif
