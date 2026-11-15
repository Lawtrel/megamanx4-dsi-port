/* Cutscenes are deliberately omitted in the current native target.
 * Return to the original caller without yielding, touching display state,
 * or interrupting the current music/effects. Dialogues remain game tasks. */
#include "common.h"
void func_80018000(arg_u8 movie){
 (void)movie;
}
